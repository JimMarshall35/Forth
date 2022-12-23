#include "Forth2.h"
#include "StringUtils.h"
/*
*
* Forth - Jim Marshall - 2022
* jimmarshall35@gmail.com
* 
* *********************************************************

*/
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
	ExecuteToken,
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

	NumPrimitives // LEAVE AT END
}PrimitiveWordTokenValues;

// forward declarations
ExecutionToken SearchForToken(ForthVm* vm);
Bool LoadNextToken(ForthVm* vm); // to be used at compile time only


#define PopIntStack(vm) *(--vm->intStackTop)
#define PushIntStack(vm, val) *(vm->intStackTop++) = val;

#define PopReturnStack(vm) *(--vm->returnStackTop)
#define PushReturnStack(vm, val) *(vm->returnStackTop++) = val

void PushDictionaryWord(ForthVm* vm, const DictionaryItem* item) {
	DictionaryItem* newItem = (DictionaryItem*)vm->memoryTop;
	*newItem = *item;

	newItem->previous = vm->dictionarySearchStart;
	vm->dictionarySearchStart = newItem;

	vm->memoryTop += sizeof(DictionaryItem) / sizeof(Cell); // need to align to make portable
}

DictionaryItem* LastWordAdded(ForthVm* vm) {
	return (vm->dictionarySearchStart == NULL) ? vm->memory : vm->dictionarySearchStart;
}

