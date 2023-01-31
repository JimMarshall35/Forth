#include "Forth2.h"
#include "ForthStringHelpers.h"

/************************************\
 *   Forth - Jim Marshall - 2022    *
 *     jimmarshall35@gmail.com      *
\************************************/

typedef enum {
	Return,
	Add,
	Subtract,
	Divide,
	Multiply,
	Modulo,
	Branch0,
	Branch,
	Dup,
	Swap,
	Rot,
	NumLiteral,
	Emit,
	Fetch,
	Store,
	ByteFetch,
	ByteStore,
	SearchForAndPushExecutionToken,
	Here,
	Allot,
	Colon,
	SemiColon,
	Show,
	ShowWords,
	Immediate,
	Create,
	Equals,
	GreaterThan,
	LessThan,
	And,
	Or,
	Not,
	CellSize,
	CommentStart,
	CommentStop,
	Drop,
	PushReturnStackWord,
	PopReturnStackWord,
	TwoDup,
	TwoSwap,
	I,
	J,
	FullStop,
	SearchForAndPushExecutionTokenCompileTime,
	StringLiteral,
	StringLiteralCompileTime,
	EnterWord,
	CallC,
	Key,
	Body,
	SwitchToCompile,
	SwitchToInterpret,

	NumPrimitives // LEAVE AT END
}PrimitiveWordTokenValues;

// forward declarations
static ExecutionToken SearchForToken(ForthVm* vm);
static Bool LoadNextToken(ForthVm* vm); // to be used at compile time only
static int CompileCStringToForthByteCode(ForthVm* vm, const char* string, char delim);

#define PopIntStack(vm) *(--vm->intStackTop)
#define PushIntStack(vm, val) *(vm->intStackTop++) = val;

#define PopReturnStack(vm) *(--vm->returnStackTop)
#define PushReturnStack(vm, val) *(vm->returnStackTop++) = val


static ForthDictHeader* LastWordAdded(ForthVm* vm) {
	return (vm->dictionarySearchStart == NULL) ? vm->memory : vm->dictionarySearchStart;
}

static Bool ParseInlineBytecodeStringToCStringInTokenBuffer(ForthVm* vm, Cell** readPtr) {
	Cell sizeInBytes = *((*readPtr)++);
	char* charCastDest = vm->tokenBuffer;
	char* charCast = (char*)*readPtr;
	for (int i = 0; i < sizeInBytes; i++) {
		charCastDest[i] = charCast[i];
	}
	charCastDest[sizeInBytes] = '\0';
	Cell cellsAdvanceRequired = sizeInBytes % sizeof(Cell) ? (sizeInBytes / sizeof(Cell)) + 1 : sizeInBytes / sizeof(Cell);
	*readPtr += cellsAdvanceRequired;
}

static Bool IsPrimitive(ExecutionToken token) {
	return (token >= 0) && (token < NumPrimitives);
}

static void PrintInlineBytecodeStringAdvancingReadPointer(const ForthVm* vm, Cell** readPtr, const char* endSequence) {
	int length = *(*readPtr)++;
	int adjustedLength = length % sizeof(Cell) ? (length / sizeof(Cell)) + 1 : length / sizeof(Cell);
	const char* charPtr = *readPtr;
	for (int i = 0; i < length; i++) {
		vm->putchar(charPtr[i]);
	}
	while (*endSequence != '\0') {
		vm->putchar(*endSequence++);
	}
	*readPtr += adjustedLength;
}

static void PrintCompiledWordContents(const ForthVm* vm, Cell* readPtr) {
	ForthPrint(vm, "\tbytecode: ");
	while (((ForthDictHeader*)*readPtr)->data[0] != Return) {
		ForthDictHeader* token = (ForthDictHeader*)(*readPtr++);
		vm->printf("%s ", token->name);
		int length;
		int adjustedLength;
		switch (token->data[0]) {
		BCase NumLiteral:
		case Branch:
		case Branch0: // intentional fallthrough
			vm->printf("%i ", *readPtr++);
		BCase StringLiteral:
			PrintInlineBytecodeStringAdvancingReadPointer(vm, &readPtr, "\" ");
		BCase SearchForAndPushExecutionToken:
			PrintInlineBytecodeStringAdvancingReadPointer(vm, &readPtr, " ");
		}
	}
	ForthPrint(vm, "return\n");
}

