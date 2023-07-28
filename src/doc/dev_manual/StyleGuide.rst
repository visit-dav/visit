Coding Style Guide
==================

Naming Conventions
------------------

* Class names start with capital letters, (eg Mesh). If a class name has multiple words, capitalize the first letter of each word, (eg RectilinearMesh). Exceptions can be made to help group classes in a package (eg vtkRectilinearMesh).
* Fields and variables have the first letter uncapitalized and the first letter for each subsequent word capitalized, (eg theImportantValue).
* Methods and functions follow the same naming convention as classes. The distinction between methods and classes will be clear because methods will always have attached parentheses. This is not true when a function is passed as a pointer, but that should be clear from context.
* Try to avoid using the following names as they are reserved by ANSI for future expansion:

+-----------------------------+------------------------------------+
|           Names             |         Description                |
+-----------------------------+------------------------------------+
| E[0-9A-Z][0-9A-Za-z]*       | Errno values                       |
+-----------------------------+------------------------------------+
| is[a-z][0-9A-Za-z]*         | Character classification           |
+-----------------------------+------------------------------------+
| to[a-z][0-9A-Za-z]*         | Character manipulation             |
+-----------------------------+------------------------------------+
| LC_[0-9A-Za-z\_]*           | Locale                             |
+-----------------------------+------------------------------------+
| SIG[_A-Z][0-9A-Za-z\_]*     | Signals                            |
+-----------------------------+------------------------------------+
| str[a-z][0-9A-Za-z\_]*      | String manipulation                |
+-----------------------------+------------------------------------+
| mem[a-z][0-9A-Za-z\_]*      | Memory manipulation                |
+-----------------------------+------------------------------------+
| wcs[a-z][0-9A-Za-z\_]*      | Wide character string manipulation |
+-----------------------------+------------------------------------+

File Structure
--------------

File Names
~~~~~~~~~~

A file containing the definition for class Foo will be named Foo.h. A
file containing the methods for the class Foo will be named Foo.C.

.h File Contents
~~~~~~~~~~~~~~~~

Each of the .h files will have the following format:

+-------------------------+
| #ifndef/#define         |
+-------------------------+
| Includes                |
+-------------------------+
| Class Description       |
+-------------------------+
| Class Definition        |
+-------------------------+
| Variable Declarations   |
+-------------------------+
| Inline Functions        |
+-------------------------+
| #endif                  |
+-------------------------+

* The ifndef is used to prevent the class from being defined multiple times. 

  * To prevent name collisions, the symbolic name being defined should be the class name in all capital letters, with each word separated by underscores. \_H should be appended.

* Forward declare classes rather than including their header file when possible but be sure to use the correct struct or class keyword in the forward declaration.
* Only one class should be defined per file. Exceptions can be made for very closely related classes.
* Inline functions should only be included in the .h file if they are public or protected. Private \* inline functions should be placed in the .C file for that class. 

  * Note that public and protected inline functions should be used sparingly. All code that includes the header must be recompiled if the function is changed.

* The .h file should be valid as a stand alone file. 

  * If other header files are included before this files inclusion, it may be making use of their definitions.

* All variables declared here should be externed. Class-scoped static variables should not be defined here.
* Avoid ``using::`` directives in header files. They will effect not only the current header file but any files in which the header file is included, directly or indirectly.

.C File Content
~~~~~~~~~~~~~~~

Each of the .C files will have the following format:

+-----------------------------+
| Includes                    |
+-----------------------------+
| Variable Declarations       |
+-----------------------------+
| Static Function Prototypes  |
+-----------------------------+
| Constructors                |
+-----------------------------+
| Destructors                 |
+-----------------------------+
| Method Definitions          |
+-----------------------------+
| Friend Functions            |
+-----------------------------+
| Static Functions            |
+-----------------------------+

The friend operators included in the .C file must be directly related to
the class whose methods are defined in that file.

