/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "tokenize.h"
#include "checkclass.h"
#include "testsuite.h"
#include <sstream>

extern std::ostringstream errout;

class TestClass : public TestFixture
{
public:
    TestClass() : TestFixture("TestClass")
    { }

private:

    void run()
    {
        TEST_CASE(virtualDestructor1);  // Base class not found => no error
        TEST_CASE(virtualDestructor2);      // Base class doesn't have a destructor
        TEST_CASE(virtualDestructor3);  // Base class has a destructor, but it's not virtual
        TEST_CASE(virtualDestructor4);  // Derived class doesn't have a destructor => no error
        TEST_CASE(virtualDestructor5);  // Derived class has empty destructor => no error
        TEST_CASE(virtualDestructorProtected);
        TEST_CASE(virtualDestructorInherited);
        TEST_CASE(virtualDestructorTemplate);

        TEST_CASE(uninitVar1);
        TEST_CASE(uninitVar2);
        TEST_CASE(uninitVarEnum);
        TEST_CASE(uninitVarStream);
        TEST_CASE(uninitVarTypedef);
        TEST_CASE(uninitVarArray1);
        TEST_CASE(uninitVarArray2);
        TEST_CASE(uninitVarArray2D);
        TEST_CASE(uninitMissingFuncDef);// can't expand function in constructor
        TEST_CASE(privateCtor1);        // If constructor is private..
        TEST_CASE(privateCtor2);        // If constructor is private..
        TEST_CASE(function);            // Function is not variable
        TEST_CASE(uninitVarHeader1);    // Class is defined in header
        TEST_CASE(uninitVarHeader2);    // Class is defined in header
        TEST_CASE(uninitVarHeader3);    // Class is defined in header
        TEST_CASE(uninitVarPublished);  // Variables in the published section are auto-initialized
        TEST_CASE(uninitOperator);      // No FP about uninitialized 'operator[]'

        TEST_CASE(noConstructor1);
        TEST_CASE(noConstructor2);
        TEST_CASE(noConstructor3);
        TEST_CASE(noConstructor4);

        TEST_CASE(operatorEq1);
        TEST_CASE(operatorEqRetRefThis1);
        TEST_CASE(operatorEqRetRefThis2); // ticket #1323
        TEST_CASE(operatorEqToSelf1);   // single class
        TEST_CASE(operatorEqToSelf2);   // nested class
        TEST_CASE(operatorEqToSelf3);   // multiple inheritance
        TEST_CASE(operatorEqToSelf4);   // nested class with multiple inheritance
        TEST_CASE(operatorEqToSelf5);   // ticket # 1233
        TEST_CASE(memsetOnStruct);
        TEST_CASE(memsetOnClass);

        TEST_CASE(this_subtraction);    // warn about "this-x"

        // can member function be made const
        TEST_CASE(const1);
        TEST_CASE(constoperator);   // operator< can often be const
        TEST_CASE(constincdec);     // increment/decrement => non-const
        TEST_CASE(constReturnReference);
    }

    // Check the operator Equal
    void checkOpertorEq(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.operatorEq();
    }

    void operatorEq1()
    {
        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void goo() {}"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'operator=' should return something\n", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "private:\n"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void goo() {}\n"
                       "private:\n"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEq("class A\n"
                       "{\n"
                       "public:\n"
                       "    void operator=(const A&);\n"
                       "};\n"
                       "class B\n"
                       "{\n"
                       "public:\n"
                       "    void operator=(const B&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'operator=' should return something\n"
                      "[test.cpp:9]: (style) 'operator=' should return something\n", errout.str());

        checkOpertorEq("struct A\n"
                       "{\n"
                       "    void operator=(const A&);\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) 'operator=' should return something\n", errout.str());
    }

    // Check that operator Equal returns reference to this
    void checkOpertorEqRetRefThis(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.operatorEqRetRefThis();
    }