static void PrintDictionaryContents(const ForthVm* vm) {
	const ForthDictHeader* item = vm->dictionarySearchStart;
	int i = 0;
	while (item->previous != NULL) {
		vm->printf("%i.) %s \n",i++ ,item->name);
		vm->printf("\tImmediate: %s\n", item->isImmediate ? "true" : "false");
		if (item->data[0] == EnterWord && !StringCompare(item->name, "enter")) {
			Cell* readPtr = item->data[1];
			PrintCompiledWordContents(vm, readPtr);
		}
		else {
			ForthPrint(vm,"\tprimitive\n");
		}
		ForthPrint(vm,"\n");
		item = ((const ForthDictHeader*)item->previous);
	}
	size_t dictionaryBytes = (char*)vm->memoryTop - (char*)vm->memory;
	size_t capacity = vm->maxMemorySize * sizeof(Cell);
	vm->printf("memory usage: %i / %i bytes. %f percent memory used\n", dictionaryBytes, capacity,
		((float)dictionaryBytes / (float) capacity) * 100.0f);
}

static ExecutionToken SearchForToken(ForthVm* vm) {
	ForthDictHeader* item = vm->dictionarySearchStart;
	while (item != NULL) {
		if (StringCompare(item->name, vm->tokenBuffer)) {
			return item;
		}
		item = item->previous;
	}
	return NULL;
}

static void AddPrimitiveToDict(ForthVm* vm, PrimitiveWordTokenValues primitive, const char* forthName, Bool isImmediate) {
	ForthDictHeader item;
	StringCopy(item.name, forthName);
	item.isImmediate = isImmediate;
	ForthDictHeader* newItem = (ForthDictHeader*)vm->memoryTop;
	*newItem = item;

	// link in new word
	newItem->previous = vm->dictionarySearchStart;
	vm->dictionarySearchStart = newItem;

	// point data to the body and compile a single primitive
	vm->memoryTop += sizeof(ForthDictHeader) / sizeof(Cell); // need to align to make portable
	newItem->data = vm->memoryTop;
	newItem->data[0] = primitive;
	vm->memoryTop++;
}

static void PrintStack(const ForthVm* vm, Cell* stack, Cell* stackTop, const char* stackName) {
	ForthPrint(vm, stackName);
	Cell* readPtr = stack;
	ForthPrint(vm, "[ ");
	while (readPtr != stackTop) {
		if (readPtr + 1 == stackTop) {
			vm->printf("%i", *(readPtr++));
		}
		else {
			vm->printf("%i, ", *(readPtr++));
		}
	}
	ForthPrint(vm, " ]\n");
}

static void PrintIntStack(const ForthVm* vm) {
	PrintStack(vm, vm->intStack, vm->intStackTop, "int stack:    ");
}

static void PrintReturnStack(const ForthVm* vm) {
	PrintStack(vm, vm->returnStack, vm->returnStackTop, "return stack: ");
}

