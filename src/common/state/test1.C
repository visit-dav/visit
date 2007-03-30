#include <string>
#include <AttributeSubject.h>
#include <Observer.h>
#include <BufferConnection.h>

#include <TestUtil.h>

using namespace std;

#define VERBOSE
#define N_TESTS 5

// Prototypes
bool Run_Test1(bool verbose, int *subtest, int *nsubtests);
bool Run_Test2(bool verbose, int *subtest, int *nsubtests);
bool Run_Test3(bool verbose, int *subtest, int *nsubtests);
bool Run_Test4(bool verbose, int *subtest, int *nsubtests);
bool Run_Test5(bool verbose, int *subtest, int *nsubtests);

// *******************************************************************
// Function: main
//
// Purpose:
//   Runs the test functions.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 8 16:31:14 PST 2000
//
// Modifications:
//   
// *******************************************************************

int
main(int argc, char *argv[])
{
    int      subtest, nsubtests;
    bool     test[N_TESTS];
    TestUtil util(argc, argv, "Tests AttributeGroup class");

    // Test sending an AttributeGroup with just "simple" types.
    test[0] = Run_Test1(util.verbose, &subtest, &nsubtests);
    util.PrintTestResult(1, subtest, nsubtests, test[0]);

    // Test sending an AttributeGroup with static arrays of "simple" types.
    test[1] = Run_Test1(util.verbose, &subtest, &nsubtests);
    util.PrintTestResult(2, subtest, nsubtests, test[1]);

    // Test sending an AttributeGroup with vectors of "simple" types.
    test[2] = Run_Test1(util.verbose, &subtest, &nsubtests);
    util.PrintTestResult(3, subtest, nsubtests, test[2]);

    // Test sending an AttributeGroup with a nested AttributeGroup.
    test[3] = Run_Test1(util.verbose, &subtest, &nsubtests);
    util.PrintTestResult(4, subtest, nsubtests, test[3]);

    // Test sending an AttributeGroup with a vector of AttributeGroups.
    test[4] = Run_Test1(util.verbose, &subtest, &nsubtests);
    util.PrintTestResult(5, subtest, nsubtests, test[4]);

    // Look through the results array.
    return util.PassFail(test, N_TESTS);
}

///////////////////////////////////////////////////////////////////////////
/// Test 1 - simple types
///
/// Notes:
///    This test is designed to make sure that simple types can be 
///    communicated properly.
///
///////////////////////////////////////////////////////////////////////////
#define PROPERTY(i, t, n) private: t n; \
public:\
   void Set_##n(t _##n){Select(i, (void *)&n); n = _##n;} \
   t Get_##n() { return n; }

// *******************************************************************
// Class: Test1
//
// Purpose:
//   Tests simple types
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:15:55 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test1 : public AttributeSubject
{
    PROPERTY(0, char, f_char)
    PROPERTY(1, int, f_int)
    PROPERTY(2, long, f_long)
    PROPERTY(3, float, f_float)
    PROPERTY(4, double, f_double)
    PROPERTY(5, string, f_string)
    PROPERTY(6, string, f_string2)
    PROPERTY(7, string, f_string3)
    PROPERTY(8, int, f_int2)
public:
    Test1() : AttributeSubject("cilfdsssi")
    {
        Reset();
    }

    virtual ~Test1()
    {
    }

    virtual void SelectAll()
    {
        Select(0, (void *)&f_char);
        Select(1, (void *)&f_int);
        Select(2, (void *)&f_long);
        Select(3, (void *)&f_float);
        Select(4, (void *)&f_double);
        Select(5, (void *)&f_string);
        Select(6, (void *)&f_string2);
        Select(7, (void *)&f_string3);
        Select(8, (void *)&f_int2);
    }

    void Reset()
    {
        // default values.
        f_char = 'A';
        f_int = f_int2 = 0;
        f_long = 0;
        f_float = 0.;
        f_double = 0.;

        f_string  = string("unset");
        f_string2 = string("unset");
        f_string3 = string("unset");

        UnSelectAll();
    }

    bool operator == (const Test1 &t)
    {
        return (f_char == t.f_char) &&
               (f_int == t.f_int) &&
               (f_long == t.f_long) &&
               (f_float == t.f_float) &&
               (f_double == t.f_double) &&
               (f_string == t.f_string) &&
               (f_string2 == t.f_string2) &&
               (f_string3 == t.f_string3) &&
               (f_int2 == t.f_int2);
    }
};