Copyright notice
""""""""""""""""

The copyright notice shall appear at the top of each .C, .h, .java,
CMakeLists.txt, and Python sources. ::

    // Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
    // Project developers.  See the top-level LICENSE file for dates and other
    // details.  No copyright assignment is required to contribute to VisIt.

Includes
~~~~~~~~

* Include files should use angle brackes. For example: #include <vtkRectilinearGrid.h>
* Class.C should include Class.h first. This is to make sure that Class.h is not using any previously declared headers. Class.C file may use quotes instead of angle brackets to include Class.h.
* Include files should be grouped from wider scope to narrower scope. This leads to grouping the include files in the following order:

  * System include files. Examples are: <math.h>, <stdio.h>.
  * X and Qt include files. Examples are: <Xlib.h>, <qgl.h>.
  * Library include files. Examples are: <dmf.h>, <vtk.h>.
  * Class definition files. Examples are: <Mesh.h>, <Field.h>.
  * Within a group, include files should be listed alphabetically.

* If the include files must be listed in a specific order, which is not alphabetic, then a comment must be added justifying it.
* Some C header files contain C++ keywords that cause compilations to fail. With the exception of header files for the standard C library, X, and Motif, all C header files must be wrapped with an *extern C* directive.  ::

    extern "C" {
    #include <hdf5.h>
    }

Forbidden Constructs
~~~~~~~~~~~~~~~~~~~~

exit() and abort()
""""""""""""""""""

Please do not use *exit()* or *abort()* in your code since we do not
want VisIt_ to fail unexpectedly. Use exceptions instead. VisIt_'s check-in
hooks will not permit unconditionally compiled code calling exit or
abort to be checked-in.

using statements
""""""""""""""""

*using* statements of any kind are not permitted in header files since
they can indirectly cause compilation problems for other compilation units
that may include your header file either directly or indirectly. In header
files, you will have to use the fully qualified class name for any class
you need to refer to. (e.g. *std::vector* and not just *vector*).
Yes, this does make for somewhat uglier header files but it also prevents
a lot of problems. VisIt_'s check-in hooks will not permit code containing
*using* statements in a header file.

In source files, when you need to use *using* statements, we prefer that
you narrow the scope of the statement as much as is practical. So, please
don't use *using namespace std* to use something like *std::string*. Instead
use *using std::string*.

Class Description
-----------------

Each class must have a brief description, like the below example. ::

    // ********************************************************
    // Class: Example
    //
    // Purpose:
    //   What this class does.
    //
    // Notes:     Any special notes for users of the class.
    //
    // Programmer: Joe Smith
    // Creation: August 29, 2007
    //
    // Modifications:
    //   Joe Smith, Fri Oct 15 13:31:51 EST 2007
    //   I added a new method to do ...
    //
    // ********************************************************