static Bool InnerInterpreter(ForthVm* vm){
	Cell* initialReturnStack = vm->returnStackTop;
	ExecutionToken token, item;
	Cell cell1, cell2, cell3, cell4; // top four of the stack commonly used, can't declare locals in case
	const char* nextTokenReadPtr = NULL;
	char* dictionaryNameWritePtr = NULL;
	do {
		token = *vm->instructionPointer++;
		switch ((PrimitiveWordTokenValues)token->data[0]) {
		BCase Return:
			vm->instructionPointer = PopReturnStack(vm);
		BCase Add:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 + cell1);
		BCase Subtract:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 - cell1);
		BCase Divide:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 / cell1);
		BCase Multiply:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 * cell1);
		BCase Modulo:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 % cell1);
		BCase Branch0:
			cell1 = PopIntStack(vm);
			if (cell1 == 0) {
				Cell offset = *vm->instructionPointer;
				vm->instructionPointer += offset;
			}
			else {
				vm->instructionPointer++; // skip over offset
			}
		BCase Branch:
			vm->instructionPointer += *vm->instructionPointer;
		BCase Dup:
			cell1 = PopIntStack(vm);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell1);
		BCase Swap:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell2);
		BCase Rot:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			cell3 = PopIntStack(vm);
			PushIntStack(vm, cell2);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell3);
		BCase NumLiteral:
			PushIntStack(vm, *vm->instructionPointer);
			vm->instructionPointer++;
		BCase Emit:
			cell1 = PopIntStack(vm);
			vm->putchar(cell1);
		BCase Fetch:
			cell1 = PopIntStack(vm);
			PushIntStack(vm, *((Cell*)cell1));
		BCase Store:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			*((Cell*)cell1) = cell2;
		BCase ByteFetch:
			cell1 = PopIntStack(vm);
			PushIntStack(vm, ((Cell)*((char*)cell1)));
		BCase ByteStore:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			*((char*)cell1) = (char)cell2;
		BCase SearchForAndPushExecutionToken:
			ParseInlineBytecodeStringToCStringInTokenBuffer(vm, &vm->instructionPointer);
			cell1 = SearchForToken(vm);
			PushIntStack(vm, cell1);
		BCase Here:
			PushIntStack(vm, vm->memoryTop);
		BCase Allot:
			cell1 = PopIntStack(vm);
			((char*)vm->memoryTop) += cell1;
		BCase Colon:
			if (vm->currentMode & Forth_CompileBit) {
				ForthPrint(vm, "you're already in colon compile mode");
				return True;
			}
			vm->currentMode |= Forth_CompileBit;
			item = (ForthDictHeader*)vm->memoryTop;
			CopyStringUntilSpaceCappingWithNull(item->name, vm->nextTokenStart);
			item->isImmediate = False;
			vm->memoryTop += (sizeof(ForthDictHeader) / sizeof(Cell));
			item->data = vm->memoryTop;
			item->data[0] = EnterWord;
			item->data[1] = &item->data[2];
			vm->memoryTop += 2;
			item->previous = vm->dictionarySearchStart;
			vm->dictionarySearchStart = item;
			LoadNextToken(vm);
		BCase SemiColon:
			if ((vm->currentMode & Forth_CompileBit) == 0) {
				ForthPrint(vm, "you're not in colon compile mode");
			}
			// compile a return token
			StringCopy(vm->tokenBuffer, "return");
			*(vm->memoryTop++) = SearchForToken(vm);
			// take us out of compile mode
			vm->currentMode &= ~(Forth_CompileBit);
		BCase Show:
			PrintIntStack(vm);
			PrintReturnStack(vm);
		BCase ShowWords:
			PrintDictionaryContents(vm);
		BCase Immediate:
			if (vm->currentMode & Forth_CompileBit) {
				ForthPrint(vm, "you can't use the word \"immediate\" when compiling a colon word");
			}
			LastWordAdded(vm)->isImmediate = True;
		BCase Create:
			item = (ForthDictHeader*)vm->memoryTop;
			CopyStringUntilSpaceCappingWithNull(item->name, vm->nextTokenStart);
			vm->memoryTop += sizeof(ForthDictHeader) / sizeof(Cell);
			item->data = vm->memoryTop;
			item->previous = vm->dictionarySearchStart;
			vm->dictionarySearchStart = item;
			LoadNextToken(vm);
		BCase Equals:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, (cell1 == cell2) ? -1 : 0);
		BCase GreaterThan:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, (cell2 > cell1) ? -1 : 0);
		BCase LessThan:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, (cell2 < cell1) ? -1 : 0);
		BCase And:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, ((cell2 != 0) && (cell1 != 0)) ? -1 : 0);
		BCase Or:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, ((cell2 != 0) || (cell1 != 0)) ? -1 : 0);
		BCase Not:
			cell1 = PopIntStack(vm);
			PushIntStack(vm, (cell1 != 0) ? 0 : -1);
		BCase CellSize:
			PushIntStack(vm, sizeof(Cell));
		BCase CommentStart:
			vm->currentMode |= Forth_CommentFlag;
		BCase CommentStop:
			vm->currentMode &= ~(Forth_CommentFlag);
		BCase Drop:
			PopIntStack(vm);
		BCase PushReturnStackWord:
			cell1 = PopIntStack(vm);
			PushReturnStack(vm, cell1);
		BCase PopReturnStackWord:
			cell1 = PopReturnStack(vm);
			PushIntStack(vm, cell1);
		BCase TwoDup:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell2);
			PushIntStack(vm, cell1);
		BCase TwoSwap:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			cell3 = PopIntStack(vm);
			cell4 = PopIntStack(vm);
			PushIntStack(vm, cell2);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell4);
			PushIntStack(vm, cell3);
		BCase I: // these two could be done in forth but we (I) want them to be fast
			cell1 = vm->returnStackTop[-2];
			PushIntStack(vm, cell1);
		BCase J:
			cell1 = vm->returnStackTop[-4];
			PushIntStack(vm, cell1);
		BCase FullStop:
			cell1 = PopIntStack(vm);
			vm->printf("%i", cell1);
		BCase SearchForAndPushExecutionTokenCompileTime:
			StringCopy(vm->tokenBuffer, "r'");
			item = SearchForToken(vm);
			*(vm->memoryTop++) = item;
			CompileCStringToForthByteCode(vm, vm->nextTokenStart, ' ');
			if (!LoadNextToken(vm)) {
				//return True;
			}
		BCase StringLiteral:
			cell1 = *vm->instructionPointer++; // size of string in bytes
			PushIntStack(vm, cell1);
			PushIntStack(vm, vm->instructionPointer); // ptr to string
			cell2 = cell1 % sizeof(Cell) ? (cell1 / sizeof(Cell)) + 1 : cell1 / sizeof(Cell); // number to advance ip by
			vm->instructionPointer += cell2;
		BCase StringLiteralCompileTime:
			StringCopy(vm->tokenBuffer, "sr\"");
			item = SearchForToken(vm);
			*vm->memoryTop++ = item;
			cell1 = CompileCStringToForthByteCode(vm, vm->nextTokenStart, '"');
			vm->nextTokenStart += cell1 + 2;
		BCase EnterWord:
			PushReturnStack(vm, vm->instructionPointer);
			vm->instructionPointer = token->data[1];
		BCase CallC:
			((ForthCFunc)token->data[1])(vm);
		BCase Key:
			PushIntStack(vm, vm->getchar());
		BCase Body:
			cell1 = PopIntStack(vm);
			PushIntStack(vm, ((ForthDictHeader*)cell1)->data);
		BCase SwitchToCompile:
			vm->currentMode |= Forth_CompileBit;
		BCase SwitchToInterpret:
			vm->currentMode &= ~Forth_CompileBit;
		}
	} while (vm->returnStackTop != initialReturnStack);
	return False;
}

