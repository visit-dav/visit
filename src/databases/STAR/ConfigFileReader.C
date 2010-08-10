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
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//  File:        ConfigFileReader.cpp                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>

// stardata includes
#include "StarObject.h"
#include "ConfigFileReader.h"

static const char*  GLOBAL_SECTION_NAME = "$global$";

/* ========================================================================= */
/**
 *      Constructs an empty metadata parser.  @See parseFile() to make
 *      things happen.
 **/

ConfigFileReader::ConfigFileReader()
{
    TRACE();
    DEBUG("entered");

    mFilename = "";
    mWaitingForValue = false;
    mWarningsThrowExceptions = false;
    mErrorsThrowExceptions = false;
    mCurrentLine = 0;

    // define default section 0 for any expressions not in a subsection
    Section s(GLOBAL_SECTION_NAME);

    mSections.push_back(s);

    DEBUG("exited");
}

/* ========================================================================= */
/**
 *      Parses the file specified and stores the information in the
 *      table.  Each section begins with a declaration in square 
 *      brackets containing the section name:
 *
 *      [section]
 *
 *      Each section contains any number of entries containing an 
 *      identifier and a value:
 *
 *      identifier = value
 *
 *      We use lex to process the tokens from the file.  
 *      We only distinguish between section headers, identifiers, 
 *      and values.  All values are stored as strings in the data 
 *      structures.
 *
 *      @return true    if the file was parsed successfully
 *              false   if unable to open file
 **/

bool ConfigFileReader::parseFile(const char* filename)
{
    TRACE(filename);
    DEBUG("filename=%s",filename);

    bool status = false;

    //FILE* infile = fopen( filename, "r" );
    FILE* infile = openFile(filename);  // searches for the file if not found

    if(infile != NULL) {
        info("Reading config file '%s'...", filename);

        readFile( infile );

        fclose( infile );
        status = true;

        mFilename = filename;
    }
    else {
        error("Unable to open config file '%s' for read", filename);
        error("(Try setting the environment variable STARPATH)");
        status = false;

        if(mErrorsThrowExceptions)
            throw ParseError();
    }

    return status;
}

/* ========================================================================= */
/**
 *      Helper function to read in the config file and put
 *      the info into data structures.
 **/

void ConfigFileReader::readFile(FILE* infile)
{
    mCurrentLine = 0;

    while(!feof(infile)) {
        char line[1024];

        fgets(line, sizeof(line), infile);

        if(feof(infile))
            break;

        mCurrentLine++;

        removeComment(line, sizeof(line), '#');

        vector<string> tokens = split(line, "=");

        switch(tokens.size()) {
            case 0: break;                            // skip empty line
            case 1: addSection(tokens[0]); break;
            case 2:
                addIdentifier(tokens[0]);
                addValue(tokens[1]);
                break;

            default:
                warning("IGNORING token '%s' and others on line %d",
                        tokens[2].c_str(), mCurrentLine);

                if(mWarningsThrowExceptions)
                    throw ParseError();
        }
    }
}

/* ========================================================================= */
/**
 *  Returns true if the section exists in the config file.
 */

bool ConfigFileReader::hasSection(const char* section)
{
    TRACE(section);
    //DEBUG("section=%s",section);

    if(section == NULL)
        return true;

    bool foundit = false;

    for(unsigned int i=0; i<mSections.size(); i++) {
        if(mSections[i].name == section) {
            foundit = true;
            break;
        }
    }

    return foundit;
}

/* ========================================================================= */
/**
 *      Returns true if the identifier is in the section specified, 
 *      otherwise returns false.
 */

bool ConfigFileReader::hasValue(const char* id, const char* section)
{
    TRACE(id,section);
    NOTNULL(id);
    //DEBUG("id=%s, section=%s",id,section);

    if(section == NULL)
        section = GLOBAL_SECTION_NAME;

    bool foundit = false;

    for(unsigned int i=0; i<mSections.size() && !foundit; i++) {
        if(mSections[i].name == section) {
            int numVariables = mSections[i].identifiers.size();

            for(int j=0; j<numVariables; j++) {
                if(mSections[i].identifiers[j] == id) {
                    foundit = true;
                    break;
                }
            }
        }
    }

    return foundit;
}

/* ========================================================================= */
/**
 *      Searches the file for identifier in a section specified 
 *      by "sectionName" and returns the value as an unmodifiable 
 *      c-string.  If either the section or the variable are not 
 *      found, the null string ("") is returned, and a warning 
 *      message is printed.
 *
 *          [sectionname]
 *          .
 *          .
 *          .
 *          id = value
 *          .
 *          .
 *          .
 **/