It is important to use these category names because they will be picked up
by doxygen to create our documentation. The asterisks should fill out the
line, 76 asterisks in all. The category labels (Class, Purpose, etc) should
be indented two spaces past the comment (//). When the text after a category
label wraps to the following line, it should be indented fours spaces after
the comment.

Class Definition
----------------

Class definitions should follow these rules:

* There should be no public fields. They violate the basic object-oriented philosophy of data hiding.
* The sections inside the class should be ordered public, protected, private. This way users of the class can stop reading when they reach protected/private. The fields in each section should be grouped together, as should the methods.
* All inheritance should be public to avoid confusion, bar good reason.
* Every non-trivial field should have a comment preceding it that describes its purpose. This comment will be picked up by doxygen when the documentation is built.
* Friends should be avoided when possible. When it is necessary to grant friend access to a series of derived types, grant it only to their base type and define protected methods for the base type that access the class.
* Define a copy constructor and assignment operator for every class.
* Constructor and destructor method definitions should never appear in the class header file because of compiler bugs on some platforms.
* Note that C++ automatically provides a constructor, a copy constructor, an assignment operator, two address-of operators, and a destructor for you: ::

    // You write
    class Empty { };
    // You get
    class Empty
    {
    public:
        Empty() {};
        ~Empty() {};
        Empty(const Empty &);
        Empty &operator=(const Empty &);
        Empty *operator&();
        const Empty *operator&() const;
    };

* If you are redefining a pure virtual method that should not be used, declare it private and have it throw an exception.
* The copy constructor and assignment operator provided by the compiler perform blind copies, meaning that pointers will also be copied, potentially introducing many bugs.

This is because many publicly available libraries, such as STL, use these
methods.

Method Structure
----------------

The structure of a method should follow this format:

+----------------+
| Prologue       |
+----------------+
| Declaration    |
+----------------+
| Body           |
+----------------+

Prologue
~~~~~~~~ 

Each method must have a prologue with the following format: ::

    // ********************************************************
    // Method: ClassName::MethodName
    //
    // Purpose: 
    //   What this method does.
    //
    // Arguments:
    //   arg1 : What the first argument does...
    //   arg2 : What the second argument does...
    //   ...
    //
    // Returns:  <0 on failure, 0 on success.
    //
    // Note: Assumes coordinates have already been read.
    //
    // Programmer: Joe VisIt
    // Creation: August 29, 2007
    //
    // Modifications:
    //   Joe VisIt, Fri Oct 15 13:31:51 EST 2007
    //   Fixed bug with ...
    //
    // ********************************************************

The category label '''Method''' can be replaced '''Function''', '''Operator''', '''Constructor''', or '''Destructor''' and still be accepted by doxygenate.

Definition
----------

The definition should follow this form: ::

    Zone * 
    RectilinearMesh::GetZone(int i, int j, int k)

If multiple lines are needed for all of the arguments, each subsequent line
should be indented to the opening parenthesis, or if that is too far, 4 spaces.

Body
----

Size
~~~~

The body should be small. Try to keep functions under 100 lines. This
promotes clarity and correctness. This tradeoff should not be paid for
a substantial speed penalty, however.

Arguments
~~~~~~~~~

* All input arguments passed by reference should be declared *const*.
* Unused arguments should not be named. Note that this eliminates the need for the lint directive ARGSUSED.

Variables
~~~~~~~~~

* Variables should be declared near their first use.
* Variable names may not coincide with any of the class' field names.
* All local pointer variables should be set to NULL or 0 when declared. This helps with later tracking of memory problems when looking at core files. 
* Associate *\** and *&* with the variable, not with the type.

For example, the following code: ::

    int* i, j;

misleads the reader into thinking both i and j are pointers to ints,
while j is actually only an int.

* Only use variables declared in the initializer list of a for loop inside that for loop.

The code fragments: ::

   for (int i = 0 ; i < size; i++)
      { ... }
   for (int i = 0 ; i < length ; i++)
      { ... }

and ::

   for (int i = 0 ; i < size ; i++)
      { ... }
   if (i == size)
      ...

Comments
~~~~~~~~

* Avoid using C-style comments. This way, when debugging, they can be used to comment out long blocks of code without worrying about nested comments.
* Indent comments to the same level as the statement to which they apply.
* Both block and single line comments are acceptable, but when modifying a pre-existing file, they should follow its convention.
* Comments are highly encouraged!

Control Structures
~~~~~~~~~~~~~~~~~~

* Use *for (;;)* instead of *while(1)*. They both result in infinite loops, but *while(1)* is flagged by many compilers as a constant condition. This eliminates the need for the lint directive CONSTCOND in this case.
* Any case of a switch statement that does not end with a break should have a FALLTHRU comment to show that this is intentional.
* When the body of a for or while is empty, place a continue in it to make the intent clear.

The following code: ::

    for (int i = 0; p[i] != '\0' ; i++);

Is more clearly represented as: ::

    for (int i = 0; p[i] != '\0' ; i++)
        continue;

Also note that this eliminates the need for the lint directive EMPTY in
this case.

Whitespace
~~~~~~~~~~

* TAB characters are **NOT ALLOWED** in VisIt_ source code.
* Semicolons should immediately follow the last character. (i.e. there is no space between the last character in a statement and its semicolon).
* Lines should not exceed 79 characters in length. Note that it is not necessary to violate this rule for strings. ::

    char *str1 = "Hello world";
    char *str2 = "Hello "
                 "world";

In the code above, str1 is equal to str2.

* All variable declarations should occur on separate lines unless closely related (e.g. int i, j, k;).
* Do not use any tabs in the source. Use *\\t* to simulate a tab in a string.
* The parenthesis of a function should immediately follow the function name. This makes searching easier for functions with common names.
* There should not be any spaces surrounding the *.* or *->*, operators and no spaces preceding a *[* operator.
* An indentation block is four spaces.
* The labels case, public, protected, and private are indented 0 or two spaces.
* Any time a new block is started, a *{* should be put on the following line at the same indentation level. The next statement should be indented an additional four spaces.
* Within reason, adding whitespace to line up parentheses or brackets on consecutive lines is encouraged, even when it violates one of the previous rules.

Reformatting
""""""""""""

Automatic source code reformatting may be performed using a program called
''[http://astyle.sourceforge.net/ artistic style]''. Here is some basic
usage that reformats a source file into a form compatible with VisIt_
coding style: ::

    astyle --brackets=break < inputfile > outputfile

End of line
~~~~~~~~~~~

The UNIX convention for end of line characters must be followed for VisIt_
source code.

Preprocessor
~~~~~~~~~~~~

* Macros should only be used if the # or ## operators are used.
* Any macros used to define a constant should be declared as a const global variable. 
* Parameterized macros used to perform a short routine should be implemented as an inline.
* Macros should only be used if the # or ## operators are used.
* The code inside the #ifdef section should be indented as if the #ifdef were not present.
* Comments should not be added on the same line after preprocessor directives because some compilers do not accept them.
* Preprocessor directives should have the *#* in column 1.

Pointer vs. References
~~~~~~~~~~~~~~~~~~~~~~

References are preferred over pointers.

References:

* Always refer to a real object.
* Do not change objects they refer to.

Pointers:

* Can represent no object (NULL).
* Can change the object they refer to.
* Can represent an array.
* Can represent a location (like the end of an array).

Caveats for ensuring that VisIt_ builds on Windows
--------------------------------------------------

The rules that have been covered before in this document apply mainly
to source code style and are conventions to simplify maintenance. This
section describes some source code constructs that must be avoided at
all times in order to ensure compatibility with the Microsoft Windows
Visual C++ (MSVC). Windows is an important development platform for
VisIt_. Adhering to these additional coding rules will reduce the
amount of time required to fix minor source code problems that burden
Windows developers.

API macros
~~~~~~~~~~

VisIt_'s header files have API macros that help the MSVC (all versions)
compiler and linker produce dynamic link libraries (DLLs) and their
associated import libraries. A DLL is a file containing executable code
which is loaded by an application at runtime and all applications that
require the code stored in the DLL use the same instance of the DLL in
the computer's memory, which saves resources. An import library is a
small stub library that contains enough symbolic information to satisfy
the linker so that all unresolved symbols are resolved at link time and
still allowing the application code to be loaded dynamically at runtime.
This link step is mostly avoided on other platforms where VisIt_'s
libraries are linked exclusively at runtime.

Import libraries are difficult to create manually due to the amount of
symbols in all of VisIt_'s libraries so the VisIt_ source code has been
augmented with API macros that allow the compiler to automatically
create the import libraries. VisIt_'s API macros come from an API
include file and there is one API include file per VisIt_ library.
The name of the API include file is usually the name of the library
appended with the "_exports.h" suffix. The API macro is added to class
declarations when the class should be made accessible to other VisIt_
libraries. ::

    #ifndef MY_EXAMPLE_CLASS_H
    #define MY_EXAMPLE_CLASS_H
    #include <example_exports.h>

    class EXAMPLE_API MyExampleClass
    {
    public:
        MyExampleClass();
        virtual ~MyExampleClass();
    };

    void EXAMPLE_API example_exported_function();
    void this_function_not_exported();
    #endif

In the above example, the header file that gets included defines the
EXAMPLE_API macro, which tells the MSVC compiler to add the flagged
symbols to its list of symbols for the import library that goes along
with the DLL that contains the class. The EXAMPLE_API macro evaluates
to whitespace on other platforms so its inclusion in VisIt_'s source
code is not disruptive. Note that the EXAMPLE_API macro has been applied
to a class and to a function to ensure that both the class's methods
and the function are both added to the import library. Any class,
function, variable, etc that lacks an export macro is not added to the
import library and will not be available to other programs or libraries.

Now that the mechanism by which symbols are added to import libaries
has been explained, suppose that you move a class from one library to
another. What happens? Well, the answer is that the class will be
compiled into the new library but it will not be put into the import
library because its API macro was not changed. To avoid this problem,
it is very important that when you move classes from one library to
another library that you change the class so it uses the appropriate
API macro for the new host library. This goes especially for VTK
classes that have become part of one of VisIt_'s libraries.


No constructor or destructor definitions in header file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Do not put class constructor or destructor definitions in the class
header file. When you put class constructors and destructors in the
class header file, MSVC gets confused when you attempt to use the class
from another DLL because sometimes the virtual method table is messed
up when the constructor and destructor are placed in the header file
possibly due to function inlining. When this happens, it is impossible
to successfully link against the library that is supposed to contain
your class. To be safe, always create a .C file that contains the
constructor and destructors for your class. ::

    #ifndef MY_CLASS_H
    #define MY_CLASS_H
    #include <mylib_exports.h>
    class MYLIB_API MyClass
    {
    public:
        // Never do this
        MyClass() { };
        virtual ~MyClass() { };
    };

Do this instead:

MyClass.h file contents: ::

    #ifndef MY_CLASS_H
    #define MY_CLASS_H
    #include <mylib_exports.h>
    class MYLIB_API MyClass
    {
    public:
        MyClass();
        virtual ~MyClass();
    };

MyClass.C file contents: ::

    #include <MyClass.h>
    MyClass::MyClass() { }
    MyClass::~MyClass() { }

Do not use 'sprintf'
~~~~~~~~~~~~~~~~~~~~

VisIt_ source code should not use *sprintf* into a static sized buffer
due to the possibility of buffer overruns, which introduce memory
problems and possible security threats. To combat this, the use of
*sprintf* is deprecated and all new code should use *snprintf*, which
behaves the same but also takes the size of the buffer as an argument
so buffer overruns are not possible.

Do not use variables called near or far
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The MSVC compiler reserves the *near* and *far* keywords for backward
compatibility with older 16-bit versions of the compiler that used
*near* and *far* to determine pointer size. Do not use near or far for
variable names because it will cause a strange compiler error.

Do not create a file called parser.h
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Windows provides a file called parser.h and if you also provide such
a file, you had better change the include directory order or you will
run into hundreds of errors when the compiler uses Microsoft's parser.h
instead of yours.

Do not create functions or methods called GetMessage
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The WIN32 API is used in certain places in VisIt_ to implement
Windows-specific functionality. Occasionally, we have run into problems
where VisIt_ classes have names such as *GetMessage*. The windows.h
include file defines a macro called GetMessage and sets it to GetMessageEx.
This caused the preprocessor to replace all GetMessage method calls on
a VisIt_ object with GetMessageEx, which is not a method of the object.
Needless to say, this is a confusing compilation problem. Steer clear of
defining method names that conflict with WIN32 macro names!

Comparing QString and std::string
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Call the .toStdString() method to compare QString to std::string.

Example: ::

    QString string1("my q string");
    std::string string2("my std string");

    // Do this:
    if (string1.toStdString() == string2))

Do not use unistd.h
~~~~~~~~~~~~~~~~~~~

Windows does not have the unistd.h header file so do not use functions
from it without making conditionally compiled code. ::

    #if defined(_WIN32)
        // Windows implementation ...
    #else
        #include <unistd.h>
        // Unix implementation ...
    #endif

Do not use libgen.h
~~~~~~~~~~~~~~~~~~~

Windows does not have libgen.h, which is sometimes used for functions
such as dirname(), basename(). Refrain from using functions from libgen
or provide a Windows implementation as well.

Sign of size() method return value
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The .size() method for STL containers returns a *size_t*. Be aware if you
attempt to do arithmetic on the value returned by .size()

Example: ::

    // Consider what happens when the following code is
    // executed with myvector being empty (size is zero)
  
    if (val > myvector.size()-1) // if test fails
    {
       return;
    }
    myvector[val] =  ... // SEGV!

Allocate dynamic arrays on the heap, not the stack
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the size of an array cannot be determined at compile-time, then it cannot be allocated on the stack, but must be allocated on the heap.

Example: ::

    const int nPoints = dataset->GetNumberOfPoints();

    // Since value of nPoints can only be determined at run-time,

    // this will not compile with Visual Studio
    int myarray[nPoints];

    // this will compile
    int *myarray2 = new int[nPoints];

CMake Conventions
-----------------

Starting with VisIt_ version 3.4, new more modern CMake conventions will be adopted, and `BLT <https://llnl-blt.readthedocs.io/en/develop/index.html>`_ will be used whenever feasible.

Handling subdirectories
~~~~~~~~~~~~~~~~~~~~~~~

Each subdirectory should have its own CMakeLists.txt which either creates a new target or adds sources to a target defined in a parent directory's CMakeLists.txt.
If a given target has source files spread out across multiple subdirectories, the `add_library` or `add_executable` calls should be in the CMakeLists.txt of the topmost directory, along with the `add_subdirectory` and any common `target_include_directories` or `target_link_libraries` calls.
The subdirectory will add its sources to the parent's target via `target_sources`.


Here's an example from src/avt/DBAtts and src/avt/DBAtts/SIL:

.. literalinclude:: ../../avt/DBAtts/CMakeLists.txt
    :language: CMake
    :start-at: add_library(avtdbatts)

.. literalinclude:: ../../avt/DBAtts/SIL/CMakeLists.txt
    :language: CMake
    :start-at: target_sources(avtdbatts PRIVATE

