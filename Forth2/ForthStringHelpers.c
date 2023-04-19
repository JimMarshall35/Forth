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

Cell ForthAtoi(const char* string)
{
	Cell signMultiplier = 1;
	Cell endPoint = 0;
	if (string[0] == '-') {
		signMultiplier = -1;
		endPoint = 1;
	}

	Cell length = StringLength(string);
	Cell rVal = 0;
	Cell unitsTensHundredsEct = 1;
	for (Cell i= length-1; i >= endPoint; --i) {
		Cell value = string[i] - '0';
		rVal += value * unitsTensHundredsEct;
		unitsTensHundredsEct *= 10;
	}

	return rVal * signMultiplier;
}

void ForthPrint(const ForthVm* vm, const char* string)
{
	char* thisChar = string;
	while (*thisChar != '\0') {
		vm->putchar(*thisChar++);
	}
}

void ForthPrintInt(const ForthVm* vm, Cell val)
{
	static char buf[32] = { 0 };
	int base = 10;
	int i = 30;
	if (val == 0) {
		vm->putchar('0');
		return;
	}
	Cell absVal = (val < 0) ? -val : val;
	for (; absVal && i; --i, absVal /= base) {
		buf[i] = "0123456789abcdef"[absVal % base];
	}
	if (val < 0) {
		buf[i--] = '-';
	}
	char* string = &buf[i + 1];
	ForthPrint(vm, string);
}