const char* ConfigFileReader::findValue(const char* id, const char* section)
{
    TRACE(id,section,warnIfNotFound);
    NOTNULL(id);
    //DEBUG("id=%s, section=%s",id,section);

    if(section == NULL)
        section = GLOBAL_SECTION_NAME;

    const char* value = "";
    int sectionNumber = 0;
    int idNumber = 0;
    bool foundit = false;

    for(unsigned int i=0; i<mSections.size() && !foundit; i++) {
        if(mSections[i].name == section) {
            sectionNumber = i;

            for(unsigned int j=0; j<mSections[i].identifiers.size(); j++) {
                if(mSections[i].identifiers[j] == id) {
                    idNumber = j;
                    foundit = true;
                    break;
                }
            }
        }
    }

    if(foundit) {
        value = mSections.at(sectionNumber).values.at(idNumber).c_str();
    }
    else {
        warning("findValue: '%s' NOT FOUND in section '%s'",id,section);

        if(mWarningsThrowExceptions)
            throw ParseError();
    }

    return value;
}

/* ========================================================================= */
/**
 *  Searches for the identifier, but tries to convert its value
 *  to an integer before returning it to the caller.
 */

int ConfigFileReader::findIntValue(const char* id, const char* section)
{
    TRACE(id,section);

    char* endptr = NULL;
    int   value  = 0;

    value = strtol(findValue(id,section), &endptr, 10);

    if(endptr != NULL and endptr[0] != '\0') {
        warning("Parse Error: Unable to convert string '%s' "
                "to an integer\n", findValue(id,section));

        if(mWarningsThrowExceptions)
            throw ParseError();
    }

    return value;
}

/* ========================================================================= */
/**
 *  Searches for the identifier, but tries to convert its value
 *  to a 64 bit integer before returning it to the caller.
 */

long long ConfigFileReader::findLongValue(const char* id, const char* section)
{
    TRACE(id,section);
    char* endptr = NULL;
    long long value  = 0;

    value = strtoll(findValue(id,section), &endptr, 10);

    if(endptr != NULL and endptr[0] != '\0') {
        warning("Parse Error: Unable to convert string '%s' "
                "to a long long\n", findValue(id,section));

        if(mWarningsThrowExceptions)
            throw ParseError();
    }

    return value;
}

/* ========================================================================= */
/**
 *  Searches for the identifier, but tries to convert its value
 *  to a float first.
 */

float ConfigFileReader::findFloatValue(const char* id, const char* section)
{
    TRACE(id,section);
    char* endptr = NULL;
    float value  = 0.0;

    value = strtof(findValue(id,section), &endptr);

    if(endptr != NULL and endptr[0] != '\0') {
        warning("Parse Error: Unable to convert string '%s' "
                "to a float\n", findValue(id,section));

        if(mWarningsThrowExceptions)
            throw ParseError();
    }

    return value;
}

/* ========================================================================= */
/**
 *  Searches for the identifier, but tries to convert its value
 *  to a double first.
 */

double ConfigFileReader::findDoubleValue(const char* id, const char* section)
{
    TRACE(id,section);
    char* endptr = NULL;
    float value  = 0.0;

    value = strtod(findValue(id,section), &endptr);

    if(endptr != NULL and endptr[0] != '\0') {
        warning("Parse Error: Unable to convert string '%s' "
                "to a double\n", findValue(id,section));

        if(mWarningsThrowExceptions)
            throw ParseError();
    }

    return value;
}

/* ========================================================================= */
/**
 *  Searches for the identifier, but tries to convert its value
 *  to an integer first.
 */

bool ConfigFileReader::findBooleanValue(const char* id, const char* section)
{
    TRACE(id,section);

    return strcmp(findValue(id,section), "true") == 0;
}

/* ========================================================================= */
/**
 *      Flags whether or not to thow an exception when a warning 
 *      or error message is issued from the parser.  By default,
 *      no exceptions are thrown when warnings or error messages
 *      are printed.  The exception thrown is of type 
 *      'ConfigFileReader::ParseError' -- note that it this is
 *      NOT a standard C++ library exception.
 **/

void ConfigFileReader::setExceptionPolicy(bool onWarnings, bool onErrors)
{
    mWarningsThrowExceptions = onWarnings;
    mErrorsThrowExceptions = onErrors;
}

/* ========================================================================= */
/**
 *  For debugging.  Prints out the data structure so you can see
 *  how well it read in the original file.
 */

void ConfigFileReader::asciiDump(FILE* outfile)
{
    TRACE(outfile);

    for(unsigned int i=0; i<mSections.size(); i++) {
        if(mSections[i].name != "") {
            fprintf(outfile,"[%s]\n", mSections[i].name.c_str());

            for(unsigned int j=0; j<mSections[i].identifiers.size(); j++) {
                if(mSections[i].identifiers[j] != "") {
                    fprintf(outfile,"'%s' = '%s'\n",
                                    mSections[i].identifiers[j].c_str(), 
                                    mSections[i].values[j].c_str());
                }
            }

            fprintf(outfile,"\n");
        }
    }
}

