#include <stdio.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include "Forth2.h"
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

/*
Applies this transformation to the contents of file fp:
removes tabs, changes newlines into spaces, limits spaces (outside of string literals and comments) to only one concecutive space.
writes result into buf
*/
void PreProcessForthSourceFileContentsIntoBuffer(FILE* fp, char* buf) {
    char* writePtr = buf;
    Bool inStringOrComment = False;
    int consecutiveSpaceCount = 0;
    while (!feof(fp)) {
        char fileChar = (char)fgetc(fp);
        if (!inStringOrComment) {
            switch (fileChar) {
            BCase '\t':
            // ignore tabs
            case '\n' :
            case ' ' : // intentional fallthrough
                // only ever one space in a row
                if (consecutiveSpaceCount++ == 0) {
                    *writePtr++ = ' ';
                }
            BCase '"' :
                if (*(writePtr - 1) == 's') {
                    inStringOrComment = True;
                }
                *writePtr++ = fileChar;
                consecutiveSpaceCount = 0;
            BCase '(':
                if (*(writePtr - 1) == ' ') {
                    inStringOrComment = True;
                }
                *writePtr++ = fileChar;
                consecutiveSpaceCount = 0;
            BDefault:
                *writePtr++ = fileChar;
                consecutiveSpaceCount = 0;
            }
        }
        else {
            if (fileChar == '"' || fileChar == ')') { // TODO: make more robust to allow ) in string literals and " in comments
                inStringOrComment = False;
            }
            *writePtr++ = fileChar;
        }
    }
    writePtr[-1] = '\0';
}

void DoCommandLineArgs(ForthVm* vm,int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {        // argv[0] points to path of exe so ignore it
        // open file from path in argv
        FILE* fp;
        char* filePath = argv[i];
        fp = fopen(filePath, "r");
        if (fp == NULL) {
            printf("can't open file %s . Error code %d", argv[i], errno);
            continue;
        }

        // get size
        fseek(fp, 0L, SEEK_END);
        long unsigned int fileSize = ftell(fp);
        rewind(fp);

        // allocate buffer to contain the pre-processed text from the file
        char* buf = malloc(fileSize + 2); // +1 for null terminator. This is the max size it will be pre-processed as characters are only skipped out never added

        // pre process forth source file so you can format it nicely in the source file.
        // Outside of comments and string literals turns newlines into spaces, remove duplicate spaces and remove all tabs entirely.
        PreProcessForthSourceFileContentsIntoBuffer(fp, buf);

        //printf("%s", buf);
        Forth_DoString(vm, buf);

        // clean up
        free(buf);
        fclose(fp);
    }
}

int main(int argc, char* argv[])
{
    Cell mainMem[MainMemorySize];
    Cell intStack[IntStackSize];
    Cell returnStack[ReturnStackSize];
    //int i = getch();
    ForthVm vm = Forth_Initialise(
        mainMem, MainMemorySize,
        IntStackSize,
        ReturnStackSize,
        &putchar, &_getch);
    Forth_DoString(&vm, fizzbuzzTest);
    DoCommandLineArgs(&vm, argc, argv);
    Repl(&vm);
}