Bool ParseInlineBytecodeStringToCStringInTokenBuffer(ForthVm* vm, Cell** readPtr) {
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

Bool IsPrimitive(ExecutionToken token) {
	return (token >= 0) && (token < NumPrimitives);
}

void PrintDictionaryContents(const ForthVm* vm) {
	// I don't like how long this function is
	const DictionaryItem* item = vm->dictionarySearchStart;
	int i = 0;
	StringCopy(vm->tokenBuffer, "return");
	const DictionaryItem* returnItem = SearchForToken(vm);
	StringCopy(vm->tokenBuffer, "lit");
	const DictionaryItem* literalItem = SearchForToken(vm);
	StringCopy(vm->tokenBuffer, "'");
	const DictionaryItem* findWordItem = SearchForToken(vm);

	while (item->previous != NULL) {
		vm->printf("%i.) %s \n",i++ ,item->name);
		vm->printf("\tImmediate: %s\n", item->isImmediate ? "true" : "false");
		if (item->type == ColonWord) {
			vm->printf("\ttype: ColonWord\n");
			vm->printf("\tbytecode:\n\t");
			Cell* wordStart = item->data;
			Bool nextIsLit = False;
			while (*wordStart != returnItem) {
			    ExecutionToken token = *wordStart++;
				if (token == literalItem) {
					vm->printf("%s ", literalItem->name);
					token = *wordStart++;
					vm->printf("%i ", token);
				}
				else if (token == findWordItem) {
					vm->printf("%s ", findWordItem->name);
					ParseInlineBytecodeStringToCStringInTokenBuffer(vm, &wordStart);
					vm->printf("%s ", vm->tokenBuffer);
				}
				else {
					vm->printf("%s ", token->name);
				}
			}
			vm->printf("%s", "return");
		}
		else if (item->type == Primitive) {
			vm->printf("\ttype: Primitive\n");
		}
		vm->printf("\n");
		item = ((const DictionaryItem*)item->previous);
	}
}

ExecutionToken SearchForToken(ForthVm* vm) {
	DictionaryItem* item = vm->dictionarySearchStart;
	while (item != NULL) {
		if (StringCompare(item->name, vm->tokenBuffer)) {
			return item;
		}
		item = item->previous;
	}
	return NULL;
}

void AddPrimitiveToDict(ForthVm* vm, PrimitiveWordTokenValues primitive, const char* forthName, Bool isImmediate) {
	DictionaryItem item;
	StringCopy(item.name, forthName);
	item.isImmediate = isImmediate;
	item.type = Primitive;
	item.data = primitive;
	PushDictionaryWord(vm, &item);
}

void PrintStack(const ForthVm* vm, Cell* stack, Cell* stackTop, const char* stackName) {
	vm->printf(stackName);
	Cell* readPtr = stack;
	while (readPtr != stackTop) {
		if (readPtr + 1 == stackTop) {
			vm->printf("%i", *(readPtr++));
		}
		else {
			vm->printf("%i, ", *(readPtr++));
		}
	}
	vm->printf(" ]\n");
}

void PrintIntStack(const ForthVm* vm) {
	PrintStack(vm, vm->intStack, vm->intStackTop, "int stack:    [ ");
}

void PrintReturnStack(const ForthVm* vm) {
	PrintStack(vm, vm->returnStack, vm->returnStackTop, "return stack: [ ");
}

Bool InnerInterpreter(ForthVm* vm){// iftokenVal < 0, then interpreter mode, don't push to return stack just execute the token tokenVal
	Cell* initialReturnStack = vm->returnStackTop;
	ExecutionToken token, item;
	Cell cell1, cell2, cell3, cell4; // top three of the stack commonly used, can't declare locals in case
	const char* nextTokenReadPtr = NULL;
	char* dictionaryNameWritePtr = NULL;
	do {
		token = *vm->instructionPointer++;
noIncrement:
		switch ((PrimitiveWordTokenValues)token->data) {
		// fancy new switch case break formatting technique - on same line as case so you can't forget it
		break; case Return:
			vm->instructionPointer = PopReturnStack(vm);
		break; case Add:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 + cell1);
		break; case Subtract:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 - cell1);
		break; case Divide:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 / cell1);
		break; case Multiply:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 * cell1);
		break; case Modulo:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2 % cell1);
		break; case Branch0:
			cell1 = PopIntStack(vm);
			if (cell1 == 0) {
				int offset = *(vm->instructionPointer + 1);
				vm->instructionPointer += offset;
			}
			else {
				vm->instructionPointer += 2; // skip over offset
			}
		break; case Branch:
			vm->instructionPointer += *(vm->instructionPointer + 1);
		break; case Dup:
			cell1 = PopIntStack(vm);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell1);
		break; case Swap:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell2);
		break; case Rot:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			cell3 = PopIntStack(vm);
			PushIntStack(vm, cell2);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell3);
		break; case NumLiteral:
			PushIntStack(vm, *vm->instructionPointer); // push literal value at ip
			vm->instructionPointer++;                  // skip over literal value
		break; case Emit:
			cell1 = PopIntStack(vm);
			vm->putchar(cell1);
		break; case Fetch:
			cell1 = PopIntStack(vm);
			PushIntStack(vm, *((Cell*)cell1));
		break; case Store:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			*((Cell*)cell1) = cell2;
		break; case ByteFetch:
			cell1 = PopIntStack(vm);
			PushIntStack(vm, ((Cell)*((char*)cell1)));
		break; case ByteStore:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			*((char*)cell1) = (char)cell2;
		break; case SearchForAndPushExecutionToken:
			ParseInlineBytecodeStringToCStringInTokenBuffer(vm, &vm->instructionPointer);
			cell1 = SearchForToken(vm, vm->scratchPadMemory);
			PushIntStack(vm, cell1);
		break; case ExecuteToken:
			// not sure how to implement yet
		break; case Here:
			PushIntStack(vm, vm->memoryTop);
		break; case Allot:
			cell1 = PopIntStack(vm);
			vm->memoryTop += cell1;
		break; case Colon:
			if (vm->currentMode & Forth_InColonDefinitionBit) {
				vm->printf("you're already in colon compile mode");
				return True;
			}
			if (vm->currentMode & Forth_CompileBit) {
				vm->printf("something's gone badly wrong - you're not in a colon definition but are in compile mode somehow");
				return True;
			}
			vm->currentMode |= Forth_InColonDefinitionBit;
			vm->currentMode |= Forth_CompileBit;
			item = (DictionaryItem*)vm->memoryTop;
			vm->tempDictionaryItemPointer = vm->memoryTop;
			CopyStringUntilSpaceCappingWithNull(item->name, vm->nextTokenStart);
			item->data = vm->memoryTop + (sizeof(DictionaryItem) / sizeof(Cell));
			item->isImmediate = False;
			item->type = ColonWord;
			vm->memoryTop += (sizeof(DictionaryItem) / sizeof(Cell));
			// don't increment dictionaryTop until the semicolon
			LoadNextToken(vm);
		break; case SemiColon:
			if ((vm->currentMode & Forth_InColonDefinitionBit) == 0) {
				vm->printf("you're not in colon compile mode");
			}
			// compile a return token
			StringCopy(vm->tokenBuffer, "return");
			*(vm->memoryTop++) = SearchForToken(vm);
			// "reveal" the new word, linking it into the dictionary
			item = vm->tempDictionaryItemPointer;
			item->previous = vm->dictionarySearchStart;
			vm->dictionarySearchStart = item;
			vm->memoryTop += (sizeof(DictionaryItem) / sizeof(Cell));//need to align somehow but not now i know it will be 
			// take us out of colon definition mode and its child mode, compile mode
			vm->currentMode &= ~(Forth_InColonDefinitionBit);
			vm->currentMode &= ~(Forth_CompileBit);
		break; case Show:
			PrintIntStack(vm);
			PrintReturnStack(vm);
		break; case ShowWords:
			PrintDictionaryContents(vm);
		break; case Immediate:
			if (vm->currentMode & Forth_InColonDefinitionBit) {
				vm->printf("you can't use the word \"immediate\" when compiling a colon word");
			}
			LastWordAdded(vm)->isImmediate = True;
		break; case Create:
			if (vm->currentMode & Forth_CompileBit) {
				item = (DictionaryItem*)vm->memoryTop;
				CopyStringUntilSpaceCappingWithNull(item->name, vm->nextTokenStart);
				
				item->type = Variable;
				vm->memoryTop += sizeof(DictionaryItem) / sizeof(Cell);
				item->data = vm->memoryTop;
				LoadNextToken(vm);
			}
			else {
				// need to implement
			}
		break; case Equals:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, (cell1 == cell2) ? -1 : 0);
		break; case GreaterThan:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, (cell2 > cell1) ? -1 : 0);
		break; case LessThan:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, (cell2 < cell1) ? -1 : 0);
		break; case And:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, ((cell2 != 0) && (cell1 != 0)) ? -1 : 0);
		break; case Or:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, ((cell2 != 0) || (cell1 != 0)) ? -1 : 0);
		break; case Not:
			cell1 = PopIntStack(vm);
			PushIntStack(vm, (cell1 != 0) ? 0 : -1);
		break; case CellSize:
			PushIntStack(vm, sizeof(Cell));
		break; case CommentStart:
			vm->currentMode |= Forth_CommentFlag;
		break; case CommentStop:
			vm->currentMode &= ~(Forth_CommentFlag);
		break; case Drop:
			PopIntStack(vm);
		break; case PushReturnStackWord:
			cell1 = PopIntStack(vm);
			PushReturnStack(vm, cell1);
		break; case PopReturnStackWord:
			cell1 = PopReturnStack(vm);
			PushIntStack(vm, cell1);
		break; case TwoDup:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			PushIntStack(vm, cell2);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell2);
			PushIntStack(vm, cell1);
		break; case TwoSwap:
			cell1 = PopIntStack(vm);
			cell2 = PopIntStack(vm);
			cell3 = PopIntStack(vm);
			cell4 = PopIntStack(vm);
			PushIntStack(vm, cell2);
			PushIntStack(vm, cell1);
			PushIntStack(vm, cell4);
			PushIntStack(vm, cell3);
		break; case I: // these two could be done in forth but we (I) want them to be fast
			cell1 = vm->returnStackTop[-2];
			PushIntStack(vm, cell1);
		break; case J:
			cell1 = vm->returnStackTop[-4];
			PushIntStack(vm, cell1);
		break; default:
			while (token->type != Primitive) {
				// push instruction pointer for later return from this non primitive token
				switch (token->type) {
				break; case ColonWord:
					PushReturnStack(vm, vm->instructionPointer); // push ip
					vm->instructionPointer = token->data; // jump to new word start
				break; case Variable: // intentional fall through
				case Constant:
					PushIntStack(vm, token->data);
					return True;
				}
				token = *vm->instructionPointer++;
			}
			goto noIncrement;
		}
		
	} while (vm->returnStackTop != initialReturnStack);
	return False;
}

