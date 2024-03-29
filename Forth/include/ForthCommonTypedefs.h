#ifndef COMMONTYPEDEFS
#define COMMONTYPEDEFS
#ifdef __cplusplus
extern "C" {
#endif

#include "ForthEnvironmentDefs.h"

typedef enum {
	False,
	True
}Bool;


typedef int (*ForthPutChar)(int val);
typedef int (*ForthGetChar)(void);

typedef enum {
	Forth_CompileBit = 1,
	Forth_CommentFlag = 2
}ForthMode;

#define DictionaryItemNameMaxLength 64
#define ForthVmMaxTasks 16
#define TaskNameMaxLength 16

typedef struct {
	char name[DictionaryItemNameMaxLength];
	Cell* data;
	Bool isImmediate;
	void* previous;
}ForthDictHeader;

typedef ForthDictHeader* ExecutionToken;

typedef struct
{
	void* nextTask;
	Cell* instructionPointer;
	Cell* intStackTop;
	Cell* returnStackTop;
	UCell isAwake;

	Cell* intStack; // at the moment not necessary because all tasks have the same size stacks - but this could change in future
	Cell* returnStack;

	// message box
	Cell message;
	Cell sender;
}ForthTaskUserArea;

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
	ForthTaskUserArea* tasks[ForthVmMaxTasks];
	UCell numTasks;

	ForthTaskUserArea* currentRunningTask;
}ForthVm;



// my solution to unintentional break case fallthrough in C / C++
#define BCase break; case 
#define BDefault break; default 


#ifdef __cplusplus
} // closing brace for extern "C"
#endif
#endif