static int CompileCStringToForthByteCode(ForthVm* vm, const char* string, char delim) {
	Cell* length = vm->memoryTop++; // save to back-patch with length afterwards;
	const char* readPtr = string;
	char* writePtr = (char*)vm->memoryTop;
	int stringLen = 0;
	while (*readPtr != delim) {
		*writePtr++ = *readPtr++;
		stringLen++;
	}
	Cell cellsRequired = stringLen % sizeof(Cell) ? (stringLen / sizeof(Cell)) + 1 : stringLen / sizeof(Cell);
	*length = stringLen;
	vm->memoryTop += cellsRequired;
	return stringLen;
}

static Bool LoadNextToken(ForthVm* vm) {
	int len = CopyNextToken(vm->nextTokenStart, vm->tokenBuffer);
	vm->nextTokenStart += len;
	while (*vm->nextTokenStart == ' ')vm->nextTokenStart++;
	if (len == 0) {
		return False; // stream has finished
	}
	else {
		return True; // continue reading stream
	}
}

static void OuterInterpreter(ForthVm* vm, const char* input) {
	vm->nextTokenStart = input;
	StringCopy(vm->tokenBuffer, "lit");
	ExecutionToken lit = SearchForToken(vm);
	if (!LoadNextToken(vm)) {
		return;
	}
	while (*vm->tokenBuffer != "\0") {
		ExecutionToken foundToken = SearchForToken(vm);
		vm->instructionPointer = &foundToken;
		if (foundToken != NULL) {
			if (vm->currentMode & Forth_CompileBit) {
				if ((vm->currentMode & Forth_CommentFlag) == 0) {
					if (foundToken->isImmediate) {
						InnerInterpreter(vm);
					}
					else {
						*(vm->memoryTop++) = foundToken;
					}
				}
				else if (foundToken->data[0] == CommentStop) {
					InnerInterpreter(vm);
				}
			}
			else {
				InnerInterpreter(vm);
			}
		}
		else if((vm->currentMode & Forth_CommentFlag) == 0){
			Cell converted = ForthAtoi(vm->tokenBuffer);
			if (vm->currentMode & Forth_CompileBit) {
				// compile number literal
				*(vm->memoryTop++) = lit;
				*(vm->memoryTop++) = converted;
			}
			else {
				PushIntStack(vm, converted);
			}
		}
		if (!LoadNextToken(vm)) {
			return;
		}
	}
}

