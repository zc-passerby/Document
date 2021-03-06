#include <Ast_Decl.h>
#include "Util.h"

TEST_CASE(TestParseDecl_Namespaces)
{
	auto input = LR"(
namespace vl {}
namespace vl::presentation::controls {}
)";
	auto output = LR"(
namespace vl
{
}
namespace vl
{
	namespace presentation
	{
		namespace controls
		{
		}
	}
}
)";
	AssertProgram(input, output);
}

TEST_CASE(TestParseDecl_Enums)
{
	auto input = LR"(
enum e1;
enum e2 : int;
enum class e3;
enum class e4 : int;
enum e5
{
	x,
	y = 1
};
enum class e6
{
	x,
	y,
	z,
};
)";
	auto output = LR"(
__forward enum e1;
__forward enum e2 : int;
__forward enum class e3;
__forward enum class e4 : int;
enum e5
{
	x,
	y = 1,
};
enum class e6
{
	x,
	y,
	z,
};
)";
	AssertProgram(input, output);
}

TEST_CASE(TestParseDecl_EnumsConnectForward)
{
	auto input = LR"(
namespace a::b
{
	enum class A;
	enum class A;
}
namespace a::b
{
	enum class A {};
}
namespace a::b
{
	enum class A;
	enum class A;
}
)";
	COMPILE_PROGRAM(program, pa, input);
	TEST_ASSERT(pa.root->children[L"a"].Count() == 1);
	TEST_ASSERT(pa.root->children[L"a"][0]->children[L"b"].Count() == 1);
	TEST_ASSERT(pa.root->children[L"a"][0]->children[L"b"][0]->children[L"A"].Count() == 5);
	const auto& symbols = pa.root->children[L"a"][0]->children[L"b"][0]->children[L"A"];

	for (vint i = 0; i < 5; i++)
	{
		auto& symbol = symbols[i];
		if (i == 2)
		{
			TEST_ASSERT(symbol->isForwardDeclaration == false);
			TEST_ASSERT(symbol->forwardDeclarationRoot == nullptr);
			TEST_ASSERT(symbol->forwardDeclarations.Count() == 4);
			TEST_ASSERT(symbol->forwardDeclarations[0] == symbols[0].Obj());
			TEST_ASSERT(symbol->forwardDeclarations[1] == symbols[1].Obj());
			TEST_ASSERT(symbol->forwardDeclarations[2] == symbols[3].Obj());
			TEST_ASSERT(symbol->forwardDeclarations[3] == symbols[4].Obj());
		}
		else
		{
			TEST_ASSERT(symbol->isForwardDeclaration == true);
			TEST_ASSERT(symbol->forwardDeclarationRoot == symbols[2].Obj());
			TEST_ASSERT(symbol->forwardDeclarations.Count() == 0);
		}
	}
}

TEST_CASE(TestParseDecl_Variables)
{
	auto input = LR"(
int x = 0;
int a, b = 0, c(0), d{0};
extern static mutable thread_local register int (*v1)();
)";
	auto output = LR"(
x: int = 0;
a: int;
b: int = 0;
c: int (0);
d: int {0};
__forward extern mutable register static thread_local v1: int () *;
)";
	AssertProgram(input, output);
}

TEST_CASE(TestParseDecl_VariablesConnectForward)
{
	auto input = LR"(
namespace a::b
{
	extern int x;
	extern int x;
}
namespace a::b
{
	int x = 0;
}
namespace a::b
{
	extern int x;
	extern int x;
}
)";
	COMPILE_PROGRAM(program, pa, input);
	TEST_ASSERT(pa.root->children[L"a"].Count() == 1);
	TEST_ASSERT(pa.root->children[L"a"][0]->children[L"b"].Count() == 1);
	TEST_ASSERT(pa.root->children[L"a"][0]->children[L"b"][0]->children[L"x"].Count() == 5);
	const auto& symbols = pa.root->children[L"a"][0]->children[L"b"][0]->children[L"x"];

	for (vint i = 0; i < 5; i++)
	{
		auto& symbol = symbols[i];
		if (i == 2)
		{
			TEST_ASSERT(symbol->isForwardDeclaration == false);
			TEST_ASSERT(symbol->forwardDeclarationRoot == nullptr);
			TEST_ASSERT(symbol->forwardDeclarations.Count() == 4);
			TEST_ASSERT(symbol->forwardDeclarations[0] == symbols[0].Obj());
			TEST_ASSERT(symbol->forwardDeclarations[1] == symbols[1].Obj());
			TEST_ASSERT(symbol->forwardDeclarations[2] == symbols[3].Obj());
			TEST_ASSERT(symbol->forwardDeclarations[3] == symbols[4].Obj());
		}
		else
		{
			TEST_ASSERT(symbol->isForwardDeclaration == true);
			TEST_ASSERT(symbol->forwardDeclarationRoot == symbols[2].Obj());
			TEST_ASSERT(symbol->forwardDeclarations.Count() == 0);
		}
	}
}

