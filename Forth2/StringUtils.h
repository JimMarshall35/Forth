#ifndef STRINGUTILS
#define STRINGUTILS
#ifdef __cplusplus
extern "C" {
#endif
#include "CommonTypedefs.h"

int StringLength(const char* string);
Bool StringCompare(const char* string1, const char* string2);
void StringCopy(char* destination, const char* source);
int CopyNextToken(const char* inputString, char* tokenOutput);
void CopyStringUntilSpaceCappingWithNull(char* dest, const char* src);

#ifdef __cplusplus
} // closing brace for extern "C"
#endif
#endif