#include "pch.h"
#include "../Forth2/StringUtils.h"

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
		Case("39212932024", 39212932024),
		Case("-640", -640),
		Case("-6533308473892983", -6533308473892983),
		Case("-3", -3),
		Case("-0", 0)
	));