TEST_CASE(TestParseDecl_Functions)
{
	auto input = LR"(
int Add(int a, int b);
int Sub(int a, int b) = 0;
friend extern static virtual explicit inline __forceinline int __stdcall Mul(int, int) { return 0; }
friend extern static virtual explicit inline __forceinline int __stdcall Div(int, int) = 0 { return 0; }
)";
	auto output = LR"(
__forward Add: int (a: int, b: int);
__forward Sub: int (a: int, b: int) = 0;
explicit extern friend inline __forceinline static virtual Mul: int (int, int) __stdcall
{
	return 0;
}
explicit extern friend inline __forceinline static virtual Div: int (int, int) __stdcall = 0
{
	return 0;
}
)";
	AssertProgram(input, output);
}

TEST_CASE(TestParseDecl_FunctionsConnectForward)
{
	auto input = LR"(
namespace a::b
{
	extern int Add(int x = 0, int y = 0);
	int Add(int a, int b);
}
namespace a::b
{
	int Add(int, int) { return 0; }
}
namespace a::b
{
	extern int Add(int, int);
	int Add(int = 0, int = 0);
}
)";
	COMPILE_PROGRAM(program, pa, input);
	TEST_ASSERT(pa.root->children[L"a"].Count() == 1);
	TEST_ASSERT(pa.root->children[L"a"][0]->children[L"b"].Count() == 1);
	TEST_ASSERT(pa.root->children[L"a"][0]->children[L"b"][0]->children[L"Add"].Count() == 5);
	const auto& symbols = pa.root->children[L"a"][0]->children[L"b"][0]->children[L"Add"];

	for (vint i = 0; i < 5; i++)
	{
		auto& symbol = symbols[i];
		if (i == 2)
		{
			TEST_ASSERT(symbol->isForwardDeclaration == false);
			TEST_ASSERT(symbol->forwardDeclarationRoot == nullptr);
			TEST_ASSERT(symbol->forwardDeclarations.Count() == 4);
			TEST_ASSERT(symbol->forwardDeclarations[0] == symbols[0].Obj());
			TEST_ASSERT(symbol->forwardDeclarations[1] == symbols[1].Obj());
			TEST_ASSERT(symbol->forwardDeclarations[2] == symbols[3].Obj());
			TEST_ASSERT(symbol->forwardDeclarations[3] == symbols[4].Obj());
		}
		else
		{
			TEST_ASSERT(symbol->isForwardDeclaration == true);
			TEST_ASSERT(symbol->forwardDeclarationRoot == symbols[2].Obj());
			TEST_ASSERT(symbol->forwardDeclarations.Count() == 0);
		}
	}
}

TEST_CASE(TestParseDecl_Classes)
{
	auto input = LR"(
class C1;
struct S1;
union U1;

class C2: C1, public C1, protected C1, private C1 { int x; public: int a,b; protected: int c,d; private: int e,f; };
struct S2: S1, public S1, protected S1, private S1 { int x; public: int a,b; protected: int c,d; private: int e,f; };
union U2: U1, public U1, protected U1, private U1 {};

struct Point { int x; int y; };
)";
	auto output = LR"(
__forward class C1;
__forward struct S1;
__forward union U1;
class C2 : private C1, public C1, protected C1, private C1
{
	private x: int;
	public a: int;
	public b: int;
	protected c: int;
	protected d: int;
	private e: int;
	private f: int;
};
struct S2 : public S1, public S1, protected S1, private S1
{
	public x: int;
	public a: int;
	public b: int;
	protected c: int;
	protected d: int;
	private e: int;
	private f: int;
};
union U2 : public U1, public U1, protected U1, private U1
{
};
struct Point
{
	public x: int;
	public y: int;
};
)";
	AssertProgram(input, output);
}