#ifdef VERBOSE
// *******************************************************************
// Class: Test1Observer
//
// Purpose:
//   Writes out the fields that changed for a Test1 object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:16:18 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test1Observer : public Observer
{
public:
    Test1Observer(Test1 *t) : Observer((Subject *)t)
    {
    }

    virtual ~Test1Observer()
    {
    }

    virtual void Update(Subject *)
    {
        Test1 *t = (Test1 *)subject;

        cout << "{";
        for(int attrId = 0; attrId < t->NumAttributes(); ++attrId)
        {
            if(!t->IsSelected(attrId)) continue;

            switch(attrId)
            {
            case 0:
                cout << "f_char=" << t->Get_f_char() << ", ";
                break;
            case 1:
                cout << "f_int=" << t->Get_f_int() << ", ";
                break;
            case 2:
                cout << "f_long=" << t->Get_f_long() << ", ";
                break;
            case 3:
                cout << "f_float=" << t->Get_f_float() << ", ";
                break;
            case 4:
                cout << "f_double=" << t->Get_f_double() << ", ";
                break;
            case 5:
                cout << "f_string=" << t->Get_f_string() << ", ";
                break;
            case 6:
                cout << "f_string2=" << t->Get_f_string2() << ", ";
                break;
            case 7:
                cout << "f_string3=" << t->Get_f_string3() << ", ";
                break;
            case 8:
                cout << "f_int2=" << t->Get_f_int2();
                break;
            }
        }
        cout << "}" << endl;
    }
};
#endif

// *******************************************************************
// Function: Run_Test1
//
// Purpose:
//   Tests to make sure that simple types can be communicated.
//
// Notes:      
//   This is done by setting values of the attributes in the
//   AttributeGroup, writing to a BufferConnection and then trying
//   to reconstruct the attributes in a different AttributeGroup.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 10:17:13 PDT 2000
//
// Modifications:
//   
// *******************************************************************

bool
Run_Test1(bool verbose, int *subtest, int *nsubtests)
{
    int              wval;
    Test1            t1, t2;
    BufferConnection buf;

    // We have 1 subtest
    *nsubtests = 1;
    *subtest = 1;

#ifdef VERBOSE
    Test1Observer    *TObserver = new Test1Observer(&t2);

    if(verbose)
    {
        cout << "=================================================" << endl;
        cout << "Running Test 1" << endl;
        cout << "=================================================" << endl;
    }
#endif

    bool retval = true;
    for(wval = 0; (wval < 9) && retval; ++wval)
    {
        t1.UnSelectAll();

        // Set fields based on the number.
        t1.Set_f_char('a');
        if(wval > 0)
            t1.Set_f_int(123);
        if(wval > 1)
            t1.Set_f_long(98765);
        if(wval > 2)
            t1.Set_f_float(9.87654321);
        if(wval > 3)
            t1.Set_f_double(1.23456789);
        if(wval > 4)
            t1.Set_f_string("VisIt");
        if(wval > 5)
            t1.Set_f_string2("is");
        if(wval > 6)
            t1.Set_f_string3("cool");
        if(wval > 7)
            t1.Set_f_int2(101010);

        // Write t1 onto buf
        t1.Write(buf);

        // Read t2 from buf and notify TObserver
        t2.Read(buf);
#ifdef VERBOSE
        if(verbose)
            t2.Notify();
#endif
        // Compare the two objects
        retval = (t1 == t2);
    }

#ifdef VERBOSE
    // Destroy the observer.
    delete TObserver;
#endif

    return retval;
}

///////////////////////////////////////////////////////////////////////////
/// Test 2 - Static List types
///
/// Notes:
///    This test is designed to make sure that static sized lists of
///    simple types can be communicated properly.
///
///////////////////////////////////////////////////////////////////////////
#undef PROPERTY
#define PROPERTY(i, t, n, l) private: t n[l]; \
public:\
   void Set_##n(t *_##n){ int j;\
       Select(i, (void *)n, l); \
       for(j = 0; j < l; ++j) \
           n[j] = _##n[j]; \
   } \
   t *Get_##n() { return n; }

