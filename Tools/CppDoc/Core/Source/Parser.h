#ifndef VCZH_DOCUMENT_CPPDOC_PARSER
#define VCZH_DOCUMENT_CPPDOC_PARSER

#include "Lexer.h"
#include "Ast.h"
#include "TypeSystem.h"

/***********************************************************************
Symbol
***********************************************************************/

class Symbol : public Object
{
	using SymbolGroup = Group<WString, Ptr<Symbol>>;
	using SymbolPtrList = List<Symbol*>;
public:
	Symbol*					parent = nullptr;
	WString					name;
	List<Ptr<Declaration>>	decls;			// only namespaces share symbols
	Ptr<Stat>				stat;			// if this scope is created by a statement
	SymbolGroup				children;

	Ptr<TypeTsysList>		resolvedTypes;	// only for Forward(Variable|Function)Declaration of which has a pending type

	bool					isForwardDeclaration = false;
	Symbol*					forwardDeclarationRoot = nullptr;
	SymbolPtrList			forwardDeclarations;

	Symbol*					specializationRoot = nullptr;
	SymbolPtrList			specializations;

	SymbolPtrList			usingNss;

	void					Add(Ptr<Symbol> child);

	Symbol* CreateDeclSymbol(Ptr<Declaration> _decl, Symbol* _specializationRoot = nullptr)
	{
		auto symbol = MakePtr<Symbol>();
		symbol->name = _decl->name.name;
		symbol->decls.Add(_decl);
		Add(symbol);

		if (!_decl->symbol)
		{
			_decl->symbol = symbol.Obj();
		}
		if (_specializationRoot)
		{
			_specializationRoot->specializations.Add(symbol.Obj());
			symbol->specializationRoot = _specializationRoot;
		}
		return symbol.Obj();
	}

	Symbol* CreateStatSymbol(Ptr<Stat> _stat)
	{
		auto symbol = MakePtr<Symbol>();
		symbol->name = L"$";
		symbol->stat = _stat;
		Add(symbol);

		_stat->symbol = symbol.Obj();
		return symbol.Obj();
	}

	bool SetForwardDeclarationRoot(Symbol* root)
	{
		if (forwardDeclarationRoot == root) return true;
		if (forwardDeclarationRoot) return false;
		forwardDeclarationRoot = root;
		root->forwardDeclarations.Add(this);
		return true;
	}
};

/***********************************************************************
Parsers
***********************************************************************/

class IIndexRecorder : public virtual Interface
{
public:
	virtual void			Index(CppName& name, Ptr<Resolving> resolving) = 0;
	virtual void			ExpectValueButType(CppName& name, Ptr<Resolving> resolving) = 0;
};

enum class DeclaratorRestriction
{
	Zero,
	Optional,
	One,
	Many,
};

enum class InitializerRestriction
{
	Zero,
	Optional,
};

struct ParsingArguments
{
	Ptr<Symbol>				root;
	Symbol*					context = nullptr;
	Ptr<ITsysAlloc>			tsys;
	Ptr<IIndexRecorder>		recorder;

	ParsingArguments();
	ParsingArguments(Ptr<Symbol> _root, Ptr<ITsysAlloc> _tsys, Ptr<IIndexRecorder> _recorder);
	ParsingArguments(const ParsingArguments& pa, Symbol* _context);
};

struct StopParsingException
{
	Ptr<CppTokenCursor>		position;

	StopParsingException() {}
	StopParsingException(Ptr<CppTokenCursor> _position) :position(_position) {}
};

class FunctionType;
class ClassDeclaration;
class VariableDeclaration;

// Parser_ResolveSymbol.cpp
enum class SearchPolicy
{
	SymbolAccessableInScope,
	ChildSymbol,
	ChildSymbolRequestedFromSubClass,
};

struct ResolveSymbolResult
{
	Ptr<Resolving>					values;
	Ptr<Resolving>					types;