TEST_CASE(TestParseDecl_ClassesConnectForward)
{
	const wchar_t* inputs[] = {
LR"(
namespace a::b
{
	class X;
	class X;
}
namespace a::b
{
	class X{};
}
namespace a::b
{
	class X;
	class X;
}
)",
LR"(
namespace a::b
{
	struct X;
	struct X;
}
namespace a::b
{
	struct X{};
}
namespace a::b
{
	struct X;
	struct X;
}
)",
LR"(
namespace a::b
{
	union X;
	union X;
}
namespace a::b
{
	union X{};
}
namespace a::b
{
	union X;
	union X;
}
)"
	};

	for (vint i = 0; i < 3; i++)
	{
		COMPILE_PROGRAM(program, pa, inputs[i]);
		TEST_ASSERT(pa.root->children[L"a"].Count() == 1);
		TEST_ASSERT(pa.root->children[L"a"][0]->children[L"b"].Count() == 1);
		TEST_ASSERT(pa.root->children[L"a"][0]->children[L"b"][0]->children[L"X"].Count() == 5);
		const auto& symbols = pa.root->children[L"a"][0]->children[L"b"][0]->children[L"X"];

		for (vint i = 0; i < 5; i++)
		{
			auto& symbol = symbols[i];
			if (i == 2)
			{
				TEST_ASSERT(symbol->isForwardDeclaration == false);
				TEST_ASSERT(symbol->forwardDeclarationRoot == nullptr);
				TEST_ASSERT(symbol->forwardDeclarations.Count() == 4);
				TEST_ASSERT(symbol->forwardDeclarations[0] == symbols[0].Obj());
				TEST_ASSERT(symbol->forwardDeclarations[1] == symbols[1].Obj());
				TEST_ASSERT(symbol->forwardDeclarations[2] == symbols[3].Obj());
				TEST_ASSERT(symbol->forwardDeclarations[3] == symbols[4].Obj());
			}
			else
			{
				TEST_ASSERT(symbol->isForwardDeclaration == true);
				TEST_ASSERT(symbol->forwardDeclarationRoot == symbols[2].Obj());
				TEST_ASSERT(symbol->forwardDeclarations.Count() == 0);
			}
		}
	}
}

TEST_CASE(TestParseDecl_Methods)
{
	auto input = LR"(
struct Vector
{
	double x = 0;
	double y = 0;

	__stdcall Vector();
	Vector(double _x, double _y);
	Vector(const Vector& v);
	Vector(Vector&& v);
	~Vector();

	operator bool()const;
	explicit operator double()const;
	Vector operator*(double z)const;
};
static Vector operator+(Vector v1, Vector v2);
static Vector operator-(Vector v1, Vector v2);
)";
	auto output = LR"(
struct Vector
{
	public x: double = 0;
	public y: double = 0;
	public __forward __ctor $__ctor: __null () __stdcall;
	public __forward __ctor $__ctor: __null (_x: double, _y: double);
	public __forward __ctor $__ctor: __null (v: Vector const &);
	public __forward __ctor $__ctor: __null (v: Vector &&);
	public __forward __dtor ~Vector: __null ();
	public __forward __type $__type: bool () const;
	public __forward explicit __type $__type: double () const;
	public __forward operator *: Vector (z: double) const;
};
__forward static operator +: Vector (v1: Vector, v2: Vector);
__forward static operator -: Vector (v1: Vector, v2: Vector);
)";
	AssertProgram(input, output);
}

TEST_CASE(TestParseDecl_ClassMemberConnectForward)
{
	auto input = LR"(
namespace a::b
{
	class Something
	{
	public:
		static const int a = 0;
		static const int b;

		Something();
		Something(const Something&);
		Something(Something&&);
		explicit Something(int);
		~Something();

		void Do();
		virtual void Do(int) = 0;
		explicit operator bool()const;
		explicit operator bool();
		Something operator++();
		Something operator++(int);
	};
}
namespace a::b
{
	const int Something::b = 0;
	Something::Something(){}
	Something::Something(const Something&){}
	Something::Something(Something&&){}
	Something::Something(int){}
	Something::~Something(){}
	void Something::Do(){}
	void Something::Do(int) {}
	Something::operator bool()const{}
	Something::operator bool(){}
	Something Something::operator++(){}
	Something Something::operator++(int){}
}
)";

	auto output = LR"(