// *******************************************************************
// Class: Test2
//
// Purpose:
//   Tests static arrays of simple attributes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:17:03 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test2 : public AttributeSubject
{
    PROPERTY(0, char, f_char, 6)
    PROPERTY(1, int, f_int, 5)
    PROPERTY(2, long, f_long, 3)
    PROPERTY(3, float, f_float, 3)
    PROPERTY(4, string, f_string, 3)
    PROPERTY(5, double, f_double, 4)
public:
    Test2() : AttributeSubject("CILFSD")
    {
        Init();
    }

    virtual ~Test2()
    {
    }

    virtual void SelectAll()
    {
        Select(0, (void *)f_char, 6);
        Select(1, (void *)f_int, 5);
        Select(2, (void *)f_long, 3);
        Select(3, (void *)f_float, 3);
        Select(4, (void *)f_string, 3);
        Select(5, (void *)f_double, 4);
    }

    void Init()
    {
        int  i;
        char c_def[] = {'V','i','s','I','t','\0'};
        int  i_def[] = {0,0,0,0,0};
        long l_def[] = {0,0,0};
        float f_def[] = {0.,0.,0.};
        double d_def[] = {0., 0., 0., 0.};

        // default values.
        for(i = 0; i < 6; ++i)
            f_char[i] = c_def[i];
        for(i = 0; i < 5; ++i)
            f_int[i] = i_def[i];
        for(i = 0; i < 3; ++i)
            f_long[i] = l_def[i];
        for(i = 0; i < 3; ++i)
            f_float[i] = f_def[i];
        for(i = 0; i < 4; ++i)
            f_double[i] = d_def[i];

        f_string[0] = string("unset");
        f_string[1] = string("unset");
        f_string[2] = string("unset");
    }

    void Reset()
    {
        Init();   
        UnSelectAll();
    }

    bool operator == (const Test2 &t)
    {
        return ((0==memcmp((void*)f_char, (void*)t.f_char, 6*sizeof(char))) &&
               (0==memcmp((void*)f_int, (void*)t.f_int, 5*sizeof(int))) &&
               (0==memcmp((void*)f_long, (void*)t.f_long, 3*sizeof(long))) &&
               (0==memcmp((void*)f_float, (void*)t.f_float, 3*sizeof(float))) &&
               (0==memcmp((void*)f_double, (void*)t.f_double, 4*sizeof(double))) &&
               (f_string[0] == t.f_string[0]) &&
               (f_string[1] == t.f_string[1]) &&
               (f_string[2] == t.f_string[2]));
    }
};

#ifdef VERBOSE
// *******************************************************************
// Function: PrintArray
//
// Purpose:
//   Template function to print an array.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:18:24 PST 2000
//
// Modifications:
//   
// *******************************************************************
template <class T>
void
PrintArray(T *array, int len)
{
    int i;

    cout << "{";
    for(i = 0; i < len; ++i)
    {
        cout << array[i];
        if(i < len - 1)
           cout << ", ";
    }
    cout << "}";
}

// *******************************************************************
// Class: Test2Observer
//
// Purpose:
//   Prints the attributes that changed for a Test2 object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:18:59 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test2Observer : public Observer
{
public:
    Test2Observer(Test2 *t) : Observer((Subject *)t)
    {
    }

    virtual ~Test2Observer()
    {
    }

    virtual void Update(Subject *)
    {
        Test2 *t = (Test2 *)subject;

        cout << "{";
        for(int attrId = 0; attrId < t->NumAttributes(); ++attrId)
        {
            if(!t->IsSelected(attrId)) continue;

            switch(attrId)
            {
            case 0:
                cout << "f_char="; PrintArray(t->Get_f_char(), 6); cout << ", ";
                break;
            case 1:
                cout << "f_int="; PrintArray(t->Get_f_int(), 5); cout << ", ";
                break;
            case 2:
                cout << "f_long="; PrintArray(t->Get_f_long(), 3); cout << ", ";
                break;
            case 3:
                cout << "f_float="; PrintArray(t->Get_f_float(), 3); cout << ", ";
                break;
            case 4:
                cout << "f_string="; PrintArray(t->Get_f_string(), 3); cout << ", ";
                break;
            case 5:
                cout << "f_double="; PrintArray(t->Get_f_double(), 4); cout << ", ";
                break;
            }
        }
        cout << "}" << endl;
    }
};
#endif

// *******************************************************************
// Function: Run_Test2
//
// Purpose:
//   Tests to make sure that static sized lists of simple types can
//   be communicated.
//
// Notes:      
//   This is done by setting values of the attributes in the
//   AttributeGroup, writing to a BufferConnection and then trying
//   to reconstruct the attributes in a different AttributeGroup.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 10:17:13 PDT 2000
//
// Modifications:
//   
// *******************************************************************

bool
Run_Test2(bool verbose, int *subtest, int *nsubtests)
{
    int              wval;
    Test2            t1, t2;
    BufferConnection buf;

    // We have 1 subtest
    *nsubtests = 1;
    *subtest = 1;

    char c_def[] = {'V','a','d','e','r','\0'};
    int  i_def[] = {1,2,3,4,5};
    long l_def[] = {1111111,2222222,3333333};
    float f_def[] = {1.1,2.2,3.3};
    double d_def[] = {1.22222, 2.33333, 3.44444, 4.55555};
    string s_def[] = {"Darth", "Vader", "Rules!"};

#ifdef VERBOSE
    Test2Observer    *TObserver = new Test2Observer(&t2);

    if(verbose)
    {
        cout << "=================================================" << endl;
        cout << "Running Test 2" << endl;
        cout << "=================================================" << endl;
    }
#endif

    bool retval = true;
    for(wval = 0; (wval < 6) && retval; ++wval)
    {
        t1.Reset();

        // Set fields based on the number.
        if(wval >= 0)
            t1.Set_f_char(c_def);
        if(wval >= 1)
            t1.Set_f_int(i_def);
        if(wval >= 2)
            t1.Set_f_long(l_def);
        if(wval >= 3)
            t1.Set_f_float(f_def);
        if(wval >= 4)
            t1.Set_f_string(s_def);
        if(wval >= 5)
            t1.Set_f_double(d_def);

        // Write t1 onto buf
        t1.Write(buf);

        // Read t2 from buf and notify TObserver
        t2.Read(buf);
#ifdef VERBOSE
        if(verbose)
            t2.Notify();
#endif
        // Compare the two objects
        retval = (t1 == t2);
    }

#ifdef VERBOSE
    // Destroy the observer.
    delete TObserver;
#endif

    return retval;
}

