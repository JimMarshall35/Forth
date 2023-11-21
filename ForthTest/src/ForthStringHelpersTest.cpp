#include "pch.h"
#include "ForthStringHelpers.h"
#include <conio.h>
#include "Forth2.h" // the fact I have to include this header to write the tests as well suggests the string helper methods which require an initialised forth VM should be moved to Forth.c/.h or changed to take a putchar pointer instead of a forth vm

#define NumDictItems 256
#define MainMemorySize (1024 * 32)
#define IntStackSize 64
#define ReturnStackSize 64
#define ScratchPadSize 256

#define TestSetup(put, get) \
Cell mainMem[MainMemorySize];\
Cell intStack[IntStackSize];\
Cell returnStack[ReturnStackSize];\
ForthVm vm = Forth_Initialise(\
    mainMem, MainMemorySize,\
    IntStackSize,\
    ReturnStackSize,\
    put, get);\


class AtoiTests :public ::testing::TestWithParam<std::tuple<std::string, Cell>> {
};

TEST_P(AtoiTests, CorrectValueReturned) {
	std::string input = std::get<0>(GetParam());
	auto expectedOutpu = std::get<1>(GetParam());
	auto result = ForthAtoi(input.c_str());
	ASSERT_EQ(result, expectedOutpu);
}

INSTANTIATE_TEST_CASE_P(
	OutputMatchesExpected,
	AtoiTests,
	::testing::Values(
		Case("123", 123),
		Case("2", 2),
		Case("6", 6),
		Case("3921293", 3921293),
		Case("-640", -640),
		Case("-6533308", -6533308),
		Case("-3", -3),
		Case("-0", 0)
	));

using Chars = std::vector<char>;
class ForthPrintIntTests : public ::testing::TestWithParam<std::tuple<Chars, Cell>> {
private:
	static std::vector<char> s_charOutput;
protected:
	void ComparePutCharOutputToExpected(const std::vector<char>& expected) {

		ASSERT_EQ(s_charOutput.size(), expected.size());// << std::string(s_charOutput.data());
		for (int i = 0; i < expected.size(); i++) {
			ASSERT_EQ(s_charOutput[i], expected[i]);// << std::string(s_charOutput.data());
		}
	}
	static void ClearCharOutput() {
		s_charOutput.clear();
	}
	static int MockPutChar(int charVal) {
		s_charOutput.push_back((char)charVal);
		return 0;
	}
};
std::vector<char> ForthPrintIntTests::s_charOutput;

TEST_P(ForthPrintIntTests, CorrectValuePrinted) {
	TestSetup(&MockPutChar, &_getch);
	ClearCharOutput();
	auto expectedOutpu = std::get<0>(GetParam());
	auto input = std::get<1>(GetParam());
	ForthPrintInt(&vm, input);
	ComparePutCharOutputToExpected(expectedOutpu);
}

Chars FromStringLiteral(const char* string) {
	Chars chars;
	while (*string != '\0') {
		chars.push_back(*string++);
	}
	return chars;
}

INSTANTIATE_TEST_CASE_P(
	OutputMatchesExpected,
	ForthPrintIntTests,
	::testing::Values(
		Case(FromStringLiteral("123"), 123),
		Case(FromStringLiteral("2"), 2),
		Case(FromStringLiteral("6"), 6),
		Case(FromStringLiteral("56643"), 56643),
		Case(FromStringLiteral("-640"), -640),
		Case(FromStringLiteral("-6533308"), -6533308),
		Case(FromStringLiteral("-3"), -3),
		Case(FromStringLiteral("0"), 0)
	));