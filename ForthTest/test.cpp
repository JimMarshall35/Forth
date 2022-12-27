#include "pch.h"
#include "../Forth2/Forth2.h"
#include <string>
#include <cstdarg>
#include <conio.h>

namespace {
#define NumDictItems 256
#define MainMemorySize (1024 * 32)
#define IntStackSize 64
#define ReturnStackSize 64
#define ScratchPadSize 256

#define TestSetup(print, put, getc) \
Cell mainMem[MainMemorySize];\
Cell intStack[IntStackSize];\
Cell returnStack[ReturnStackSize];\
ForthVm vm = Forth_Initialise(\
    mainMem, MainMemorySize,\
    intStack, IntStackSize,\
    returnStack, ReturnStackSize,\
    print, put, getc);\

class IntegerStackTests :public ::testing::TestWithParam<std::tuple<std::string, std::vector<Cell>>> {
protected:
    void CompareStackToExpected(const ForthVm& vm, const std::vector<Cell>& expected) {
        size_t stackSize = vm.intStackTop - vm.intStack;
        ASSERT_EQ(stackSize, expected.size());
        for (int i = 0; i < expected.size(); i++) {
            ASSERT_EQ(vm.intStack[i], expected[i]);
        }
    }
};

TEST_P(IntegerStackTests, CorrectValueOnStackAfterEnd) {
    // arrange
    TestSetup(&printf, &putchar, &_getch);
    std::string stringToDo = std::get<0>(GetParam());
    auto expectedStack = std::get<1>(GetParam());

    // act
    Forth_DoString(&vm, stringToDo.c_str());

    // assert
    CompareStackToExpected(vm, expectedStack);
}

using Stack = std::vector<Cell>;
#define Case std::make_tuple

INSTANTIATE_TEST_CASE_P(
    CorrectValueOnStackAfterEndTestCases,
    IntegerStackTests,
    ::testing::Values(
        // +
        Case("1 2 +", Stack{ 3 }),
        Case("400 20 +", Stack{ 420 }),
        Case("420 400 20 +", Stack{ 420,420 }),
        // -
        Case("1 2 -", Stack{ -1 }),
        Case("24 21 -", Stack{ 3 }),
        Case("32 32 -", Stack{ 0 }),
        Case("32 32 32 -", Stack{ 32, 0 }),
        Case("32 -32 32 -", Stack{ 32, -64 }),
        // /
        Case("4 2 /", Stack{ 2 }),
        Case("12 3 /", Stack{ 4 }),
        Case("123 120 1 /", Stack{ 123, 120 }),
        Case("4 3 /", Stack{ 1 }),
        // *
        Case("1 200 1 *", Stack{ 1, 200 }),
        Case("20003 2 *", Stack{ 40006 }),
        Case("3 3 *", Stack{ 9 }),
        Case("1 2 + 25 4 *", Stack{ 3, 100 }),
        // %
        Case("9 3 %", Stack{ 0 }),

        // : and ;
        Case(": test 1 2 + ; test", Stack{ 3 }),
        Case(": test 1 2 + ; : test2 test ; test2", Stack{ 3 }),
        Case(": test 1 2 + ; : test2 test 3 + ; test2", Stack{ 6 }),
        Case(": test 1 2 + ; : test2 test 3 + ; : test3 test2 420 + 2 - ; test3", Stack{ 424 }),


        // branch0 - I've decided to make branch0 not be able to be used like this
        // - I've made branches take up less code by not needing to have a literal token before the branch
        // value - as a result you can't use them like this and I don't want to add logic to not compile literal 
        // tokens if the previous token was a branch, ect, I'd rather keep it simple and only use branch and branch0
        // by having higher level immediate words compile them into ifs, do's, ect, no need to expose them for use like this.
        // todo - add some kind of error checking in perhaps
        //Case(": test branch0 4 420 ; 1 test", Stack{ 420 }),
        //Case(": test branch0 4 420 ; 0 test", Stack{  }),
        //Case(": test branch0 7 1 2 + ; 1 test", Stack{ 3 }),
        //Case(": test branch0 7 1 2 + ; 0 test", Stack{  }),

        // comments
        Case(": test ( comment 1 ) 1 23 ( another comment ) + ; test", Stack{ 24 }),

        Case("1 2 3 4 2swap", Stack{ 3, 4, 1, 2 }),
        Case("1 2 3 4 2dup", Stack{ 1, 2, 3, 4, 3, 4 }),

        // built in composite words
        Case(": test if 1 else 2 then ; 0 test 1 test", Stack{ 2,1 }),
        Case(": test if 1 2 3 + + else 2 32 + then ; 0 test 1 test", Stack{ 34, 6 }),
        Case(": test do 123 loop ; 5 0 test", Stack{ 123,123,123,123,123 }),
        Case(": test do 123 loop ; 3 0 test", Stack{ 123,123,123 }),
        Case(": test do 3 0 do 1 loop 2 loop ; 3 0 test", Stack{ 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2 }),
        Case(": test do 3 0 do 1 loop 2 loop ; 2 0 test", Stack{ 1, 1, 1, 2, 1, 1, 1, 2 })

    ));
class PutCharTests :public ::testing::TestWithParam<std::tuple<std::string, std::vector<char>, std::vector<Cell>>> {
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
    static int MockPrintf(const char* format, ...) {
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsprintf_s(buffer, format, args);
        const char* readPtr = buffer;
        while (*readPtr != '\0') {
            s_charOutput.push_back(*readPtr++);
        }
        return 1;
    }
    void CompareStackToExpected(const ForthVm& vm, const std::vector<Cell>& expected) {
        size_t stackSize = vm.intStackTop - vm.intStack;
        ASSERT_EQ(stackSize, expected.size());
        for (int i = 0; i < expected.size(); i++) {
            ASSERT_EQ(vm.intStack[i], expected[i]);
        }
    }
};

std::vector<char> PutCharTests::s_charOutput;

TEST_P(PutCharTests, CorrectValuesPassedToPutChar) {
    // not sure how these pass despite being run in parallel... but they seem to
    // and do fail if given incorrect expected outputs

    // arrange
    TestSetup(&MockPrintf, &MockPutChar, &_getch);
    ClearCharOutput();

    std::string stringToDo = std::get<0>(GetParam());
    auto expectedChars = std::get<1>(GetParam());
    auto expectedStack = std::get<2>(GetParam());

    // act
    Forth_DoString(&vm, stringToDo.c_str());

    // assert
    ComparePutCharOutputToExpected(expectedChars);
    CompareStackToExpected(vm, expectedStack);
}

using Chars = std::vector<char>;
Chars FromStringLiteral(const char* string) {
    Chars chars;
    while (*string != '\0') {
        chars.push_back(*string++);
    }
    return chars;
}

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
"; 16 fizzbuzz";

const char* fizzbuzzTestOutput = "0\n1\n2\nfizz\n4\nbuzz\nfizz\n7\n8\nfizz\nbuzz\n11\nfizz\n13\n14\nfizzbuzz\n";
INSTANTIATE_TEST_CASE_P(
    CorrectValuesPassedToPutCharTestCases,
    PutCharTests,
    ::testing::Values(
        // emit
        Case("30 emit", Chars{ 30 }, Stack{}),
        // begin, until
        Case(": test begin 29 emit -1 + dup not until drop ; 3 test", Chars{ 29, 29, 29 }, Stack{}),
        Case(": test begin 29 emit -1 + dup not until drop ; 4 test", Chars{ 29, 29, 29, 29 }, Stack{}),
        Case(": test begin 29 emit -1 + dup not until drop ; 1 test", Chars{ 29 }, Stack{}),
        // string literal, print, do
        Case(": test s\" hello world\" print ; test", FromStringLiteral("hello world"), Stack{}),
        Case(": test s\" What's up ?!?!\" print 123 456 ; test", FromStringLiteral("What's up ?!?!"), Stack{ 123,456 }),
        Case(": test 0 do s\" hello\" print loop ; 3 test", FromStringLiteral("hellohellohello"), Stack{}),
        Case(": test s\" hello world\" print ; 420 . test", FromStringLiteral("420hello world"), Stack{}),
        Case(fizzbuzzTest, FromStringLiteral(fizzbuzzTestOutput), Stack{})
    ));

}