///////////////////////////////////////////////////////////////////////////
/// Test 3 - Vector types
///
/// Notes:
///    This test is designed to make sure that vectors of simple types can
///    be communicated properly.
///
///////////////////////////////////////////////////////////////////////////
#undef PROPERTY
#define PROPERTY(i, t, n) private: t n; \
public:\
   void Set_##n(const t &_##n){ t::const_iterator pos;\
       Select(i, (void *)&n); \
       for(pos = _##n.begin(); pos != _##n.end(); ++pos) \
           n.push_back(*pos); \
   } \
   t &Get_##n() { return n; }

// *******************************************************************
// Function: CompareVector
//
// Purpose:
//   Template function that compares the elements of two vectors to
//   determine if the vectors are equal.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:19:36 PST 2000
//
// Modifications:
//   
// *******************************************************************

template<class T>
bool
CompareVector(const std::vector<T> &v1, const std::vector<T> &v2)
{
    if(v1.size() != v2.size())
        return false;

    bool retval = true;
    for(int i = 0; (i < v1.size()) && retval; ++i)
        retval = (v1[i] == v2[i]);

    return retval;
}

// *******************************************************************
// Class: Test3
//
// Purpose:
//   Tests vectors of simple types.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:20:30 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test3 : public AttributeSubject
{
    PROPERTY(0, charVector, f_char)
    PROPERTY(1, intVector, f_int)
    PROPERTY(2, longVector, f_long)
    PROPERTY(3, floatVector, f_float)
    PROPERTY(4, stringVector, f_string)
    PROPERTY(5, doubleVector, f_double)
public:
    Test3() : AttributeSubject("c*i*l*f*s*d*")
    {
        Reset();
    }

    virtual ~Test3()
    {
    }

    virtual void SelectAll()
    {
        Select(0, (void *)&f_char);
        Select(1, (void *)&f_int);
        Select(2, (void *)&f_long);
        Select(3, (void *)&f_float);
        Select(4, (void *)&f_string);
        Select(5, (void *)&f_double);

    }

    void Reset()
    {
        f_char.clear();
        f_int.clear();
        f_long.clear();
        f_float.clear();
        f_double.clear();

        UnSelectAll();
    }

    bool operator == (const Test3 &t)
    {
        return (CompareVector(f_char, t.f_char) &&
                CompareVector(f_int, t.f_int) &&
                CompareVector(f_long, t.f_long) &&
                CompareVector(f_float, t.f_float) &&
                CompareVector(f_double, t.f_double) &&
                CompareVector(f_string, t.f_string));
    }
};

#ifdef VERBOSE
// *******************************************************************
// Function: PrintVector
//
// Purpose:
//   Template function to print all the elements in a vector.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:26:04 PST 2000
//
// Modifications:
//   
// *******************************************************************

template <class T>
void
PrintVector(const std::vector<T> &array)
{
    int i;
    cout << "{";
    for(i = 0; i < array.size(); ++i)
    {
        cout << array[i];
        if(i < array.size() - 1)
           cout << ", ";
    }
    cout << "}";
}

// *******************************************************************
// Class: Test3Observer
//
// Purpose:
//   Prints out the attributes that changed for a Test3 object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:26:40 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test3Observer : public Observer
{
public:
    Test3Observer(Test3 *t) : Observer((Subject *)t)
    {
    }

    virtual ~Test3Observer()
    {
    }

    virtual void Update(Subject *)
    {
        Test3 *t = (Test3 *)subject;

        cout << "{";
        for(int attrId = 0; attrId < t->NumAttributes(); ++attrId)
        {
            if(!t->IsSelected(attrId)) continue;

            switch(attrId)
            {
            case 0:
                cout << "f_char="; PrintVector(t->Get_f_char()); cout << ", ";
                break;
            case 1:
                cout << "f_int="; PrintVector(t->Get_f_int()); cout << ", ";
                break;
            case 2:
                cout << "f_long="; PrintVector(t->Get_f_long()); cout << ", ";
                break;
            case 3:
                cout << "f_float="; PrintVector(t->Get_f_float()); cout << ", ";
                break;
            case 4:
                cout << "f_string="; PrintVector(t->Get_f_string()); cout << ", ";
                break;
            case 5:
                cout << "f_double="; PrintVector(t->Get_f_double()); cout << ", ";
                break;
            }
        }
        cout << "}" << endl;
    }
};
#endif