/* ========================================================================= */
/**
 *      Adds a new section to the data structures.  All subsequent
 *      calls to addIdentifier() and addValue() will be stored in this
 *      section.  This method expects section names to be between
 *      square brackets, for example, [sectionname].
 **/

void ConfigFileReader::addSection(string sectionName)
{
    TRACE(sectionName);

    if(!mWaitingForValue) {
        // remove whitespace
        trim(sectionName);

        if(sectionName.length() > 0) {
            if(sectionName[0]=='[' && sectionName[sectionName.length()-1]==']'){
                trim(sectionName, "[] \t\n");

                Section s(sectionName);

                mSections.push_back(s);
            }
            else {
                warning("PARSE ERROR reading section on line %d", mCurrentLine);

                if(mWarningsThrowExceptions)
                    throw ParseError();
            }
        }
        else {
            ; // skip over empty lines, they can be ignored
        }
    }
    else {
        warning("PARSE ERROR: while adding new section '%s', "
                "in previous section='%s', id='%s' has no value\n",
                sectionName.c_str(), mSections.back().name.c_str(),
                mSections.back().identifiers.back().c_str());

        addValue("");

        if(mWarningsThrowExceptions)
            throw ParseError();
    }
}

/* ========================================================================= */
/**
 *      Adds an identifier (i.e. a variable name) to the current section.
 *      Id's take the form 
 *          id = value
 *      in the config file.
 */

void ConfigFileReader::addIdentifier(string identifier)
{
    TRACE(identifier);
    
    if(!mWaitingForValue) {
        // remove whitespace
        trim(identifier);

        mSections.back().identifiers.push_back(identifier);
        mWaitingForValue = true;
    }
    else {
        warning("PARSE ERROR: in section '%s': IGNORING "
                "identifier '%s', previous id='%s' has no value\n",
                mSections.back().name.c_str(),identifier.c_str(),
                mSections.back().identifiers.back().c_str());

        if(mWarningsThrowExceptions)
            throw ParseError();
    }
}

/* ========================================================================= */
/**
 *      Adds the right hand side of an assignment to the current
 *      section, paired with the most recently added identifier 
 *      (left hand side).  Stores the value as a string.
 *      Removes leading whitespace or quotes before storing the
 *      value, but does not remove internal whitespace or quotes.
 *
 *      Example:
 *          [filenames]
 *          files = "foo bar"
 *
 *      stores 'foo bar' (without the quotes) in the lookup
 *      table, under the section 'filenames'
 **/

void ConfigFileReader::addValue(string rhs)
{
    TRACE(rhs);

    if(mWaitingForValue) {
        trim(rhs, " \t\n\"");

        mSections.back().values.push_back(rhs);
    }
    else {
        warning("PARSE ERROR: in section '%s': IGNORING value "
                "'%s', which has no identifier\n",
                mSections.back().name.c_str(), rhs.c_str());

        if(mWarningsThrowExceptions)
            throw ParseError();
    }

    mWaitingForValue = false;
}

/* ========================================================================= */
 
ConfigFileReader::~ConfigFileReader()
{
    TRACE();
}

/* ========================================================================= */

#ifdef STAND_ALONE_CONFIG_FILE_READER

//  g++ -g -I.. -DSTAND_ALONE_CONFIG_FILE_READER ConfigFileReader.cpp

int main()
{
const char* filename = "/local/data/mhd/jcd0004/test/multires/jcd0004.mrm";
//const char* filename = "/local/data/mhd/jcd0004/multires/jcd0004.mrd";

    ConfigFileReader parser;

    parser.parseFile(filename);
    parser.asciiDump();
}

#endif

/* ========================================================================= */

#ifdef USE_LEX

//  LEX Tokens 
enum Token 
{ 
    END, BOOLEAN, SECTION, IDENTIFIER, FLOAT, INT, STRING,
    EQUALS, COMMENT, NEWLINE, UNKNOWN, START
};

//  LEX Prototypes
int yylex(void);

#define YY_NO_UNPUT     // gets rid of compiler error

void ConfigFileReader::readFile(FILE* infile)
{
    extern FILE* yyin;                      // LEX file variable
    extern char *yytext;                    // LEX token string

    yyin = infile;

    int token = START;

    while(token != END) {
        DEBUG("before call to yylex()");
        token = yylex();
        DEBUG("after call to yylex()");

        switch(token) {
            case SECTION:    addSection(yytext);     break;
            case IDENTIFIER: addIdentifier(yytext);  break;
            case FLOAT:      addValue(yytext);       break;
            case INT:        addValue(yytext);       break;
            case BOOLEAN:    addValue(yytext);       break;
            case STRING:     addValue(yytext);       break;
            case UNKNOWN:    addValue(yytext);       break;
            case COMMENT:    /* ignore */              break;
            case NEWLINE:    /* ignore */              break;
            case END:        /* ignore */              break;
            default:         warning("IGNORING bad token '%s'",yytext);
        }
    }
}

#endif // USE_LEX

/* ========================================================================= */