static const char* coreWords =
//"create : r' enter here ! 1 cell * allot here 1 cell * + here ! 1 cell * allot ( boiler plate created to call function ) r' ] here ! 1 cell * allot r' create here ! 1 cell * allot r' enter here ! 1 cell * allot here 1 cell * + here ! "
//"showWords "
// helpers
": allotCell cell * allot ; "

": , ( valueToCompile -- ) here ! 1 allotCell ; "

": cells cell / ; "

": branchOffsetInCells ( addressOfBranch -- offsetToTheAddressInCellsFromCurrentDictPointer ) "
	"here swap - ( calculate offset in bytes ) "
	"cells ( convert to offset in cells ) "
"; "

": backPatch ( addressOfBranch -- ) "
	"branchOffsetInCells "
	"swap ! ( store the offset at the address to be back-patched ) "
"; "

// immediate words - these compile themselves down into control flow structures made up of branch and branch0 only
// and these automate the setting of branch offsets by running at compile time as denoted by "immediate"
": if "
	"' branch0 , ( compile a branch0 token ) "
	"here ( 'here' now points to where the compiled ifs branch offset will go, push it to be back-patched by then or else ) "
	"1 allotCell ( advance top of memory to compile next part of the program ) "
"; immediate "

": then "
	"dup ( duplicate the address that will be set with the branch offset ) "
	"backPatch "
"; immediate "

": else "
	"' branch , ( compile branch token ) "
	"here ( the top of memory now points to the cell where the branch tokens offset is, push it so it can be back - patched by then ) "
	"1 allotCell ( move to the cell after, we will calculate the offset to this cell to back patch the if ) "
	"swap ( swap so that the if tokens branch offset address is on top ) "
	"dup ( two copies of the if's branch offset address are on top, and under that is this elses branch offset address ) "
	"backPatch "
"; immediate "

// loops 
": begin "
	"here "
"; immediate "

": until "
	"' branch0 , ( compile a branch0 token ) "
	"branchOffsetInCells "
	"-1 * "
	"here ! "
	"1 allotCell ( advance top of memory to compile next part of the program ) "