// *******************************************************************
// Function: Run_Test3
//
// Purpose:
//   Tests to make sure that vectors of simple types can be communicated.
//
// Notes:      
//   This is done by setting values of the attributes in the
//   AttributeGroup, writing to a BufferConnection and then trying
//   to reconstruct the attributes in a different AttributeGroup.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 10:17:13 PDT 2000
//
// Modifications:
//   
// *******************************************************************

bool
Run_Test3(bool verbose, int *subtest, int *nsubtests)
{
    int              i, wval;
    Test3            t1, t2;
    BufferConnection buf;

    // We have 1 subtest
    *nsubtests = 1;
    *subtest = 1;

    // Test data
    char c_def[] = {'V','a','d','e','r','\0'};
    int  i_def[] = {1,2,3,4,5};
    long l_def[] = {1111111,2222222,3333333};
    float f_def[] = {1.1,2.2,3.3};
    double d_def[] = {1.22222, 2.33333, 3.44444, 4.55555};
    string s_def[] = {"Darth", "Vader", "Rules!"};

    // Create vectors of test data
    charVector c_vec;
    for(i = 0; i < 6; ++i)
        c_vec.push_back(c_def[i]);
    intVector i_vec;
    for(i = 0; i < 5; ++i)
        i_vec.push_back(i_def[i]);
    longVector l_vec;
    for(i = 0; i < 3; ++i)
        l_vec.push_back(l_def[i]);
    floatVector f_vec;
    for(i = 0; i < 3; ++i)
        f_vec.push_back(f_def[i]);
    doubleVector d_vec;
    for(i = 0; i < 4; ++i)
        d_vec.push_back(d_def[i]);
    stringVector s_vec;
    for(i = 0; i < 3; ++i)
        s_vec.push_back(std::string(s_def[i]));

#ifdef VERBOSE
    Test3Observer    *TObserver = new Test3Observer(&t2);
    if(verbose)
    {
        cout << "=================================================" << endl;
        cout << "Running Test 3" << endl;
        cout << "=================================================" << endl;
    }
#endif

    bool retval = true;
    for(wval = 0; (wval < 6) && retval; ++wval)
    {
        t1.Reset();

        // Set fields based on the number.
        if(wval >= 0)
            t1.Set_f_char(c_vec);
        if(wval >= 1)
            t1.Set_f_int(i_vec);
        if(wval >= 2)
            t1.Set_f_long(l_vec);
        if(wval >= 3)
            t1.Set_f_float(f_vec);
        if(wval >= 4)
            t1.Set_f_string(s_vec);
        if(wval >= 5)
            t1.Set_f_double(d_vec);

        // Write t1 onto buf
        t1.Write(buf);

        // Read t2 from buf and notify TObserver
        t2.Read(buf);

#ifdef VERBOSE
        if(verbose)
            t2.Notify();
#endif
        // Compare the two objects
        retval = (t1 == t2);
    }

#ifdef VERBOSE
    // Destroy the observer.
    delete TObserver;
#endif

    return retval;
}

///////////////////////////////////////////////////////////////////////////
/// Test 4 - Nested types
///
/// Notes:
///    This test is designed to make sure that an AttributeGroup can be
///    nested in another AttributeGroup and be communicated properly.
///
///////////////////////////////////////////////////////////////////////////
#undef PROPERTY
#define PROPERTY(i, t, n) private: t n; \
public:\
   void Set_##n(t _##n){Select(i, (void *)&n); n = _##n;} \
   t Get_##n() { return n; }

// *******************************************************************
// Class: Test4Inner
//
// Purpose:
//   The class that will be nested. It contains just some simple
//   attributes.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:27:22 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test4Inner : public AttributeSubject
{
    PROPERTY(0, string, f_string)
    PROPERTY(1, double, f_double)
public:
    Test4Inner() : AttributeSubject("sd")
    {
        Reset();
    }

    // Copy constructor
    Test4Inner(Test4Inner &t) : AttributeSubject("sd")
    {
        f_string = t.f_string;
        f_double = t.f_double;
    }

    virtual ~Test4Inner()
    {
    }

    virtual void SelectAll()
    {
        Select(0, (void *)&f_string);
        Select(1, (void *)&f_double);
    }

    void Reset()
    {
        // default values.
        f_double = 0.;
        f_string  = string("unset");

        UnSelectAll();
    }

    void operator = (const Test4Inner &t)
    {
        f_string = t.f_string;
        f_double = t.f_double;
    }

    bool operator == (const Test4Inner &t)
    {
        return (f_double == t.f_double) &&
               (f_string == t.f_string);
    }
};

