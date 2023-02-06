#ifndef COMMONTYPEDEFS
#define COMMONTYPEDEFS
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#define DictionaryItemNameMaxLength 64

typedef enum {
	False,
	True
}Bool;
typedef int32_t Cell;
typedef uint32_t UCell;

typedef int (*ForthPutChar)(int val);
typedef int (*ForthGetChar)(void);

typedef enum {
	Forth_CompileBit = 1,
	Forth_CommentFlag = 2
}ForthMode;

typedef struct {
	char name[DictionaryItemNameMaxLength];
	Cell* data;
	Bool isImmediate;
	void* previous;
}ForthDictHeader;

typedef ForthDictHeader* ExecutionToken;

typedef struct {
	// vm state flags
	ForthMode currentMode;

	// for interpreting compiled byte code (innerinterpreter)
	Cell* instructionPointer;

	ForthDictHeader* dictionarySearchStart;

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

	ForthPutChar putchar;
	ForthGetChar getchar;
}ForthVm;

// my solution to unintentional break case fallthrough in C / C++
#define BCase break; case 


#ifdef __cplusplus
} // closing brace for extern "C"
#endif
#endif