"; immediate "
/*
*  compiler generated do / loop pseudo code
* https://sincereflattery.blog/2020/07/08/i-do-forth/
* *******************************************

   jump to test
start:
	save indexes to return stack     // do is up to and including this line

	loop content

	retrieve indexes from return stack // loop starts here
	add one to index
test:
	if index has not reached the end, jump to start
exit:
	clean up of indexes //ends here
	rest of code

*/
": do "
	"' branch , "
	"here ( label of initial jump ) "
	"1 allotCell "
	"here ( start label in pseudo code above ) "
	"swap "
	"' R , ( compile code to push i onto return stack ) "
	"' R , ( compile code to push limit onto return stack ) "
"; immediate "

": loop "
	"( compile code to pop i and limit from return stack ) "
	"' R> , "
	"' R> , "

	"( compile code to increment i ) "
	"' lit , "
	"1 , "
	"' + , "

	"( we are now at the test label ) "
	"dup "
	"backPatch "

	"( compile code to compare i and limit and branch if not equal ) "
	"' 2dup , "
	"' = , "
	"' branch0 , "
	"here "
	"- cell "
	"/ , "

	"( compile code to clean up i and limit from int stack now that the loop has ended ) "
	"' drop , "
	"' drop , "
"; immediate "

// other misc words

": cr 10 emit ; "

": print ( length address -- ) "
	"swap "
	"0 do "
		"dup i + c@ emit "
	"loop drop "
"; "


;

void Forth_RegisterCFunc(ForthVm* vm, ForthCFunc function, const char* name, Bool isImmediate) {
	ForthDictHeader item;
	StringCopy(item.name, name);
	item.isImmediate = isImmediate;
	ForthDictHeader* newItem = (ForthDictHeader*)vm->memoryTop;
	*newItem = item;

	// link in new word
	newItem->previous = vm->dictionarySearchStart;
	vm->dictionarySearchStart = newItem;

	// point data to the body and compile a CallC primitive and C function ptr address
	vm->memoryTop += sizeof(ForthDictHeader) / sizeof(Cell); // need to align to make portable
	newItem->data = vm->memoryTop;
	newItem->data[0] = CallC;
	newItem->data[1] = function;
	vm->memoryTop += 2;
}

