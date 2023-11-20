
#ifndef FORTH2
#define FORTH2
#ifdef __cplusplus
extern "C" {
#endif


#include "ForthCommonTypedefs.h"

	typedef Bool(*ForthCFunc)(ForthVm* vm);

	// only obtain a forth Vm like this
	ForthVm Forth_Initialise(
		Cell* memoryForCompiledWordsAndVariables,
		UCell memorySize,
		UCell intStackSize,
		UCell returnStackSize,
		ForthPutChar putc,
		ForthGetChar getc);

	Bool Forth_DoString(ForthVm* vm, const char* inputString);
	void Forth_RegisterCFunc(ForthVm* vm, ForthCFunc function, const char* name, Bool isImmediate);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif
#endif