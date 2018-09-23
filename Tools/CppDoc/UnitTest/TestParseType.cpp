#include <Parser.h>
#include <Ast_Type.h>

extern Ptr<RegexLexer>		GlobalCppLexer();
extern void					Log(Ptr<Type> type, StreamWriter& writer);

void AssertType(const WString& type, const WString& log)
{
	CppTokenReader reader(GlobalCppLexer(), type);
	auto cursor = reader.GetFirstToken();

	ParsingArguments pa;
	List<Ptr<Declarator>> declarators;
	ParseDeclarator(pa, DeclaratorRestriction::Zero, InitializerRestriction::Zero, cursor, declarators);
	TEST_ASSERT(!cursor);
	TEST_ASSERT(declarators.Count() == 1);
	TEST_ASSERT(declarators[0]->name.tokenCount == 0);
	TEST_ASSERT(declarators[0]->initializer == nullptr);

	auto output = GenerateToStream([&](StreamWriter& writer)
	{
		Log(declarators[0]->type, writer);
	});
	TEST_ASSERT(output == log);
}

TEST_CASE(TestParseType_Primitive)
{
#define TEST_PRIMITIVE_TYPE(TYPE)\
	AssertType(L#TYPE, L#TYPE);\
	AssertType(L"signed " L#TYPE, L"signed " L#TYPE);\
	AssertType(L"unsigned " L#TYPE, L"unsigned " L#TYPE);\

	TEST_PRIMITIVE_TYPE(auto);
	TEST_PRIMITIVE_TYPE(void);
	TEST_PRIMITIVE_TYPE(bool);
	TEST_PRIMITIVE_TYPE(char);
	TEST_PRIMITIVE_TYPE(wchar_t);
	TEST_PRIMITIVE_TYPE(char16_t);
	TEST_PRIMITIVE_TYPE(char32_t);
	TEST_PRIMITIVE_TYPE(short);
	TEST_PRIMITIVE_TYPE(int);
	TEST_PRIMITIVE_TYPE(__int8);
	TEST_PRIMITIVE_TYPE(__int16);
	TEST_PRIMITIVE_TYPE(__int32);
	TEST_PRIMITIVE_TYPE(__int64);
	TEST_PRIMITIVE_TYPE(long);
	TEST_PRIMITIVE_TYPE(long long);
	TEST_PRIMITIVE_TYPE(float);
	TEST_PRIMITIVE_TYPE(double);
	TEST_PRIMITIVE_TYPE(long double);

#undef TEST_PRIMITIVE_TYPE
}

TEST_CASE(TestParseType_Short)
{
	AssertType(L"decltype(0)", L"decltype(0)");
	AssertType(L"constexpr int", L"int constexpr");
	AssertType(L"const int", L"int const");
	AssertType(L"volatile int", L"int volatile");
}

TEST_CASE(TestParseType_Long)
{
	AssertType(L"int constexpr", L"int constexpr");
	AssertType(L"int const", L"int const");
	AssertType(L"int volatile", L"int volatile");
	AssertType(L"int ...", L"int...");
	AssertType(L"int<long, short<float, double>>", L"int<long, short<float, double>>");
}

TEST_CASE(TestParseType_ShortDeclarator)
{
	AssertType(L"int* __ptr32", L"int *");
	AssertType(L"int* __ptr64", L"int *");
	AssertType(L"int*", L"int *");
	AssertType(L"int &", L"int &");
	AssertType(L"int &&", L"int &&");
	AssertType(L"int & &&", L"int & &&");
}