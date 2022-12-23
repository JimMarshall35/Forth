// Forth2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <stdio.h>
#include "Forth2.h"
#define NumDictItems 256
#define MainMemorySize (1024 * 32)
#define IntStackSize 64
#define ReturnStackSize 64
#define ScratchPadSize 256


#define INPUT_BUFFER_SIZE 10000
static char InputBuffer[INPUT_BUFFER_SIZE];

void Repl(ForthVm* vm) {
    int numTokens = 0;
    do {
        //memset(InputBuffer, 0, sizeof(int) * INPUT_BUFFER_SIZE);
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
    //const char* string1 = "hello world";
    //const char* string2 = "hello world";
    //const char* string3 = "hello world!";
    //const char* string4 = "hello World";

    //bool oneAndtwo = StringCompare(string1, string2);
    //bool oneAndThree = StringCompare(string1, string3);
    //bool oneAndFour = StringCompare(string1, string4);

    DictionaryItem dictItems[NumDictItems];
    Cell mainMem[MainMemorySize];
    Cell intStack[IntStackSize];
    Cell returnStack[ReturnStackSize];
    Cell scratchPad[ReturnStackSize];

    ForthVm vm = Forth_Initialise(
        mainMem, MainMemorySize,
        intStack, IntStackSize,
        returnStack, ReturnStackSize,
        scratchPad, ScratchPadSize,
        &printf, &putchar);
    Repl(&vm);
}
