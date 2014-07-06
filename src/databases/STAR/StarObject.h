/*****************************************************************************
*
* Copyright (c) 2010, University of New Hampshire Computer Science Department
* All rights reserved.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  File:        StarObject.h                                                //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef STAR_OBJECT_H_
#define STAR_OBJECT_H_

//#define TRACE_ON
//#define DEBUG_ON
//#define DEBUG_TO_STDERR

#define TRACE(...)
#define ASSERTIONS

// C includes
#include <signal.h>    // to catch SIGSEGV segmentation faults
#include <cstdio>
#include <cstring>
#include <cstdlib>
#ifdef _GNU_SOURCE
# include <execinfo.h>  // for backtrace and backtrace_symbols
#endif

// C++ includes
#include <typeinfo>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

/* ========================================================================= */
/* ==                                MACROS                               == */
/* ========================================================================= */

/* ===============================  MESSAGES  ============================== */

#ifdef STAR_VISIT
#include "DebugStream.h"
static char _msg_[2048];

// if debug is on, tee the output to both the visit log and the console
#if defined(DEBUG_ON) && defined(DEBUG_TO_STDERR)
#define TEE_MSG(txt) std::cerr << txt << endl
#else
#define TEE_MSG(txt)
#endif

#define INFO(fmt,...) do{\
    snprintf(_msg_, sizeof _msg_,"[INFO:%s] " fmt,__func__,## __VA_ARGS__);\
    debug4 << _msg_ << endl;\
    TEE_MSG(_msg_);\
}while(0)

#define WARNING(fmt,...) do{\
    snprintf(_msg_,sizeof _msg_, "[WARNING:%s] " fmt,__func__,## __VA_ARGS__);\
    debug2 << _msg_ << endl;\
    TEE_MSG(_msg_);\
}while(0)

#define ERROR(fmt,...) do{\
    snprintf(_msg_, sizeof _msg_,"[ERROR:%s] " fmt,__func__,## __VA_ARGS__);\
    debug1 << _msg_ << endl;\
    TEE_MSG(_msg_);\
}while(0)

#if defined(DEBUG_ON)
#define FATAL(fmt,...) do{\
    snprintf(_msg_, sizeof _msg_,"[FATAL ERROR:%s:%s:%d] " fmt,__func__,__FILE__, __LINE__,## __VA_ARGS__);\
    debug1 << _msg_ << endl;\
    TEE_MSG(_msg_);\
    exit(-1);\
}while(0)
#else
#define FATAL(fmt,...) do{\
    snprintf(_msg_, sizeof _msg_,"[FATAL ERROR:%s:%s:%d] " fmt,__func__,__FILE__, __LINE__,## __VA_ARGS__);\
    debug1 << _msg_ << endl;\
    TEE_MSG(_msg_);\
}while(0)
#endif

#else

#define INFO(fmt,...) fprintf(stderr,"[INFO] " fmt "\n",## __VA_ARGS__)
#define WARNING(fmt,...) fprintf(stderr,"[WARNING] " fmt "\n",## __VA_ARGS__)
#define ERROR(fmt,...) fprintf(stderr,"[ERROR] " fmt "\n",## __VA_ARGS__)
#if defined(DEBUG_ON)
#define FATAL(fmt,...) exit(-!!fprintf(stderr,"[FATAL ERROR] " fmt "\n",## __VA_ARGS__))
#else
#define FATAL(fmt,...) fprintf(stderr,"[FATAL ERROR] " fmt "\n",## __VA_ARGS__)
#endif

#endif // STAR_VISIT

/* ================================  DEBUG  ================================ */

#ifdef DEBUG_ON
#ifdef DEBUG_TO_STDERR

#define DEBUG(fmt,...) fprintf(stderr,"[%s] " fmt "\n",__func__,## __VA_ARGS__)

#else
static FILE* _dfp;  // debug file pointer
#define dbgfile _dfp?_dfp:(_dfp=fopen("debug.log","wa"))?_dfp:stderr

#define DEBUG(fmt, ...) fprintf(dbgfile, "[%s, %s:%d] " fmt "\n",\
                                __func__, __FILE__, __LINE__,## __VA_ARGS__)

#endif // DEBUG_TO_STDERR

#define HERE(...) fprintf(stderr,"Here: %s: %d\n", __FILE__, __LINE__)