ForthVm Forth_Initialise(
	Cell* memoryForCompiledWordsAndVariables,
	size_t memorySize,
	Cell* intStack,
	size_t intStackSize,
	Cell* returnStack,
	size_t returnStackSize,
	ForthPrintf printf,
	ForthPutChar putc,
	ForthGetChar getc) {

	ForthVm vm;
	
	vm.dictionarySearchStart = NULL;

	vm.currentMode = 0;

	vm.memory = memoryForCompiledWordsAndVariables;
	vm.memoryTop = vm.memory;
	vm.maxMemorySize = memorySize;

	vm.intStack = intStack;
	vm.intStackTop = vm.intStack;
	vm.maxIntStackSize = intStackSize;

	vm.returnStack = returnStack;
	vm.returnStackTop = vm.returnStack;
	vm.maxReturnStackSize = returnStackSize;

	vm.printf = printf;
	vm.putchar = putc;
	vm.getchar = getc;

	vm.instructionPointer = vm.memory;

	AddPrimitiveToDict(&vm, Return,                                    "return",    False);
	AddPrimitiveToDict(&vm, Add,                                       "+",         False);
	AddPrimitiveToDict(&vm, Subtract,                                  "-",         False);
	AddPrimitiveToDict(&vm, Divide,                                    "/",         False);
	AddPrimitiveToDict(&vm, Multiply,                                  "*",         False);
	AddPrimitiveToDict(&vm, Modulo,                                    "%",         False);
	AddPrimitiveToDict(&vm, Branch0,                                   "branch0",   False);
	AddPrimitiveToDict(&vm, Branch,                                    "branch",    False);
	AddPrimitiveToDict(&vm, Dup,                                       "dup",       False);
	AddPrimitiveToDict(&vm, Swap,                                      "swap",      False);
	AddPrimitiveToDict(&vm, Rot,                                       "rot",       False);
	AddPrimitiveToDict(&vm, NumLiteral,                                "lit",       False);
	AddPrimitiveToDict(&vm, Emit,                                      "emit",      False);
	AddPrimitiveToDict(&vm, Fetch,                                     "@",         False);
	AddPrimitiveToDict(&vm, Store,                                     "!",         False);
	AddPrimitiveToDict(&vm, ByteFetch,                                 "c@",        False);
	AddPrimitiveToDict(&vm, ByteStore,                                 "c!",        False);
	AddPrimitiveToDict(&vm, SearchForAndPushExecutionToken,            "r'",        False);
	AddPrimitiveToDict(&vm, Here,                                      "here",      False);
	AddPrimitiveToDict(&vm, Allot,                                     "allot",     False);
	AddPrimitiveToDict(&vm, Colon,                                     ":",         False);
	AddPrimitiveToDict(&vm, SemiColon,                                 ";",         True);
	AddPrimitiveToDict(&vm, Show,                                      "show",      False);
	AddPrimitiveToDict(&vm, ShowWords,                                 "showWords", False);
	AddPrimitiveToDict(&vm, Immediate,                                 "immediate", False);
	AddPrimitiveToDict(&vm, Create,                                    "create",    False);
	AddPrimitiveToDict(&vm, Equals,                                    "=",         False);
	AddPrimitiveToDict(&vm, GreaterThan,                               ">",         False);
	AddPrimitiveToDict(&vm, LessThan,                                  "<",         False);
	AddPrimitiveToDict(&vm, And,                                       "and",       False);
	AddPrimitiveToDict(&vm, Or,                                        "or",        False);
	AddPrimitiveToDict(&vm, Not,                                       "not",       False);
	AddPrimitiveToDict(&vm, CellSize,                                  "cell",      False);
	AddPrimitiveToDict(&vm, CommentStart,                              "(",         True);
	AddPrimitiveToDict(&vm, CommentStop,                               ")",         True);
	AddPrimitiveToDict(&vm, Drop,                                      "drop",      False);
	AddPrimitiveToDict(&vm, PushReturnStackWord,                       "R",         False);
	AddPrimitiveToDict(&vm, PopReturnStackWord,                        "R>",        False);
	AddPrimitiveToDict(&vm, TwoDup,                                    "2dup",      False);
	AddPrimitiveToDict(&vm, TwoSwap,                                   "2swap",     False);
	AddPrimitiveToDict(&vm, I,                                         "i",         False);
	AddPrimitiveToDict(&vm, J,                                         "j",         False);
	AddPrimitiveToDict(&vm, FullStop,                                  ".",         False);
	AddPrimitiveToDict(&vm, SearchForAndPushExecutionTokenCompileTime, "'",         True);
	AddPrimitiveToDict(&vm, StringLiteral,                             "sr\"",      False);
	AddPrimitiveToDict(&vm, StringLiteralCompileTime,                  "s\"",       True);
	AddPrimitiveToDict(&vm, EnterWord,                                 "enter",     False);
	AddPrimitiveToDict(&vm, CallC,                                     "call",      False); // it's not really necessary to add these last two as primitives here but I have done anyway
	AddPrimitiveToDict(&vm, Key,                                       "key",       False);
	AddPrimitiveToDict(&vm, Body,                                      ">body",     False);
	AddPrimitiveToDict(&vm, SwitchToCompile,                           "]",         False);
	AddPrimitiveToDict(&vm, SwitchToInterpret,                         "[",         True);

	// load core vocabulary of words that are not primitive, ie are defined in forth
	OuterInterpreter(&vm, coreWords);

	return vm;
}
void Forth_Teardown()
{
}

Bool Forth_DoString(ForthVm* vm, const char* inputString)
{
	OuterInterpreter(vm, inputString);
	return True;
}
