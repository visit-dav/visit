#ifndef VISIT_STREAM_H
#define VISIT_STREAM_H

#if defined(_WIN32) && defined(USING_MSVC6)
// We're on Windows using the Microsoft VC++ 6.0 compiler. We need to 
// include the .h versions of iostream and fstream.

#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>

#else
// Include iostream and some using statements.
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::ios;
using std::ostream;
using std::streambuf;

// Include fstream and some using statements.
#include <fstream>

using std::ifstream;
using std::istream;
using std::ofstream;
using std::streampos;

// Include strstream and some using statements.
#include <strstream>

using std::ostrstream;
#endif

#endif