// *******************************************************************
// Class: Test4
//
// Purpose:
//   This class contains some nested AttributeGroups.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:28:07 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test4 : public AttributeSubject
{
    PROPERTY(0, Test4Inner, f_attr)
    PROPERTY(1, int, f_int)
    PROPERTY(2, Test4Inner, f_attr2)
    PROPERTY(3, double, f_double)
    PROPERTY(4, Test4Inner, f_attr3)
public:
    Test4() : AttributeSubject("aiada"), f_attr(), f_attr2(), f_attr3()
    {
        Reset();
    }

    virtual ~Test4()
    {
    }

    virtual void SelectAll()
    {
        Select(0, (void *)&f_attr);
        Select(1, (void *)&f_int);
        Select(2, (void *)&f_attr2);
        Select(3, (void *)&f_double);
        Select(4, (void *)&f_attr3);
    }

    void Reset()
    {
        // default values.
        f_attr.Reset();
        f_int    = 0;
        f_attr2.Reset();
        f_double = 0.;
        f_attr3.Reset();

        UnSelectAll();
    }

    bool operator == (const Test4 &t)
    {
        return (f_attr == t.f_attr) &&
               (f_int == t.f_int) &&
               (f_attr2 == t.f_attr2) &&
               (f_double == t.f_double) &&
               (f_attr3 == t.f_attr3);
    }
};
#ifdef VERBOSE
// *******************************************************************
// Class: Test4Observer
//
// Purpose:
//   Prints out the attributes that changed for a Test4 object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:28:40 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test4Observer : public Observer
{
public:
    Test4Observer(Test4 *t) : Observer((Subject *)t)
    {
    }

    virtual ~Test4Observer()
    {
    }

    virtual void Update(Subject *)
    {
        Test4 *t = (Test4 *)subject;

        cout << "{";
        for(int attrId = 0; attrId < t->NumAttributes(); ++attrId)
        {
            if(!t->IsSelected(attrId)) continue;

            switch(attrId)
            {
            case 0:
                cout << "f_attr={f_string=" << t->Get_f_attr().Get_f_string();
                cout << ", f_double=" << t->Get_f_attr().Get_f_double();
                cout << "}, ";
                break;
            case 1:
                cout << "f_int=" << t->Get_f_int() << ", ";
                break;
            case 2:
                cout << "f_attr2={f_string=" << t->Get_f_attr2().Get_f_string();
                cout << ", f_double=" << t->Get_f_attr2().Get_f_double();
                cout << "}, ";
                break;
            case 3:
                cout << "f_double=" << t->Get_f_double() << ", ";
                break;
            case 4:
                cout << "f_attr3={f_string=" << t->Get_f_attr3().Get_f_string();
                cout << ", f_double=" << t->Get_f_attr3().Get_f_double();
                cout << "}";
                break;
            }
        }
        cout << "}" << endl;
    }
};
#endif

// *******************************************************************
// Function: Run_Test4
//
// Purpose:
//   Tests to make sure that nested types can be communicated.
//
// Notes:      
//   This is done by setting values of the attributes in the
//   AttributeGroup, writing to a BufferConnection and then trying
//   to reconstruct the attributes in a different AttributeGroup.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 10:17:13 PDT 2000
//
// Modifications:
//   
// *******************************************************************

bool
Run_Test4(bool verbose, int *subtest, int *nsubtests)
{
    int              wval;
    Test4            t1, t2;
    BufferConnection buf;

    // We have 1 subtest
    *nsubtests = 1;
    *subtest = 1;

#ifdef VERBOSE
    Test4Observer    *TObserver = new Test4Observer(&t2);
    if(verbose)
    {
        cout << "=================================================" << endl;
        cout << "Running Test 4" << endl;
        cout << "=================================================" << endl;
    }
#endif

    // Create the nested object.
    Test4Inner inner;
    inner.Set_f_string(std::string("VisIt!"));
    inner.Set_f_double(1.2345678);

    Test4Inner inner2;
    inner2.Set_f_string(std::string("Yoda"));
    inner2.Set_f_double(2.3333333);

    Test4Inner inner3;
    inner3.Set_f_string(std::string("Skywalker"));
    inner3.Set_f_double(3.4444444);

    bool retval = true;
    // Try writing the attributes one at a time.
    for(wval = 0; (wval < 5) && retval; ++wval)
    {
        t1.Reset();
        t2.Reset();

        // Set fields based on the number.
        if(wval == 0)
            t1.Set_f_attr(inner);
        if(wval == 1)
            t1.Set_f_int(112233);
        if(wval == 2)
            t1.Set_f_attr2(inner2);
        if(wval == 3)
            t1.Set_f_double(1.2222222);
        if(wval == 4)
            t1.Set_f_attr3(inner3);

        // Write t1 onto buf
        t1.Write(buf);

        // Read t2 from buf and notify TObserver
        t2.Read(buf);

#ifdef VERBOSE
        if(verbose)
            t2.Notify();
#endif
        // Compare the two objects
        retval = (t1 == t2);
    }

    // Try writing multiple attributes
    for(wval = 0; (wval < 5) && retval; ++wval)
    {
        t1.Reset();
        t2.Reset();

        // Set fields based on the number.
        if(wval >= 0)
            t1.Set_f_attr(inner);
        if(wval >= 1)
            t1.Set_f_int(112233);
        if(wval >= 2)
            t1.Set_f_attr2(inner2);
        if(wval >= 3)
            t1.Set_f_double(1.2222222);
        if(wval >= 4)
            t1.Set_f_attr3(inner3);

        // Write t1 onto buf
        t1.Write(buf);

        // Read t2 from buf and notify TObserver
        t2.Read(buf);

#ifdef VERBOSE
        if(verbose)
            t2.Notify();
#endif
        // Compare the two objects
        retval = (t1 == t2);
    }

#ifdef VERBOSE
    // Destroy the observer.
    delete TObserver;
#endif

    return retval;
}

