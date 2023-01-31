#include "ForthStringHelpers.h"

Cell StringLength(const char* string)
{
	int len = 0;
	while (*(string++) != '\0') {
		len++;
	}
	return len;
}

Bool StringCompare(const char* string1, const char* string2) {
	const char* readPtr1 = string1;
	const char* readPtr2 = string2;

	while (*readPtr1 != '\0') {
		if (*readPtr1 != *readPtr2) {
			return False;
		}
		readPtr1++;
		readPtr2++;
	}
	if (*readPtr2 != '\0') {
		return False;
	}
	return True;
}

void StringCopy(char* destination, const char* source) {
	char* destinationWritePtr = destination;
	const char* srcReadPtr = source;
	while (*srcReadPtr != '\0') {
		*destinationWritePtr = *srcReadPtr;
		srcReadPtr++;
		destinationWritePtr++;
	}
	*destinationWritePtr = '\0';
}

int CopyNextToken(const char* inputString, char* tokenOutput)
{
	Bool shouldContinue = True;
	const char* thisChar = inputString;
	char* writePtr = tokenOutput;
	int length = 0;
	while (shouldContinue) {
		char thisCharVal = *(thisChar++);
		if (thisCharVal == ' ') { // finished reading token
			*writePtr = '\0';
			writePtr = tokenOutput;
			shouldContinue = False;
		}
		else if (thisCharVal == '\0') {  // finished reading string
			*writePtr = '\0';
			shouldContinue = False;
		}
		else { // read character of string
			*(writePtr++) = thisCharVal;
			length++;
		}

	}
	return length;
}


void CopyStringUntilSpaceCappingWithNull(char* dest, const char* src) {
	do {
		*(dest++) = *(src++);
	} while (*src != ' ');
	*dest = '\0';
}

static Cell myPow(Cell x, Cell n)
{
	Cell number = 1;

	for (Cell i = 0; i < n; ++i) {
		number *= x;
	}

	return number;
}

Cell ForthAtoi(const char* string)
{
	Cell length = StringLength(string);
	Cell rVal = 0;
	Cell signMultiplier = 1;
	if (string[0] == '-') {
		++string;
		--length;
		signMultiplier = -1;
	}

	for (Cell i=0; i < length; i++) {
		Cell value = string[i] - '0';
		Cell ValueMultiplier = myPow(10, (length - (i + 1)));
		rVal += value * ValueMultiplier;
	}

	return rVal * signMultiplier;
}

void ForthPrint(const ForthVm* vm, const char* string)
{
	char thisChar;
	if (*string == '\0') {
		return;
	}
	do{
		thisChar = *string++;
		vm->putchar(thisChar);
	} while (thisChar != '\0');
}

void ForthPrintInt(const ForthVm* vm, Cell cell)
{

}

