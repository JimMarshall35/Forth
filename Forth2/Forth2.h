
#ifndef FORTH2
#define FORTH2
#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "CommonTypedefs.h"
#define DictionaryItemNameMaxLength 64
#define DictionarySize 512
	typedef int32_t Cell;
	typedef uint32_t UCell;
	//typedef Cell ExecutionToken;

	
	typedef void* (*AllocateHeap)(int numBytes);
	typedef void (*FreeHeap)(void* memory);
	typedef void (*ReallocateHeap)(void* memory, int numBytes);
	typedef int (*ForthPrintf)(const char* format, ...);
	typedef int (*ForthPrintf)(const char* format, ...);
	typedef int (*ForthPutChar)(int val);

	typedef enum {
		ColonWord,
		Variable,
		Constant,
		Primitive,
	}DictionaryEntryType;
	
	typedef enum {
		Forth_CompileBit = 1, 
		Forth_InColonDefinitionBit = 2,
		Forth_CommentFlag = 4
	}ForthMode;


	typedef struct {
		char name[DictionaryItemNameMaxLength];
		Cell* data;
		Bool isImmediate;
		void* previous;
	}DictionaryItem;

	typedef DictionaryItem* ExecutionToken;


	typedef struct {
		// vm state flags
		ForthMode currentMode;

		// for interpreting compiled byte code (innerinterpreter)
		Cell* instructionPointer;

		DictionaryItem* dictionarySearchStart;

		// memory for compiled words and variables
		Cell* memory;
		Cell* memoryTop;
		UCell maxMemorySize; // in cells

		Cell* intStack;
		Cell* intStackTop;
		UCell maxIntStackSize;

		Cell* returnStack;
		Cell* returnStackTop;
		UCell maxReturnStackSize;

		// for outer interpreter
		char tokenBuffer[DictionaryItemNameMaxLength + 1]; // holds current token as a c string during text interpretation
		const char* nextTokenStart;                        // points to the next token during text interpretation

		ForthPrintf printf;
		ForthPutChar putchar;

	}ForthVm;

	typedef Bool(*ForthCFunc)(ForthVm* vm);

	// only obtain a forth Vm like this
	ForthVm Forth_Initialise(
		Cell* memoryForCompiledWordsAndVariables,
		UCell memorySize,
		Cell* intStack,
		UCell intStackSize,
		Cell* returnStack,
		UCell returnStackSize,
		ForthPrintf printf,
		ForthPutChar putc);

	Bool Forth_DoString(ForthVm* vm, const char* inputString);
	void Forth_RegisterCFunc(ForthVm* vm, ForthCFunc function, const char* name, Bool isImmediate);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif
#endif