///////////////////////////////////////////////////////////////////////////
/// Test 5 - Vectors of AttributeGroups
///
/// Notes:
///    This test is designed to make sure that a vector of AttributeGroups
///    can be communicated properly.
///
///////////////////////////////////////////////////////////////////////////
#undef PROPERTY
#define PROPERTY(i, t, n) private: t n; \
public:\
   void Set_##n(t _##n){Select(i, (void *)&n); n = _##n;} \
   t Get_##n() { return n; }

// *******************************************************************
// Class: Test5Data
//
// Purpose:
//   This class represents the data that will be stored in a vector.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:29:16 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test5Data : public AttributeSubject
{
    PROPERTY(0, string, f_string)
    PROPERTY(1, double, f_double)
public:
    Test5Data() : AttributeSubject("sd")
    {
        Init();
    }

    // Copy constructor
    Test5Data(const Test5Data &t) : AttributeSubject("sd")
    {
        f_string = t.f_string;
        f_double = t.f_double;
    }

    virtual ~Test5Data()
    {
    }

    virtual void SelectAll()
    {
        Select(0, (void *)&f_string);
        Select(1, (void *)&f_double);
    }

    void Init()
    {
        // default values.
        f_double = 0.;
        f_string  = string("unset");
    }

    void Reset()
    {
        Init();
        UnSelectAll();
    }

    void operator = (const Test5Data &t)
    {
        f_string = t.f_string;
        f_double = t.f_double;
    }

    bool operator == (const Test5Data &t)
    {
        return (f_double == t.f_double) &&
               (f_string == t.f_string);
    }
};

// *******************************************************************
// Function: CompareAttributeGroupVectors
//
// Purpose:
//   Compares two AttributeGroupVectors that contain pointers to
//   Test5Data objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:29:52 PST 2000
//
// Modifications:
//   
// *******************************************************************

bool
CompareAttributeGroupVectors(const AttributeGroupVector &ag1,
    const AttributeGroupVector &ag2)
{
    if(ag1.size() != ag2.size())
        return false;

    bool retval = true;
    for(int i = 0; (i < ag1.size()) && retval; ++i)
    {
        Test5Data *t1 = (Test5Data *)ag1[i];
        Test5Data *t2 = (Test5Data *)ag2[i];

        // Compare the AttributeGroups
        retval = ((*t1) == (*t2));
    }

    return retval;
}

// *******************************************************************
// Class: Test5
//
// Purpose:
//   This class holds an AttributeGroupsVector of Test5Data objects.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:30:42 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test5 : public AttributeSubject
{
    PROPERTY(0, int, f_int)
    PROPERTY(2, float, f_float)
private:
    AttributeGroupVector f_attr;
public:
    void AddData(const Test5Data &data)
    {
        Select(1, (void *)&f_attr);

        // Create a new Test5Data based on the one we passed in and 
        // then add it to the AttributeGroupVector
        Test5Data *n_data = new Test5Data(data);
        f_attr.push_back(n_data);
    }

    // This is a factory method for creating a new instance of
    // a Test5Data object. This is required for an AttributeGroup
    // that contains an AttributeGroupVector. It is called to
    // make new elements when an AttributeGroupVector attribute
    // must grow. The int parameter is the attrId of the 
    // AttributeGroupVector that is growing. Objects that are returned
    // should match the data type being stored in the AttributeGroupVector.
    //
    AttributeGroup *CreateSubAttributeGroup(int)
    {
        return new Test5Data;
    }

    const AttributeGroupVector &Get_f_attr() const { return f_attr; }
public:
    Test5() : AttributeSubject("ia*f")
    {
        Init();
    }

    virtual ~Test5()
    {
        Init();
    }

    virtual void SelectAll()
    {
        Select(0, (void *)&f_int);
        Select(1, (void *)&f_attr);
        Select(2, (void *)&f_float);
    }

    void Init()
    {
        // default values.
        f_int   = 0;
        f_float = 0.;

        // Empty the AttributeGroup vector
        for(int i = 0; i < f_attr.size(); ++i)
        {
            AttributeGroup *ag = (AttributeGroup *)f_attr[i];
            delete ag;
        }
        f_attr.clear();
    }

    void Reset()
    {
        Init();
        UnSelectAll();
    }

    bool operator == (const Test5 &t)
    {
        return (f_int == t.f_int) &&
               (f_float == t.f_float) &&
               CompareAttributeGroupVectors(f_attr, t.f_attr);
    }
};