namespace a
{
	namespace b
	{
		class Something
		{
			public static a: int const = 0;
			public __forward static b: int const;
			public __forward __ctor $__ctor: __null ();
			public __forward __ctor $__ctor: __null (Something const &);
			public __forward __ctor $__ctor: __null (Something &&);
			public __forward explicit __ctor $__ctor: __null (int);
			public __forward __dtor ~Something: __null ();
			public __forward Do: void ();
			public __forward virtual Do: void (int) = 0;
			public __forward explicit __type $__type: bool () const;
			public __forward explicit __type $__type: bool ();
			public __forward operator ++: Something ();
			public __forward operator ++: Something (int);
		};
	}
}
namespace a
{
	namespace b
	{
		b: int const (Something ::) = 0;
		__ctor $__ctor: __null () (Something ::)
		{
		}
		__ctor $__ctor: __null (Something const &) (Something ::)
		{
		}
		__ctor $__ctor: __null (Something &&) (Something ::)
		{
		}
		__ctor $__ctor: __null (int) (Something ::)
		{
		}
		__dtor ~Something: __null () (Something ::)
		{
		}
		Do: void () (Something ::)
		{
		}
		Do: void (int) (Something ::)
		{
		}
		__type $__type: bool () const (Something ::)
		{
		}
		__type $__type: bool () (Something ::)
		{
		}
		operator ++: Something () (Something ::)
		{
		}
		operator ++: Something (int) (Something ::)
		{
		}
	}
}
)";

	COMPILE_PROGRAM(program, pa, input);
	AssertProgram(program, output);

	auto& inClassMembers = pa.root->children[L"a"][0]->children[L"b"][0]->children[L"Something"][0]->decls[0].Cast<ClassDeclaration>()->decls;
	TEST_ASSERT(inClassMembers.Count() == 13);

	auto& outClassMembers = pa.root->children[L"a"][0]->children[L"b"][0]->decls[1].Cast<NamespaceDeclaration>()->decls;
	TEST_ASSERT(outClassMembers.Count() == 12);

	for (vint i = 0; i < 12; i++)
	{
		auto inClassSymbol = inClassMembers[i + 1].f1->symbol;
		auto outClassSymbol = outClassMembers[i]->symbol;

		TEST_ASSERT(inClassSymbol->isForwardDeclaration == true);
		TEST_ASSERT(inClassSymbol->forwardDeclarationRoot == outClassSymbol);
		TEST_ASSERT(outClassSymbol->isForwardDeclaration == false);
		TEST_ASSERT(outClassSymbol->forwardDeclarations.Count() == 1);
		TEST_ASSERT(outClassSymbol->forwardDeclarations[0] == inClassSymbol);
	}
}

TEST_CASE(TestParseDecl_ClassMemberScope)
{
	auto input = LR"(
namespace a
{
	struct X
	{
		enum class Y;
	};
	struct Z;
}
namespace b
{
	struct Z : a::X
	{
		Y Do(a::X, X, a::X::Y, X::Y, Y, Z);
	};
}
namespace b
{
	struct X;
	Z::Y Z::Do(a::X, X, a::X::Y, X::Y, Y, Z)
	{
		X x;
		Y y;
		Z z;
	}
}
)";
	auto output = LR"(