void CompileCStringToForthByteCode(ForthVm* vm, const char* string, char delim) {
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
}

Bool LoadNextToken(ForthVm* vm) {
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

void OuterInterpreter(ForthVm* vm, const char* input) {
	vm->nextTokenStart = input;
	if (!LoadNextToken(vm)) {
		return;
	}
	while (*vm->tokenBuffer != "\0") {
		ExecutionToken foundToken = SearchForToken(vm);
		vm->scratchPadTop = vm->scratchPadMemory;
		if (foundToken != NULL) {
			if (vm->currentMode & Forth_InColonDefinitionBit) {
				if ((vm->currentMode & Forth_CommentFlag) == 0) {
					if ((foundToken->isImmediate || ((vm->currentMode & Forth_CompileBit) == 0))) {
						vm->instructionPointer = &foundToken;
						InnerInterpreter(vm, foundToken);
					}
					else {

						*(vm->memoryTop++) = foundToken; // compile token
						if (StringCompare(foundToken->name, "'")) {
							CompileCStringToForthByteCode(vm, vm->nextTokenStart, ' ');
							if (!LoadNextToken(vm)) {
								return;
							}
						}
					}
				}
				else if (foundToken->data == CommentStop) {
					vm->instructionPointer = &foundToken;
					InnerInterpreter(vm);
				}
			}
			else {
				vm->instructionPointer = &foundToken;
				InnerInterpreter(vm);
			}
		}
		else if((vm->currentMode & Forth_CommentFlag) == 0){
			Cell converted = atoi(vm->tokenBuffer);
			if (vm->currentMode & Forth_CompileBit) {
				// compile number literal
				StringCopy(vm->tokenBuffer, "lit");
				*(vm->memoryTop++) = SearchForToken(vm);;
				*(vm->memoryTop++) = converted;
			}
			else {
				// try convert token string to number and push it (real good)
				PushIntStack(vm, converted);
			}
		}
		if (!LoadNextToken(vm)) {
			return;
		}
	}
}

const char* coreWords =
// helpers

": compile ( valueToCompile -- ) here ! 1 allot ; "

": cells cell / ; "

": branchOffsetInCells ( addressOfBranch -- offsetToTheAddressInCellsFromCurrentDictPointer ) "
	"here swap - ( calculate offset in bytes ) "
	"cells ( convert to offset in cells ) "
"; "

": backPatch ( addressOfBranch -- ) "
	"branchOffsetInCells "
	"1 + ( add one cell for some reasom ) "
	"swap ! ( store the offset at the address to be back-patched ) "
"; "

// immediate words - these compile themselves down into control flow structures made up of branch and branch0 only
// and these automate the setting of branch offsets by running at compile time as denoted by "immediate"

": if "
	"' branch0 compile ( compile a branch0 token ) "
	"' lit compile ( compile a literal token because the print dictionary function expects one to be there ) "
	"here ( 'here' now points to where the compiled ifs branch offset will go, push it to be back-patched by then or else ) "
	"1 allot ( advance top of memory to compile next part of the program ) "
"; immediate "

": then "
	"dup ( duplicate the address that will be set with the branch offset ) "
	"backPatch "
"; immediate "

": else "
	"' branch compile ( compile branch token ) "
	"' lit compile ( compile a literal token for shits and giggles ) "
	"here ( the top of memory now points to the cell where the branch tokens offset is, push it so it can be back - patched by then ) "
	"1 allot ( move to the cell after, we will calculate the offset to this cell to back patch the if ) "
	"swap ( swap so that the if tokens branch offset address is on top ) "
	"dup ( two copies of the if's branch offset address are on top, and under that is this elses branch offset address ) "
	"backPatch "
"; immediate "

// loops 
": begin "
	"here show "
"; immediate "

": until "
	"' branch0 compile ( compile a branch0 token ) "
	"' lit compile ( compile a literal token for shits and giggles ) "
	"branchOffsetInCells "
	"1 - "
	"-1 * "
	"here ! "
	"1 allot show ( advance top of memory to compile next part of the program ) "
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
	if index has not reached the end, jump to start //ends here
exit:
	clean up of indexes
	rest of code

*/
": do "
	"' branch compile "
	"' lit compile "
	"here ( label of initial jump ) "
	"1 allot "
	"here ( start label in pseudo code above ) "
	"swap "
	"' R compile ( compile code to push i onto return stack ) "
	"' R compile ( compile code to push limit onto return stack ) "
"; immediate "

": loop "
	"( compile code to pop i and limit from return stack ) "
	"' R> compile "
	"' R> compile "

	"( compile code to increment i ) "
	"' lit compile "
	"1 compile "
	"' + compile "

	"( we are now at the test label ) "
	"dup "
	"backPatch "

	"( compile code to compare i and limit and branch if not equal ) "
	"' 2dup compile "
	"' = compile "
	"' branch0 compile "
	"here "
	"' lit compile "
	"- cell "
	"/ compile "

	"( compile code to clean up i and limit from int stack now that the loop has ended ) "
	"' drop compile "
	"' drop compile "
"; immediate "

;

ForthVm Forth_Initialise(
	Cell* memoryForCompiledWordsAndVariables,
	size_t memorySize,
	Cell* intStack,
	size_t intStackSize,
	Cell* returnStack,
	size_t returnStackSize,
	Cell* scratchPad,
	UCell scratchPadSize,
	ForthPrintf printf,
	ForthPutChar putc) {

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

	vm.scratchPadMemory = scratchPad;
	vm.scratchPadTop = vm.scratchPadMemory;
	vm.maxScratchPadSize = scratchPadSize;

	vm.printf = printf;
	vm.putchar = putc;

	vm.instructionPointer = vm.memory;

	vm.compileNextTokenAsString = False;

	AddPrimitiveToDict(&vm, Return,                         "return",    False);
	AddPrimitiveToDict(&vm, Add,                            "+",         False);
	AddPrimitiveToDict(&vm, Subtract,                       "-",         False);
	AddPrimitiveToDict(&vm, Divide,                         "/",         False);
	AddPrimitiveToDict(&vm, Multiply,                       "*",         False);
	AddPrimitiveToDict(&vm, Modulo,                         "%",         False);
	AddPrimitiveToDict(&vm, Branch0,                        "branch0",   False);
	AddPrimitiveToDict(&vm, Branch,                         "branch",    False);
	AddPrimitiveToDict(&vm, Dup,                            "dup",       False);
	AddPrimitiveToDict(&vm, Swap,                           "swap",      False);
	AddPrimitiveToDict(&vm, Rot,                            "rot",       False);
	AddPrimitiveToDict(&vm, NumLiteral,                     "lit",       False);
	AddPrimitiveToDict(&vm, Emit,                           "emit",      False);
	AddPrimitiveToDict(&vm, Fetch,                          "@",         False);
	AddPrimitiveToDict(&vm, Store,                          "!",         False);
	AddPrimitiveToDict(&vm, ByteFetch,                      "c@",        False);
	AddPrimitiveToDict(&vm, ByteStore,                      "c!",        False);
	AddPrimitiveToDict(&vm, SearchForAndPushExecutionToken, "'",         False);
	AddPrimitiveToDict(&vm, ExecuteToken,                   "execute",   False);
	AddPrimitiveToDict(&vm, Here,                           "here",      False);
	AddPrimitiveToDict(&vm, Allot,                          "allot",     False);
	AddPrimitiveToDict(&vm, Colon,                          ":",         False);
	AddPrimitiveToDict(&vm, SemiColon,                      ";",         True);
	AddPrimitiveToDict(&vm, Show,                           "show",      False);
	AddPrimitiveToDict(&vm, ShowWords,                      "showWords", False);
	AddPrimitiveToDict(&vm, Immediate,                      "immediate", False);
	AddPrimitiveToDict(&vm, Create,                         "create",    False);
	AddPrimitiveToDict(&vm, Equals,                         "=",         False);
	AddPrimitiveToDict(&vm, GreaterThan,                    ">",         False);
	AddPrimitiveToDict(&vm, LessThan,                       "<",         False);
	AddPrimitiveToDict(&vm, And,                            "and",       False);
	AddPrimitiveToDict(&vm, Or,                             "or",        False);
	AddPrimitiveToDict(&vm, Not,                            "not",       False);
	AddPrimitiveToDict(&vm, CellSize,                       "cell",      False);
	AddPrimitiveToDict(&vm, CommentStart,                   "(",         True);
	AddPrimitiveToDict(&vm, CommentStop,                    ")",         True);
	AddPrimitiveToDict(&vm, Drop,                           "drop",      False);
	AddPrimitiveToDict(&vm, PushReturnStackWord,            "R",         False);
	AddPrimitiveToDict(&vm, PopReturnStackWord,             "R>",        False);
	AddPrimitiveToDict(&vm, TwoDup,                         "2dup",      False);
	AddPrimitiveToDict(&vm, TwoSwap,                        "2swap",     False);
	AddPrimitiveToDict(&vm, I,                              "i",         False);
	AddPrimitiveToDict(&vm, J,                              "j",         False);

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