	void							Merge(Ptr<Resolving>& to, Ptr<Resolving> from);
	void							Merge(const ResolveSymbolResult& rar);
};
extern ResolveSymbolResult			ResolveSymbol(const ParsingArguments& pa, CppName& name, SearchPolicy policy, ResolveSymbolResult input = {});
extern ResolveSymbolResult			ResolveChildSymbol(const ParsingArguments& pa, Ptr<Type> classType, CppName& name, ResolveSymbolResult input = {});

// Parser_Misc.cpp
extern bool							SkipSpecifiers(Ptr<CppTokenCursor>& cursor);
extern bool							ParseCppName(CppName& name, Ptr<CppTokenCursor>& cursor, bool forceSpecialMethod = false);
extern Ptr<Type>					GetTypeWithoutMemberAndCC(Ptr<Type> type);
extern Ptr<Type>					ReplaceTypeInMemberAndCC(Ptr<Type>& type, Ptr<Type> typeToReplace);
extern Ptr<Type>					AdjustReturnTypeWithMemberAndCC(Ptr<FunctionType> functionType);
extern bool							ParseCallingConvention(TsysCallingConvention& callingConvention, Ptr<CppTokenCursor>& cursor);

// Parser_Type.cpp
extern Ptr<Type>					ParseLongType(const ParsingArguments& pa, Ptr<CppTokenCursor>& cursor);

// Parser_Declarator.cpp
struct ParsingDeclaratorArguments
{
	ClassDeclaration*				containingClass;
	bool							forParameter;
	DeclaratorRestriction			dr;
	InitializerRestriction			ir;

	ParsingDeclaratorArguments(ClassDeclaration* _containingClass, bool _forParameter, DeclaratorRestriction _dr, InitializerRestriction _ir)
		:containingClass(_containingClass)
		, forParameter(_forParameter)
		, dr(_dr)
		, ir(_ir)
	{
	}
};

inline ParsingDeclaratorArguments	pda_Type()
	{	return { nullptr,	false,			DeclaratorRestriction::Zero,		InitializerRestriction::Zero		}; } // Type
inline ParsingDeclaratorArguments	pda_VarType()
	{	return { nullptr,	false,			DeclaratorRestriction::Optional,	InitializerRestriction::Zero		}; } // Type or Variable without Initializer
inline ParsingDeclaratorArguments	pda_VarInit()
	{	return { nullptr,	false,			DeclaratorRestriction::One,			InitializerRestriction::Optional	}; } // Variable with Initializer
inline ParsingDeclaratorArguments	pda_VarNoInit()
	{	return { nullptr,	false,			DeclaratorRestriction::One,			InitializerRestriction::Zero		}; } // Variable without Initializer
inline ParsingDeclaratorArguments	pda_Param(bool forParameter)
	{	return { nullptr,	forParameter,	DeclaratorRestriction::Optional,	InitializerRestriction::Optional	}; } // Parameter
inline ParsingDeclaratorArguments	pda_Decls()	
	{	return { nullptr,	false,			DeclaratorRestriction::Many,		InitializerRestriction::Optional	}; } // Declarations

extern void							ParseMemberDeclarator(const ParsingArguments& pa, const ParsingDeclaratorArguments& pda, Ptr<CppTokenCursor>& cursor, List<Ptr<Declarator>>& declarators);
extern void							ParseNonMemberDeclarator(const ParsingArguments& pa, const ParsingDeclaratorArguments& pda, Ptr<CppTokenCursor>& cursor, List<Ptr<Declarator>>& declarators);
extern Ptr<Declarator>				ParseNonMemberDeclarator(const ParsingArguments& pa, const ParsingDeclaratorArguments& pda, Ptr<CppTokenCursor>& cursor);
extern Ptr<Type>					ParseType(const ParsingArguments& pa, Ptr<CppTokenCursor>& cursor);

// Parser_Declaration.cpp
extern void							ParseDeclaration(const ParsingArguments& pa, Ptr<CppTokenCursor>& cursor, List<Ptr<Declaration>>& output);
extern void							BuildVariables(List<Ptr<Declarator>>& declarators, List<Ptr<VariableDeclaration>>& varDecls);
extern void							BuildSymbols(const ParsingArguments& pa, List<Ptr<VariableDeclaration>>& varDecls);
extern void							BuildVariablesAndSymbols(const ParsingArguments& pa, List<Ptr<Declarator>>& declarators, List<Ptr<VariableDeclaration>>& varDecls);
extern Ptr<VariableDeclaration>		BuildVariableAndSymbol(const ParsingArguments& pa, Ptr<Declarator> declarator);