#else

#define DEBUG(format, ...)
#define HERE(...)

#endif // DEBUG_ON

#ifdef __GNUC__
#define NOTIMPL fprintf(stderr,"%s NOT IMPLEMENTED YET\n",__PRETTY_FUNCTION__)
#else
#define NOTIMPL fprintf(stderr,"%s NOT IMPLEMENTED YET\n",__func__)
#endif

/* ==============================  ASSERTIONS  ============================= */

#ifdef ASSERTIONS

#define REQUIRE(expression, format, ...) \
    if(!(expression)) { \
        fprintf(stderr,"\nASSERTION FAILURE: in %s, line %d, func '%s'\n",\
                             __FILE__, __LINE__, __func__);\
        fprintf(stderr,"Error Message: ");\
        fprintf(stderr, format "\n",## __VA_ARGS__);\
        fprintf(stderr,"\n"); \
        fflush(stderr); \
        abort(); \
    }

// deprecated. this is the old syntax
#define OLDREQUIRE(expression, print_statement) \
    if(!(expression)) { \
        printf("\nASSERTION FAILURE: in %s, line %d",__FILE__, __LINE__);\
        printf("\nError Message: ");\
        printf print_statement; \
        printf("\n"); \
        fflush(stdout); \
        abort(); \
    }

#define REQUIRE_EQUALS(a,b) OLDREQUIRE((a)==(b),(#a "=%s, " #b "=%s",a,b))
#define REQUIRE_INT_EQUALS(a,b) OLDREQUIRE((a)==(b),(#a "=%d, " #b "=%d",(a),(b)))
#define NOTNULL(ptr) OLDREQUIRE((ptr!=NULL),(#ptr " is null!"))

/*  
// Here is the new NOTNULL macro with cleaner syntax.

#define NOTNULL(ptr) REQUIRE((ptr!=NULL), #ptr " is null!")
*/

#define BOUNDS_CHECK(i,min,max) OLDREQUIRE(((i>=min)&&(i<max)),\
            ("out of bounds: %s=%d, valid=[%d...%d]", #i,i,(int)min,(int)max-1))

#define RANGE_CHECK(f,min,max) OLDREQUIRE( ((f>=(float)min)&&(f<=(float)max)),\
            ("out of range: %s=%0.3f, valid=[%0.3f...%0.3f]", #f,f,min,max))

#else

#define REQUIRE(expression, format, ...) /* nothing */
#define OLDREQUIRE(expression, msg) /* nothing */
#define NOTNULL(ptr) /* nothing */
#define BOUNDS_CHECK(i,min,max) /* nothing */
#define RANGE_CHECK(f,min,max) /* nothing */

#endif // ASSERTIONS

/* ==============================  BASE CLASS  ============================= */
/**
 *      STARdata Project Description
 *
 *      The purpose of the following project is to provide the scientist
 *      with a multiresolution and adaptive resolution data model
 *      implementation to aid in visualization of large multidimensional,
 *      multiattribute scientific data sets.
 *
 *      Class StarObject is the base class of all classes in the project.
 *      It includes commonly used functions, macros, assertions, and 
 *      basic debugging facilities.
 **/

class StarObject
{
public:  // constants
public:  // inner classes
    class Error {}; class IOerror {};  // for throwing exceptions

public:  // 'structors
        StarObject()  {}
virtual ~StarObject() {}

public:  // api

/* ========================================================================= */
/**
 *      Returns the name of the class (implemented 
 *      using C++'s bizarre RTTI syntax).  Override in subclass
 *      to get a cleaner name.
 **/

virtual const char* className() 
{ 
    return typeid(this).name(); 
}

/* ========================================================================= */
/**
 *      Reimplement this function in your subclass.  By default
 *      returns the address and the name of the class.
 **/

virtual std::string toString()
{
    char buf[1024];

    sprintf(buf, "[%s@%p]", className(), this);

    return buf;
}

/* ========================================================================= */
/**
 *      For the future when I get to threads.
 **/

virtual void wait(long timeout=-1)
{
    NOTIMPL;
}

/* ========================================================================= */
/**
 *      For the future when I get to threads.
 **/

virtual void notify()
{
    NOTIMPL;
}

/* ========================================================================= */
/**
 *      For the future when I get to threads.
 **/

virtual void notifyAll()
{
    NOTIMPL;
}

/* ========================================================================= */

/* ========================================================================= */
/* ==                                STATIC                               == */
/* ========================================================================= */


/* ========================================================================= */
/**
 *      Splits a string into tokens.  Inspired by java.  
 *
 *      There are 3 versions of this function.  The first is a full
 *      C++ version, using vectors and strings.  The second uses
 *      C-style strings as input, but returns a vector of 
 *      strings.  The third is a full C-style implementation,
 *      that returns an argv[][] style array of strings,
 *      up to the maximum specified.
 *
 *      By default, the delimiters is whitespace.
 *
 *      @param line     string to split
 *      @param delim    string of delimiters
 *      @param tokens   array to hold individual tokens
 *      @param max      maximum number of elements to put into the array
 **/

static std::vector<std::string> split(std::string line, std::string delim=" \t\n")
{
    std::vector<std::string> tokens;

    char* saveptr = NULL;
    char* copy = new char[line.length()+1];

    strcpy(copy, line.c_str());

    char* token = strtok_r(copy, delim.c_str(), &saveptr);

    while(token != NULL) {
        tokens.push_back(token);

        token = strtok_r(NULL, delim.c_str(), &saveptr);
    }

    delete [] copy;

    return tokens;
}

/* ========================================================================= */
/**
 *      For removing comments from lines.  This removes everything 
 *      from the endmark to the newline by replacing the first 
 *      instance of the endmark character with the null byte.  
 *      For example, the string
 *      
 *          "file = /local/tmp/data/filename.tsd  # this is a comment"
 *
 *      Will become
 *
 *          "file = /local/tmp/data/filename.tsd  "
 *
 *      if the commentChar character passed in was '#'.
 **/

static char* removeComment(char* line, int size, char commentChar)
{
    char* ptr = line;

    for(int i=0; i<size; i++, ptr++) {
        if(ptr == NULL) {
            break;
        }
        else if(*ptr == '\0') {
            break;
        }
        else if(*ptr == commentChar) {
            *ptr = '\0';
            break;
        }
    }

    return line;
}

/* ========================================================================= */
/**
 *      Removes, in place, the leading and trailing whitespace 
 *      (by default) from the string passed in.  Optional 
 *      @param delim to specify which characters to remove 
 *      from the beginning or the end of the string.
 **/

static void trim(std::string& line, std::string delim=" \t\n")
{
    // remove from the beginning
    while(line.length()>0 && strchr(delim.c_str(), line[0]) != NULL)
        line.erase(0,1);

    // remove from the end
    int j = line.length()-1;
    while(line.length()>0 && strchr(delim.c_str(), line[j]) != NULL) {
        line.erase(j,1);

        j = line.length()-1;
    }
}

/*=========================================================================*/
/**
 *      Splits a full pathname into a head (directory prefix) 
 *      and a tail (filename.  For example, the input:
 *
 *          /local/data/mhd/jcd0004/test/jcd0004.mrm
 *
 *      will return a vector of string with 2 entries, the head
 *
 *           /local/data/mhd/jcd0004/test/
 *
 *      and the tail (the filename itself)
 *
 *           jcd0004.mrm
 **/

static std::vector<std::string> splitPathName(std::string fullpath)
{
    std::vector<std::string> pathPlusFile;  // the return value
    std::string filename = "";
    std::string pathname = fullpath;

    std::vector<std::string> dirnames = split(fullpath, "/");

    if(dirnames.size()>0) {
        filename = dirnames[dirnames.size()-1];
        pathname.erase(pathname.find(std::string("/")+filename));
    }

    pathPlusFile.push_back(pathname);
    pathPlusFile.push_back(filename);

    return pathPlusFile;
}

/* ========================================================================= */
/**
 *      Splits a filename into a prefix and an extension,
 *      based on the '.' as the separator character.
 *      For example, the input: jcd0004.3df.000060.mrm will 
 *      return a vector two entries, the prefix
 *      'jcd0004.3df.000060' and the extension 'mrm'.
 *
 *      @param filename is the name of the file to be split,
 *             passed by value because the characters are
 *             manipulated in place.
 **/

static std::vector<std::string> splitFileName(std::string filename)
{
    std::vector<std::string> prefixPlusExtension;  // the return value
    std::string extension = "";
    std::string prefix = filename;

    std::vector<std::string> filenameParts = split(filename, ".");

    if(filenameParts.size()>0) {
        extension = filenameParts[filenameParts.size()-1];
        prefix.erase(prefix.find(std::string(".")+extension));
    }

    prefixPlusExtension.push_back(prefix);
    prefixPlusExtension.push_back(extension);

    return prefixPlusExtension;
}

/* ========================================================================= */
/**
 *      Methods to convert string to 64 bit integer (long long),
 *      errors in parsing will return 0 and print a warning message.
 **/

static long long toLong(std::string str)
{
    const char* txt = str.c_str();
    long long value = 0;
    char* endptr = NULL;

    if(txt != NULL) {
        value = strtoll(txt, &endptr, 10);
        
        if(endptr == NULL || endptr[0] != '\0' || txt[0] == '\0') {
            WARNING("Unable to convert '%s' to long long", txt);
            value = 0;
        }
    }

    return value;
}

/* ========================================================================= */
/**
 *      Converts a string or a single character to 
 *      integer, errors in parsing will return 0 
 *      and print a warning message.
 **/

static int toInt(char c)
{
    // need to pass a null terminated string to convert it to an int
    char str[2] = {0};

    str[0] = c;
    str[1] = '\0';

    return toInt(str);
}
static int toInt(std::string str)
{
    const char* txt = str.c_str();
    int value = 0;
    char* endptr = NULL;

    if(txt != NULL) {
        value = strtol(txt, &endptr, 10);
        
        if(endptr == NULL || endptr[0] != '\0' || txt[0] == '\0') {
            WARNING("Unable to convert '%s' to int", txt);
            value = 0;
        }
    }

    return value;
}

static bool isInt(std::string str)
{
    const char* txt = str.c_str();
    bool isInteger = false;
    char* endptr = NULL;

    if(txt != NULL) {
        int res = strtol( txt, &endptr, 10 ); (void) res;
        
        if(endptr == NULL || endptr[0] != '\0' || txt[0] == '\0')
            isInteger = false;
        else
            isInteger = true;
    }

    return isInteger;
}

/* ========================================================================= */
/**
 *      Converts a string to float, errors in parsing
 *      will return 0.0 and print a warning message.
 */

static float toFloat(std::string str)
{
    const char* txt = str.c_str();
    float value = 0.0;
    char* endptr = NULL;

    if(txt != NULL) {
        value = strtod( txt, &endptr );
        
        if(endptr == NULL || endptr[0] != '\0' || txt[0] == '\0') {
            WARNING( "Unable to convert '%s' to float", txt );
            value = 0.0;
        }
    }

    return value;
}

/* ========================================================================= */
/**
 *      Opens a file for READ using a search path.
 *
 *      Returns NULL if all attempts fail.
 *
 *      First, tries to open the file in the current directory.
 *
 *      If that fails, uses the default search path and tries to
 *      open the file in each directory in the path.
 *
 *      If that fails, tries to use the search path defined by 
 *      the environment variable STARPATH to create new filenames 
 *      to try to open.  
 *
 *      Similar to other unix paths, directories are separated 
 *      by a colon.  The STARPATH environment variable can be 
 *      up to 2048 characters in size, and there can be a maximum 
 *      of 64 paths separated by colons in the STARPATH.
 *
 *      Opening a file for writing ("w" or "wb" modes) does 
 *      not make sense when using search paths, so is not supported
 *      by this function call.  If opening the file for "w" or 
 *      "wb" fails beceause of, for example, permissions problems, 
 *      using a searchpath to try other places may end up writing 
 *      the output file to an obscure location that the user has 
 *      trouble finding.
 **/

static FILE* openFile(const char* name, const char* path=NULL)
{
    static const char* mode = "rb";
    static bool firstTime = true;
    static const char* DEFAULT_PATH= ".:..:../..:../metadata:../../metadata:"
                               "../../../metadata:./data:../data:../../data:"
                               "/local:/local/data:/data";
    static char* STARPATH = NULL;
    static char* HOME_DIR = NULL;

    if(firstTime) {
        STARPATH = getenv("STARPATH");
        HOME_DIR = getenv("HOME");

        firstTime = false;
    }

    FILE* fileptr = NULL;

    fileptr = fopen(name, mode);

    if(fileptr == NULL) {
        std::vector<std::string> searchPaths;
        if(path != NULL)         searchPaths.push_back(path);
        if(DEFAULT_PATH != NULL) searchPaths.push_back(DEFAULT_PATH);
        if(HOME_DIR != NULL)     searchPaths.push_back(HOME_DIR);
        if(STARPATH != NULL)     searchPaths.push_back(STARPATH);

        for(unsigned i=0; i<searchPaths.size() && fileptr==NULL; i++) {
            std::vector<std::string> tokens = split(searchPaths[i], ":");

            for(unsigned i=0; i<tokens.size() && fileptr==NULL; i++) {
                std::string filename = tokens[i] + "/" + name;
                fileptr = fopen(filename.c_str(), mode);

                //printf("tried '%s', FILE=%p\n",filename.c_str(),fileptr);
            }
        }
    }

    return fileptr;
}

/* ========================================================================= */

static long long filesize(const char* filename)
{
    FILE* infile = fopen(filename, "rb");

    if(infile == NULL)
        FATAL("Unable to open file '%s' for read (binary)",filename);

    long long size = filesize(infile);

    fclose(infile);

    return size;
}

static long long filesize(FILE* infile)
{
    if(infile == NULL) 
        FATAL("filesize: fatal error: infile is null");

    long long currentOffset = ftell(infile);

    fseek(infile, 0, SEEK_END);

    long long size = ftell(infile);

    fseek(infile, currentOffset, SEEK_SET);

    return size;
}

/* ========================================================================= */

static const char* moduleName()
{
   return "stardata";
}

/* ========================================================================= */

static bool fileExists(const char* filename)
{
    FILE* file = fopen(filename, "r");
    bool file_exists = false;

    if(file != NULL) {
        fclose(file);
        file_exists = true;
    }

    return file_exists;
}

/* ========================================================================= */

static void printStackTrace()
{
#ifdef _GNU_SOURCE
    size_t sz;
    void *bt[20];
    char **strings;
    char command[1024];
    char line[1024];
    const char* tmpfilename = "_backtrace_";  // temp file to process backtrace

    sz = backtrace(bt, 20);
    strings = backtrace_symbols(bt, sz);

    fprintf(stderr,"BACKTRACE:\n");

    sprintf(command, "rm -f %s && touch %s", tmpfilename, tmpfilename);
    system(command);

    for(unsigned i = 0; i < sz; ++i) {
        std::string funcname = "";
        std::vector<std::string> tokens = split(strings[i], " ()");

        if(tokens.size()>=4)             // case 1: mac output format
            funcname = tokens[3];
        else if(tokens.size()>=2)        // case 2: linux output format
            funcname = split(tokens[1],"+")[0];
        else                             // default: print the whole string
            funcname = strings[i];

        snprintf(command, sizeof command, 
                "echo '[%d]' `c++filt -n \"%s\"` >> %s",
                i,funcname.c_str(),tmpfilename);

        system(command);
    }

    FILE* fp = fopen(tmpfilename, "r");
    for(unsigned i = 0; i < sz; ++i) {
        fgets(line, sizeof line, fp);
        fprintf(stderr,"%s", line);
    }

    fflush(fp);
    fclose(fp);
    remove(tmpfilename);
#endif
}

/* ========================================================================= */

static void segFaultHandler(int signal)
{
    if(signal==SIGSEGV)
        fprintf(stderr,"\n\n---SEGMENTATION FAULT---\n");

    if(signal==SIGBUS)
        fprintf(stderr,"\n\n-----  BUS ERROR  ------\n");

    printStackTrace();

#if defined(TRACE_ON) && defined(DEBUG_ON)
    fprintf(stderr,"\nCALL STACK (from TRACE macros):\n");
    Debug::printCallStack();
#endif 
    exit( -1 ); // HOOKS_IGNORE
}

/* ========================================================================= */

static void catchSegmentationFaults(bool catchSegFaults)
{
    if(catchSegFaults) {
        signal(SIGSEGV, &segFaultHandler);
        signal(SIGBUS, &segFaultHandler);
    }
}

/* ========================================================================= */

private: // helper methods

private: // members

private: // disable copy constructor and operator=
        StarObject(StarObject&);
        StarObject& operator=(StarObject&);
};

#endif // STAR_OBJECT_H_

/* ========================================================================= */

