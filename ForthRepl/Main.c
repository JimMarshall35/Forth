#include <stdio.h>
#include "../Forth2/Forth2.h"
#define NumDictItems 256
#define MainMemorySize (1024 * 32)
#define IntStackSize 64
#define ReturnStackSize 64
#define InputBufferSize 10000

static char InputBuffer[InputBufferSize];

void Repl(ForthVm* vm) {
    int numTokens = 0;
    do {
        printf("[Forth]>>> ");
        gets(InputBuffer);
        Bool result = Forth_DoString(vm, InputBuffer);
        numTokens = 0;
        if (!result) {
            printf("Error\n");
        }
    } while (True);
}


int main()
{
    Cell mainMem[MainMemorySize];
    Cell intStack[IntStackSize];
    Cell returnStack[ReturnStackSize];

    ForthVm vm = Forth_Initialise(
        mainMem, MainMemorySize,
        intStack, IntStackSize,
        returnStack, ReturnStackSize,
        &printf, &putchar);
    Repl(&vm);
}