extern Ptr<Expr>					ParseExpr(const ParsingArguments& pa, bool allowComma, Ptr<CppTokenCursor>& cursor);
extern Ptr<Stat>					ParseStat(const ParsingArguments& pa, Ptr<CppTokenCursor>& cursor);
extern Ptr<Program>					ParseProgram(const ParsingArguments& pa, Ptr<CppTokenCursor>& cursor);

/***********************************************************************
Helpers
***********************************************************************/

// Test if the next token's content matches the expected value
__forceinline bool TestToken(Ptr<CppTokenCursor>& cursor, const wchar_t* content, bool autoSkip = true)
{
	vint length = (vint)wcslen(content);
	if (cursor && cursor->token.length == length && wcsncmp(cursor->token.reading, content, length) == 0)
	{
		if (autoSkip) cursor = cursor->Next();
		return true;
	}
	return false;
}

// Test if the next token's type matches the expected value
__forceinline bool TestToken(Ptr<CppTokenCursor>& cursor, CppTokens token1, bool autoSkip = true)
{
	if (cursor && (CppTokens)cursor->token.token == token1)
	{
		if (autoSkip) cursor = cursor->Next();
		return true;
	}
	return false;
}

#define TEST_AND_SKIP(TOKEN)\
	if (TestToken(current, TOKEN, false) && current->token.start == start)\
	{\
		start += current->token.length;\
		current = current->Next();\
	}\
	else\
	{\
		return false;\
	}\

// Test if next two tokens' types match expected value, and there should not be spaces between tokens
__forceinline bool TestToken(Ptr<CppTokenCursor>& cursor, CppTokens token1, CppTokens token2, bool autoSkip = true)
{
	if (auto current = cursor)
	{
		vint start = current->token.start;
		TEST_AND_SKIP(token1);
		TEST_AND_SKIP(token2);
		if (autoSkip) cursor = current;
		return true;
	}
	return false;
}

// Test if next three tokens' types match expected value, and there should not be spaces between tokens
__forceinline bool TestToken(Ptr<CppTokenCursor>& cursor, CppTokens token1, CppTokens token2, CppTokens token3, bool autoSkip = true)
{
	if (auto current = cursor)
	{
		vint start = current->token.start;
		TEST_AND_SKIP(token1);
		TEST_AND_SKIP(token2);
		TEST_AND_SKIP(token3);
		if (autoSkip) cursor = current;
		return true;
	}
	return false;
}

// Throw exception if failed to test
__forceinline void RequireToken(Ptr<CppTokenCursor>& cursor, const wchar_t* content)
{
	if (!TestToken(cursor, content))
	{
		throw StopParsingException(cursor);
	}
}

// Throw exception if failed to test
__forceinline void RequireToken(Ptr<CppTokenCursor>& cursor, CppTokens token1)
{
	if (!TestToken(cursor, token1))
	{
		throw StopParsingException(cursor);
	}
}

// Throw exception if failed to test
__forceinline void RequireToken(Ptr<CppTokenCursor>& cursor, CppTokens token1, CppTokens token2)
{
	if (!TestToken(cursor, token1, token2))
	{
		throw StopParsingException(cursor);
	}
}

// Throw exception if failed to test
__forceinline void RequireToken(Ptr<CppTokenCursor>& cursor, CppTokens token1, CppTokens token2, CppTokens token3)
{
	if (!TestToken(cursor, token1, token2, token3))
	{
		throw StopParsingException(cursor);
	}
}

// Skip one token
__forceinline void SkipToken(Ptr<CppTokenCursor>& cursor)
{
	if (cursor)
	{
		cursor = cursor->Next();
	}
	else
	{
		throw StopParsingException(cursor);
	}
}

#endif