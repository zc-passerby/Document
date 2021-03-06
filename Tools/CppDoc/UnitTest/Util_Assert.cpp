#include "Util.h"

/***********************************************************************
AssertMultilines
***********************************************************************/

void AssertMultilines(const WString& output, const WString& log)
{
	StringReader srExpect(log);
	StringReader srActual(L"\r\n" + output);

	while (true)
	{
		TEST_ASSERT(srExpect.IsEnd() == srActual.IsEnd());
		if (srExpect.IsEnd()) break;

		auto expect = srExpect.ReadLine();
		auto actual = srActual.ReadLine();
		TEST_ASSERT(expect == actual);
	}
}

/***********************************************************************
AssertType
***********************************************************************/

void AssertType(const WString& input, const WString& log, const WString& logTsys)
{
	ParsingArguments pa(new Symbol, ITsysAlloc::Create(), nullptr);
	AssertType(input, log, logTsys, pa);
}

void AssertType(const WString& input, const WString& log, const WString& logTsys, ParsingArguments& pa)
{
	CppTokenReader reader(GlobalCppLexer(), input);
	auto cursor = reader.GetFirstToken();

	auto type = ParseType(pa, cursor);
	TEST_ASSERT(!cursor);

	auto output = GenerateToStream([&](StreamWriter& writer)
	{
		Log(type, writer);
	});
	TEST_ASSERT(output == log);

	try
	{
		TypeTsysList tsys;
		TypeToTsys(pa, type, tsys);
		if (tsys.Count() == 0)
		{
			TEST_ASSERT(L"" == logTsys);
		}
		else
		{
			TEST_ASSERT(tsys.Count() == 1);
			auto outputTsys = GenerateToStream([&](StreamWriter& writer)
			{
				Log(tsys[0], writer);
			});
			TEST_ASSERT(outputTsys == logTsys);
		}
	}
	catch (const NotConvertableException&)
	{
		TEST_ASSERT(L"" == logTsys);
	}
}

/***********************************************************************
AssertExpr
***********************************************************************/

void AssertExpr(const WString& input, const WString& log, const WString& logTsys)
{
	ParsingArguments pa(new Symbol, ITsysAlloc::Create(), nullptr);
	AssertExpr(input, log, logTsys, pa);
}

void AssertExpr(const WString& input, const WString& log, const WString& logTsys, ParsingArguments& pa)
{
	CppTokenReader reader(GlobalCppLexer(), input);
	auto cursor = reader.GetFirstToken();

	auto expr = ParseExpr(pa, true, cursor);
	TEST_ASSERT(!cursor);

	auto output = GenerateToStream([&](StreamWriter& writer)
	{
		Log(expr, writer);
	});
	TEST_ASSERT(output == log);

	ExprTsysList tsys;
	try
	{
		ExprToTsys(pa, expr, tsys);
	}
	catch (const IllegalExprException&)
	{
		TEST_ASSERT(L"" == logTsys);
		return;
	}

	if (L"" == logTsys)
	{
		TEST_ASSERT(tsys.Count() == 0);
	}
	else
	{
		TEST_ASSERT(tsys.Count() == 1);
		auto outputTsys = GenerateToStream([&](StreamWriter& writer)
		{
			Log(tsys[0].tsys, writer);
			switch (tsys[0].type)
			{
			case ExprTsysType::LValue:
				writer.WriteString(L" $L");
				break;
			case ExprTsysType::PRValue:
				writer.WriteString(L" $PR");
				break;
			case ExprTsysType::XValue:
				writer.WriteString(L" $X");
				break;
			}
		});
		TEST_ASSERT(outputTsys == logTsys);
	}
}

/***********************************************************************
AssertStat
***********************************************************************/

void AssertStat(const WString& input, const WString& log)
{
	ParsingArguments pa(new Symbol, ITsysAlloc::Create(), nullptr);
	AssertStat(input, log, pa);
}

void AssertStat(const WString& input, const WString& log, ParsingArguments& pa)
{
	CppTokenReader reader(GlobalCppLexer(), input);
	auto cursor = reader.GetFirstToken();

	auto stat = ParseStat(pa, cursor);
	TEST_ASSERT(!cursor);

	auto output = GenerateToStream([&](StreamWriter& writer)
	{
		Log(stat, writer, 0);
	});

	AssertMultilines(output, log);
}

/***********************************************************************
AssertProgram
***********************************************************************/

void AssertProgram(const WString& input, const WString& log, Ptr<IIndexRecorder> recorder)
{
	COMPILE_PROGRAM_WITH_RECORDER(program, pa, input, recorder);
	AssertProgram(program, log);
}

void AssertProgram(Ptr<Program> program, const WString& log)
{
	auto output = GenerateToStream([&](StreamWriter& writer)
	{
		Log(program, writer);
	});
	AssertMultilines(output, log);
}