class ReturnStackTests :public ::testing::TestWithParam<std::tuple<std::string, std::vector<Cell>, std::vector<Cell>>> {
private:
protected:
    void CompareReturnStackToExpected(const ForthVm& vm, const std::vector<Cell>& expected) {
        size_t stackSize = vm.returnStackTop - vm.returnStack;
        ASSERT_EQ(stackSize, expected.size());
        for (int i = 0; i < expected.size(); i++) {
            ASSERT_EQ(vm.returnStack[i], expected[i]);
        }
    }
    void CompareIntStackToExpected(const ForthVm& vm, const std::vector<Cell>& expected) {
        size_t stackSize = vm.intStackTop - vm.intStack;
        ASSERT_EQ(stackSize, expected.size());
        for (int i = 0; i < expected.size(); i++) {
            ASSERT_EQ(vm.intStack[i], expected[i]);
        }
    }
};

TEST_P(ReturnStackTests, CorrectValueOnStackAfterEnd) {
    // arrange
    TestSetup(&printf, &putchar, &_getch);
    std::string stringToDo = std::get<0>(GetParam());
    auto expectedReturnStack = std::get<1>(GetParam());
    auto expectedIntStack = std::get<2>(GetParam());

    // act
    Forth_DoString(&vm, stringToDo.c_str());

    // assert
    CompareReturnStackToExpected(vm, expectedReturnStack);
    CompareIntStackToExpected(vm, expectedIntStack);
}

using RStack = std::vector<Cell>;
INSTANTIATE_TEST_CASE_P(
    CorrectValueOnStackAfterEndTestCases,
    ReturnStackTests,
    ::testing::Values(
        // not a very good test because you can't push to the return stack
        // without also popping or else the interpretter will try to return to that address
        // that has been pushed - but better than nothing (maybe)
        Case(": test 10 R R> ; test", RStack{  }, Stack{ 10 })

    ));