    void operatorEqRetRefThis1()
    {
        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return *this; }\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return a; }\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &)\n"
            "};\n"
            "A & A::operator=(const A &a) { return a; }\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a)\n"
            "};\n"
            "A & A::operator=(const A &a) { return a; }\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { return *this; }\n"
            "    };\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { return b; }\n"
            "    };\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:7]: (style) 'operator=' should return reference to self\n", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqRetRefThis(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { return b; }\n");
        ASSERT_EQUALS("[test.cpp:10]: (style) 'operator=' should return reference to self\n", errout.str());
    }

    void operatorEqRetRefThis2()
    {
        // ticket # 1323
        checkOpertorEqRetRefThis(
            "class szp\n"
            "{\n"
            "  szp &operator =(int *other) {};\n"
            "};");
        ASSERT_EQUALS("", errout.str());
    }

    // Check that operator Equal checks for assignment to self
    void checkOpertorEqToSelf(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
        settings._showAll = true;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.operatorEqToSelf();
    }

    void operatorEqToSelf1()
    {
        // this test has an assignment test but it is not needed
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { if (&a != this) { } return *this; }\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but it is not needed
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return *this; }\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test and has it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if (&a != this)\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this class needs an assignment test but doesn't have it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "        return *this;\n"
            "    }\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (possible style) 'operator=' should check for assignment to self\n", errout.str());

        // this test has an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { if (&a != this) { } return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &)\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test and has it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & operator=(const A &a)\n"
            "{\n"
            "    if (&a != this)\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "    }\n"
            "    return *this;\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test but doesnt have it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & operator=(const A &a)\n"
            "{\n"
            "    free(s);\n"
            "    s = strdup(a.s);\n"
            "    return *this;\n"
            "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (possible style) 'operator=' should check for assignment to self\n", errout.str());

        // ticket #1224
        checkOpertorEqToSelf(
            "const SubTree &SubTree::operator= (const SubTree &b)\n"
            "{\n"
            "    CodeTree *oldtree = tree;\n"
            "    tree = new CodeTree(*b.tree);\n"
            "    delete oldtree;\n"
            "    return *this;\n"
            "}\n"
            "const SubTree &SubTree::operator= (const CodeTree &b)\n"
            "{\n"
            "    CodeTree *oldtree = tree;\n"
            "    tree = new CodeTree(b);\n"
            "    delete oldtree;\n"
            "    return *this;\n"
            "}");
        ASSERT_EQUALS("", errout.str());

    }

    void operatorEqToSelf2()
    {
        // this test has an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { if (&b != this) { } return *this; }\n"
            "    };\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { return *this; }\n"
            "    };\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test but has it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        char *s;\n"
            "        B & operator=(const B &b)\n"
            "        {\n"
            "            if (&b != this)\n"
            "            {\n"
            "            }\n"
            "            return *this;\n"
            "        }\n"
            "    };\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test but doesn't have it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        char *s;\n"
            "        B & operator=(const B &b)\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(b.s);\n"
            "            return *this;\n"
            "        }\n"
            "    };\n"
            "};\n");
        ASSERT_EQUALS("[test.cpp:8]: (possible style) 'operator=' should check for assignment to self\n", errout.str());

        // this test has an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { if (&b != this) { } return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        // this test doesn't have an assignment test but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test and has it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        char * s;\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b)\n"
            "{\n"
            "    if (&b != this)\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(b.s);\n"
            "    }\n"
            "    return *this;\n"
            " }\n");
        ASSERT_EQUALS("", errout.str());

        // this test needs an assignment test but doesn't have it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B\n"
            "    {\n"
            "    public:\n"
            "        char * s;\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b)\n"
            "{\n"
            "    free(s);\n"
            "    s = strdup(b.s);\n"
            "    return *this;\n"
            " }\n");
        ASSERT_EQUALS("[test.cpp:11]: (possible style) 'operator=' should check for assignment to self\n", errout.str());
    }

    void operatorEqToSelf3()
    {
        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &a) { return *this; }\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance and needs an assignment test but there is no trivial way to test for it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        free(s);\n"
            "        s = strdup(a.s);\n"
            "        return *this;\n"
            "    }\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a) { return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance and needs an assignment test but there is no trivial way to test for it
        checkOpertorEqToSelf(
            "class A : public B, public C\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &);\n"
            "};\n"
            "A & A::operator=(const A &a)\n"
            "{\n"
            "    free(s);\n"
            "    s = strdup(a.s);\n"
            "    return *this;\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqToSelf4()
    {
        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B : public C, public D\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &b) { return *this; }\n"
            "    };\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance and needs an assignment test but there is no trivial way to test for it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B : public C, public D\n"
            "    {\n"
            "    public:\n"
            "        char * s;\n"
            "        B & operator=(const B &b)\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(b.s);\n"
            "            return *this;\n"
            "        }\n"
            "    };\n"
            "};\n");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance so there is no trivial way to test for self assignment but doesn't need it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B : public C, public D\n"
            "    {\n"
            "    public:\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b) { return *this; }\n");
        ASSERT_EQUALS("", errout.str());

        // this test has multiple inheritance and needs an assignment test but there is no trivial way to test for it
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    class B : public C, public D\n"
            "    {\n"
            "    public:\n"
            "        char * s;\n"
            "        B & operator=(const B &);\n"
            "    };\n"
            "};\n"
            "A::B & A::B::operator=(const A::B &b)\n"
            "{\n"
            "    free(s);\n"
            "    s = strdup(b.s);\n"
            "    return *this;\n"
            "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void operatorEqToSelf5()
    {
        // ticket # 1233
        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if((&a!=this))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if(!(&a==this))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if(false==(&a==this))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());

        checkOpertorEqToSelf(
            "class A\n"
            "{\n"
            "public:\n"
            "    char *s;\n"
            "    A & operator=(const A &a)\n"
            "    {\n"
            "        if(true!=(&a==this))\n"
            "        {\n"
            "            free(s);\n"
            "            s = strdup(a.s);\n"
            "        }\n"
            "        return *this;\n"
            "    }\n"
            "};");
        ASSERT_EQUALS("", errout.str());
    }

    // Check that base classes have virtual destructors
    void checkVirtualDestructor(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.virtualDestructor();
    }

    void virtualDestructor1()
    {
        // Base class not found

        checkVirtualDestructor("class Derived : public Base { };");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Derived : Base { };");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor2()
    {
        // Base class doesn't have a destructor

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());

        checkVirtualDestructor("class Base { };\n"
                               "class Derived : Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor3()
    {
        // Base class has a destructor, but it's not virtual

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : private Fred, public Base { public: ~Derived() { (void)11; } };");
        ASSERT_EQUALS("[test.cpp:1]: (error) Class Base which is inherited by class Derived does not have a virtual destructor\n", errout.str());
    }

    void virtualDestructor4()
    {
        // Derived class doesn't have a destructor => no error

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { };");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : private Fred, public Base { };");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructor5()
    {
        // Derived class has empty destructor => no error

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived() {} };");
        ASSERT_EQUALS("", errout.str());

        checkVirtualDestructor("class Base { public: ~Base(); };\n"
                               "class Derived : public Base { public: ~Derived(); }; Derived::~Derived() {}");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructorProtected()
    {
        // Base class has protected destructor, it makes Base *p = new Derived(); fail
        // during compilation time, so error is not possible. => no error
        checkVirtualDestructor("class A\n"
                               "{\n"
                               "protected:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void virtualDestructorInherited()
    {
        // class A inherits virtual destructor from class Base -> no error
        checkVirtualDestructor("class Base\n"
                               "{\n"
                               "public:\n"
                               "virtual ~Base() {}\n"
                               "};\n"
                               "class A : private Base\n"
                               "{\n"
                               "public:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};\n");
        ASSERT_EQUALS("", errout.str());

        // class A inherits virtual destructor from struct Base -> no error
        // also notice that public is not given, but destructor is public, because
        // we are using struct instead of class
        checkVirtualDestructor("struct Base\n"
                               "{\n"
                               "virtual ~Base() {}\n"
                               "};\n"
                               "class A : public Base\n"
                               "{\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};\n");
        ASSERT_EQUALS("", errout.str());

        // Unknown Base class -> it could have virtual destructor, so ignore
        checkVirtualDestructor("class A : private Base\n"
                               "{\n"
                               "public:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};\n");
        ASSERT_EQUALS("", errout.str());

        // Virtual destructor is inherited -> no error
        checkVirtualDestructor("class Base2\n"
                               "{\n"
                               "virtual ~Base2() {}\n"
                               "};\n"
                               "class Base : public Base2\n"
                               "{\n"
                               "};\n"
                               "class A : private Base\n"
                               "{\n"
                               "public:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};\n");
        ASSERT_EQUALS("", errout.str());

        // class A doesn't inherit virtual destructor from class Base -> error
        checkVirtualDestructor("class Base\n"
                               "{\n"
                               "public:\n"
                               "~Base() {}\n"
                               "};\n"
                               "class A : private Base\n"
                               "{\n"
                               "public:\n"
                               "    ~A() { }\n"
                               "};\n"
                               "\n"
                               "class B : public A\n"
                               "{\n"
                               "public:\n"
                               "    ~B() { int a; }\n"
                               "};\n");
        TODO_ASSERT_EQUALS("[test.cpp:7]: (error) Class A which is inherited by class B does not have a virtual destructor\n", errout.str());
    }

    void virtualDestructorTemplate()
    {
        checkVirtualDestructor("template <typename T> class A\n"
                               "{\n"
                               " public:\n"
                               " virtual ~A(){}\n"
                               "};\n"
                               "template <typename T> class AA\n"
                               "{\n"
                               " public:\n"
                               " ~AA(){}\n"
                               "};\n"
                               "class B : public A<int>, public AA<double>\n"
                               "{\n"
                               " public:\n"
                               " ~B(){int a;}\n"
                               "};\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Class AA<double> which is inherited by class B does not have a virtual destructor\n", errout.str());
    }

    void checkUninitVar(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._showAll = true;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.constructors();
    }

    void uninitVar1()
    {
        checkUninitVar("enum ECODES\n"
                       "{\n"
                       "    CODE_1 = 0,\n"
                       "    CODE_2 = 1\n"
                       "};\n"
                       "\n"
                       "class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() {}\n"
                       "\n"
                       "private:\n"
                       "    ECODES _code;\n"
                       "};\n");

        ASSERT_EQUALS("[test.cpp:10]: (style) Member variable not initialized in the constructor 'Fred::_code'\n", errout.str());


        checkUninitVar("class A{};\n"
                       "\n"
                       "class B : public A\n"
                       "{\n"
                       "public:\n"
                       "  B() {}\n"
                       "private:\n"
                       "  float f;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:6]: (style) Member variable not initialized in the constructor 'B::f'\n", errout.str());

        checkUninitVar("class C\n"
                       "{\n"
                       "    FILE *fp;\n"
                       "\n"
                       "public:\n"
                       "    C(FILE *fp);\n"
                       "};\n"
                       "\n"
                       "C::C(FILE *fp) {\n"
                       "    C::fp = fp;\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVar2()
    {
        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { (*this).i = 0; }\n"
                       "private:\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray1()
    {
        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() {}\n"
                       "\n"
                       "private:\n"
                       "    char name[255];\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Member variable not initialized in the constructor 'John::name'\n", errout.str());

        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() {John::name[0] = '\\0';}\n"
                       "\n"
                       "private:\n"
                       "    char name[255];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { strcpy(name, ""); }\n"
                       "\n"
                       "private:\n"
                       "    char name[255];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { }\n"
                       "\n"
                       "    double  operator[](const unsigned long i);\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class A;\n"
                       "class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { }\n"
                       "    A a[5];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        checkUninitVar("class A;\n"
                       "class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { }\n"
                       "    A *a[5];\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:5]: (style) Member variable not initialized in the constructor 'John::a'\n", errout.str());
    }

    void uninitVarArray2()
    {
        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { *name = 0; }\n"
                       "\n"
                       "private:\n"
                       "    char name[255];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarArray2D()
    {
        checkUninitVar("class John\n"
                       "{\n"
                       "public:\n"
                       "    John() { a[0][0] = 0; }\n"
                       "\n"
                       "private:\n"
                       "    char a[2][2];\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitMissingFuncDef()
    {
        // Unknown member function
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    void Init();"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        // Unknown non-member function (friend class)
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    friend ABC;\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        // Unknown non-member function (is Init a virtual function?)
        checkUninitVar("class Fred : private ABC\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("", errout.str());

        // Unknown non-member function
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { Init(); }\n"
                       "private:\n"
                       "    int i;\n"
                       "};\n");
        ASSERT_EQUALS("[test.cpp:4]: (style) Member variable not initialized in the constructor 'Fred::i'\n", errout.str());
    }

    void uninitVarEnum()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    enum abc {a,b,c};\n"
                       "    Fred() {}\n"
                       "private:\n"
                       "    unsigned int i;\n"
                       "};\n");

        ASSERT_EQUALS("[test.cpp:5]: (style) Member variable not initialized in the constructor 'Fred::i'\n", errout.str());
    }

    void uninitVarStream()
    {
        checkUninitVar("#include <fstream>\n"
                       "class Foo\n"
                       "{\n"
                       "private:\n"
                       "    int foo;\n"
                       "public:\n"
                       "    Foo(std::istream &in)\n"
                       "    {\n"
                       "        if(!(in >> foo))\n"
                       "            throw 0;\n"
                       "    }\n"
                       "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarTypedef()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "public:\n"
                       "    typedef int * pointer;\n"
                       "    Foo() : a(0) {}\n"
                       "    pointer a;\n"
                       "};\n");

        ASSERT_EQUALS("", errout.str());
    }

    void privateCtor1()
    {
        checkUninitVar("class Foo {\n"
                       "    int foo;\n"
                       "    Foo() { }\n"
                       "};\n");

        ASSERT_EQUALS("[test.cpp:3]: (possible style) Member variable not initialized in the constructor 'Foo::foo'\n", errout.str());
    }

    void privateCtor2()
    {
        checkUninitVar("class Foo\n"
                       "{\n"
                       "private:\n"
                       "    int foo;\n"
                       "    Foo() { }\n"
                       "public:\n"
                       "    Foo(int _i) { }\n"
                       "};\n");

        ASSERT_EQUALS("[test.cpp:5]: (possible style) Member variable not initialized in the constructor 'Foo::foo'\n"
                      "[test.cpp:7]: (possible style) Member variable not initialized in the constructor 'Foo::foo'\n", errout.str());
    }


    void function()
    {
        checkUninitVar("class A\n"
                       "{\n"
                       "public:\n"
                       "    A();\n"
                       "    int* f(int*);\n"
                       "};\n"
                       "\n"
                       "A::A()\n"
                       "{\n"
                       "}\n"
                       "\n"
                       "int* A::f(int* p)\n"
                       "{\n"
                       "    return p;\n"
                       "}\n");

        ASSERT_EQUALS("", errout.str());
    }


    void uninitVarHeader1()
    {
        checkUninitVar("#file \"fred.h\"\n"
                       "class Fred\n"
                       "{\n"
                       "private:\n"
                       "    unsigned int i;\n"
                       "public:\n"
                       "    Fred();\n"
                       "};\n"
                       "#endfile\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitVarHeader2()
    {
        checkUninitVar("#file \"fred.h\"\n"
                       "class Fred\n"
                       "{\n"
                       "private:\n"
                       "    unsigned int i;\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "};\n"
                       "#endfile\n");
        ASSERT_EQUALS("[fred.h:6]: (style) Member variable not initialized in the constructor 'Fred::i'\n", errout.str());
    }

    void uninitVarHeader3()
    {
        checkUninitVar("#file \"fred.h\"\n"
                       "class Fred\n"
                       "{\n"
                       "private:\n"
                       "    mutable int i;\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "};\n"
                       "#endfile\n");
        ASSERT_EQUALS("[fred.h:6]: (style) Member variable not initialized in the constructor 'Fred::i'\n", errout.str());
    }


    void uninitVarPublished()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "__published:\n"
                       "    int *i;\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void uninitOperator()
    {
        checkUninitVar("class Fred\n"
                       "{\n"
                       "public:\n"
                       "    Fred() { }\n"
                       "    int *operator [] (int index) { return 0; }\n"
                       "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void checkNoConstructor(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.constructors();
    }

    void noConstructor1()
    {
        // There are nonstatic member variables - constructor is needed
        checkNoConstructor("class Fred\n"
                           "{\n"
                           "    int i;\n"
                           "};\n");
        ASSERT_EQUALS("[test.cpp:1]: (style) The class 'Fred' has no constructor. Member variables not initialized.\n", errout.str());
    }

    void noConstructor2()
    {
        checkNoConstructor("class Fred\n"
                           "{\n"
                           "public:\n"
                           "    static void foobar();\n"
                           "};\n"
                           "\n"
                           "void Fred::foobar()\n"
                           "{ }\n");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor3()
    {
        checkNoConstructor("class Fred\n"
                           "{\n"
                           "private:\n"
                           "    static int foobar;\n"
                           "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void noConstructor4()
    {
        checkNoConstructor("class Fred\n"
                           "{\n"
                           "public:\n"
                           "    int foobar;\n"
                           "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    void checkNoMemset(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");


        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.noMemset();
    }

    void memsetOnClass()
    {
        checkNoMemset("class A\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      " A fail;\n"
                      " memset(&fail, 0, sizeof(A));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:7]: (error) Using 'memset' on class\n", errout.str());

        checkNoMemset("struct A\n"
                      "{\n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      " struct A fail;\n"
                      " memset(&fail, 0, sizeof(A));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void memsetOnStruct()
    {
        checkNoMemset("class A\n"
                      "{\n"
                      " void g( struct sockaddr_in6& a);\n"
                      "private:\n"
                      " std::string b; \n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      " struct sockaddr_in6 fail;\n"
                      " memset(&fail, 0, sizeof(struct sockaddr_in6));\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());

        checkNoMemset("struct A\n"
                      "{\n"
                      " void g( struct sockaddr_in6& a);\n"
                      "private:\n"
                      " std::string b; \n"
                      "};\n"
                      "void f()\n"
                      "{\n"
                      " struct A fail;\n"
                      " memset(&fail, 0, sizeof(struct A));\n"
                      "}\n");
        ASSERT_EQUALS("[test.cpp:10]: (error) Using 'memset' on struct that contains a 'std::string'\n", errout.str());
    }


    void checkThisSubtraction(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
        settings._showAll = true;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.thisSubtraction();
    }

    void this_subtraction()
    {
        checkThisSubtraction("; this-x ;");
        ASSERT_EQUALS("[test.cpp:1]: (possible style) Suspicious pointer subtraction\n", errout.str());

        checkThisSubtraction("; *this = *this-x ;");
        ASSERT_EQUALS("", errout.str());

        checkThisSubtraction("; *this = *this-x ;\n"
                             "this-x ;");
        ASSERT_EQUALS("[test.cpp:2]: (possible style) Suspicious pointer subtraction\n", errout.str());

        checkThisSubtraction("; *this = *this-x ;\n"
                             "this-x ;\n"
                             "this-x ;\n");
        ASSERT_EQUALS("[test.cpp:2]: (possible style) Suspicious pointer subtraction\n"
                      "[test.cpp:3]: (possible style) Suspicious pointer subtraction\n", errout.str());
    }

    void checkConst(const char code[])
    {
        // Tokenize..
        Tokenizer tokenizer;
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList();

        // Clear the error log
        errout.str("");

        // Check..
        Settings settings;
        settings._checkCodingStyle = true;
        CheckClass checkClass(&tokenizer, &settings, this);
        checkClass.checkConst();
    }

    void const1()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int getA() { return a; }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::getA' can be const\n", errout.str());

        // constructors can't be const..
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "public:\n"
                   "    Fred() { }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // assignment through |=..
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int setA() { a |= true; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // functions with a function call can't be const..
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    int x;\n"
                   "    void b() { a(); }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());

        // static functions can't be const..
        checkConst("class foo\n"
                   "{\n"
                   "public:\n"
                   "    static unsigned get()\n"
                   "    { return 0; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    // operator< can often be const
    void constoperator()
    {
        checkConst("struct Fred {\n"
                   "    int a;\n"
                   "    bool operator<(const Fred &f) { return (a < f.a); }\n"
                   "};\n");
        ASSERT_EQUALS("[test.cpp:3]: (style) The function 'Fred::operator<' can be const\n", errout.str());
    }

    // increment/decrement => not const
    void constincdec()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    void nextA() { return ++a; }\n"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }

    // return pointer/reference => not const
    void constReturnReference()
    {
        checkConst("class Fred {\n"
                   "    int a;\n"
                   "    int &getR() { return a; }\n"
                   "    int *getP() { return &a; }"
                   "};\n");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestClass)
