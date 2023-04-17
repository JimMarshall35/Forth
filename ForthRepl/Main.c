#include <stdio.h>
#include <conio.h>
#include "../Forth2/Forth2.h"
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

/*
: fizzbuzz 0 do i 0 = if 0 . cr else i 15 % 0 = if s" fizzbuzz" print cr else i 3 % 0 = if s" fizz" print cr else i 5 % 0 = if s" buzz" print cr else i . cr then then then then loop ;
*/
const char* fizzbuzzTest = 
": fizzbuzz "
    "0 do "
        "i 0 = if "
            "0 . cr "
        "else i 15 % 0 = if "
            "s\" fizzbuzz\" print cr "
        "else i 3 % 0 = if "
            "s\" fizz\" print cr "
        "else i 5 % 0 = if "
            "s\" buzz\" print cr "
        "else "
            "i . cr "
        "then "
        "then "
        "then "
        "then "
    "loop "
";";

void putChar(char c) {
    printf("%c", c);
}

int main()
{
    Cell mainMem[MainMemorySize];
    Cell intStack[IntStackSize];
    Cell returnStack[ReturnStackSize];
    //int i = getch();
    ForthVm vm = Forth_Initialise(
        mainMem, MainMemorySize,
        intStack, IntStackSize,
        returnStack, ReturnStackSize,
        &putchar, &_getch);
    Forth_DoString(&vm, fizzbuzzTest);
    Repl(&vm);
}