namespace a
{
	struct X
	{
		public __forward enum class Y;
	};
	__forward struct Z;
}
namespace b
{
	struct Z : public a :: X
	{
		public __forward Do: Y (a :: X, X, a :: X :: Y, X :: Y, Y, Z);
	};
}
namespace b
{
	__forward struct X;
	Do: Z :: Y (a :: X, X, a :: X :: Y, X :: Y, Y, Z) (Z ::)
	{
		x: X;
		y: Y;
		z: Z;
	}
}
)";

	SortedList<vint> accessed;
	auto recorder = CreateTestIndexRecorder([&](CppName& name, Ptr<Resolving> resolving)
	{
		BEGIN_ASSERT_SYMBOL
			ASSERT_SYMBOL(0, L"a", 11, 12, NamespaceDeclaration, 1, 10)
			ASSERT_SYMBOL(1, L"X", 11, 15, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(2, L"Y", 13, 2, ForwardEnumDeclaration, 5, 13)
			ASSERT_SYMBOL(3, L"a", 13, 7, NamespaceDeclaration, 1, 10)
			ASSERT_SYMBOL(4, L"X", 13, 10, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(5, L"X", 13, 13, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(6, L"a", 13, 16, NamespaceDeclaration, 1, 10)
			ASSERT_SYMBOL(7, L"X", 13, 19, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(8, L"Y", 13, 22, ForwardEnumDeclaration, 5, 13)
			ASSERT_SYMBOL(9, L"X", 13, 25, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(10, L"Y", 13, 28, ForwardEnumDeclaration, 5, 13)
			ASSERT_SYMBOL(11, L"Y", 13, 31, ForwardEnumDeclaration, 5, 13)
			ASSERT_SYMBOL(12, L"Z", 13, 34, ClassDeclaration, 11, 8)
			ASSERT_SYMBOL(13, L"Z", 19, 1, ClassDeclaration, 11, 8)
			ASSERT_SYMBOL(14, L"Y", 19, 4, ForwardEnumDeclaration, 5, 13)
			ASSERT_SYMBOL(15, L"Z", 19, 6, ClassDeclaration, 11, 8)
			ASSERT_SYMBOL(16, L"a", 19, 12, NamespaceDeclaration, 1, 10)
			ASSERT_SYMBOL(17, L"X", 19, 15, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(18, L"X", 19, 18, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(19, L"a", 19, 21, NamespaceDeclaration, 1, 10)
			ASSERT_SYMBOL(20, L"X", 19, 24, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(21, L"Y", 19, 27, ForwardEnumDeclaration, 5, 13)
			ASSERT_SYMBOL(22, L"X", 19, 30, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(23, L"Y", 19, 33, ForwardEnumDeclaration, 5, 13)
			ASSERT_SYMBOL(24, L"Y", 19, 36, ForwardEnumDeclaration, 5, 13)
			ASSERT_SYMBOL(25, L"Z", 19, 39, ClassDeclaration, 11, 8)
			ASSERT_SYMBOL(26, L"X", 21, 2, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(27, L"Y", 22, 2, ForwardEnumDeclaration, 5, 13)
			ASSERT_SYMBOL(28, L"Z", 23, 2, ClassDeclaration, 11, 8)
		END_ASSERT_SYMBOL
	});
	AssertProgram(input, output, recorder);
	TEST_ASSERT(accessed.Count() == 29);
}

TEST_CASE(TestParseDecl_UsingNamespace)
{
	auto input = LR"(
namespace a::b
{
	struct X {};
	enum class Y {};
}
namespace c
{
	using namespace a;
	using namespace a::b;
}
namespace c
{
	struct Z : X
	{
		a::b::Y y1;
		b::Y y2;
		Y y3;
	};
}
)";
	auto output = LR"(
namespace a
{
	namespace b
	{
		struct X
		{
		};
		enum class Y
		{
		};
	}
}
namespace c
{
	using namespace a;
	using namespace a :: b;
}
namespace c
{
	struct Z : public X
	{
		public y1: a :: b :: Y;
		public y2: b :: Y;
		public y3: Y;
	};
}
)";

	SortedList<vint> accessed;
	auto recorder = CreateTestIndexRecorder([&](CppName& name, Ptr<Resolving> resolving)
	{
		BEGIN_ASSERT_SYMBOL
			ASSERT_SYMBOL(0, L"a", 8, 17, NamespaceDeclaration, 1, 10)
			ASSERT_SYMBOL(1, L"a", 9, 17, NamespaceDeclaration, 1, 10)
			ASSERT_SYMBOL(2, L"b", 9, 20, NamespaceDeclaration, 1, 13)
			ASSERT_SYMBOL(3, L"X", 13, 12, ClassDeclaration, 3, 8)
			ASSERT_SYMBOL(4, L"a", 15, 2, NamespaceDeclaration, 1, 10)
			ASSERT_SYMBOL(5, L"b", 15, 5, NamespaceDeclaration, 1, 13)
			ASSERT_SYMBOL(6, L"Y", 15, 8, EnumDeclaration, 4, 12)
			ASSERT_SYMBOL(7, L"b", 16, 2, NamespaceDeclaration, 1, 13)
			ASSERT_SYMBOL(8, L"Y", 16, 5, EnumDeclaration, 4, 12)
			ASSERT_SYMBOL(9, L"Y", 17, 2, EnumDeclaration, 4, 12)
		END_ASSERT_SYMBOL
	});
	AssertProgram(input, output, recorder);
	TEST_ASSERT(accessed.Count() == 10);
}