#ifdef VERBOSE
// Operator to write out a Test5Data
ostream &
operator << (ostream &os, Test5Data &t5)
{
    os << "{f_string=" << t5.Get_f_string() << ", f_double=" <<
          t5.Get_f_double() << "}";

    return os;
}

// Operator to write out an AttributeVector of Test5Data's
ostream &
operator << (ostream &os, const AttributeGroupVector &av)
{
    os << "{";
    for(int i = 0; i < av.size(); ++i)
    {
        Test5Data *t5 = (Test5Data *)av[i];
        os << *t5;
        if(i < av.size() - 1)
           os << ", ";
    }
    os << "}";

    return os;
}

// *******************************************************************
// Class: Test5Observer
//
// Purpose:
//   Prints the fields that changed for a Test5 object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 14:35:20 PST 2000
//
// Modifications:
//   
// *******************************************************************

class Test5Observer : public Observer
{
public:
    Test5Observer(Test5 *t) : Observer((Subject *)t)
    {
    }

    virtual ~Test5Observer()
    {
    }

    virtual void Update(Subject *)
    {
        Test5 *t = (Test5 *)subject;

        cout << "{";
        for(int attrId = 0; attrId < t->NumAttributes(); ++attrId)
        {
            if(!t->IsSelected(attrId)) continue;

            switch(attrId)
            {
            case 0:
                cout << "f_int=" << t->Get_f_int() << ", ";
                break;
            case 1:
                cout << "f_attr=" << t->Get_f_attr() << ", ";
                break;
            case 2:
                cout << "f_float=" << t->Get_f_float();
                break;
            }
        }
        cout << "}" << endl;
    }
};
#endif

// *******************************************************************
// Function: Run_Test5
//
// Purpose:
//   Tests to make sure that a vector of AttributeGroups can be
//   communicated.
//
// Notes:      
//   This is done by setting values of the attributes in the
//   AttributeGroup, writing to a BufferConnection and then trying
//   to reconstruct the attributes in a different AttributeGroup.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 9 10:17:13 PDT 2000
//
// Modifications:
//   
// *******************************************************************

bool
Run_Test5(bool verbose, int *subtest, int *nsubtests)
{
    int              wval;
    Test5            t1, t2;
    BufferConnection buf;

    // We have 1 subtest
    *nsubtests = 1;
    *subtest = 1;

#ifdef VERBOSE
    Test5Observer    *TObserver = new Test5Observer(&t2);
    if(verbose)
    {
        cout << "=================================================" << endl;
        cout << "Running Test 5" << endl;
        cout << "=================================================" << endl;
    }
#endif

    // Create the nested object.
    Test5Data datum1;
    datum1.Set_f_string(std::string("VisIt!"));
    datum1.Set_f_double(1.2345678);

    Test5Data datum2;
    datum2.Set_f_string(std::string("Yoda"));
    datum2.Set_f_double(2.3333333);

    Test5Data datum3;
    datum3.Set_f_string(std::string("Skywalker"));
    datum3.Set_f_double(3.4444444);

    bool retval = true;
    // Try writing the attributes one at a time.
    for(wval = 0; (wval < 5) && retval; ++wval)
    {
        t1.Reset();
        t2.Reset();

        // Set fields based on the number.
        if(wval >= 0)
            t1.Set_f_int(112233);
        if(wval >= 1)
            t1.AddData(datum1);
        if(wval >= 2)
            t1.AddData(datum2);
        if(wval >= 3)
            t1.AddData(datum3);
        if(wval >= 4)
            t1.Set_f_float(3.14159);

        // Write t1 onto buf
        t1.Write(buf);

        // Read t2 from buf and notify TObserver
        t2.Read(buf);

#ifdef VERBOSE
        if(verbose)
            t2.Notify();
#endif
        // Compare the two objects
        retval = (t1 == t2);
    }

#ifdef VERBOSE
    // Destroy the observer.
    delete TObserver;
#endif

    return retval;
}
