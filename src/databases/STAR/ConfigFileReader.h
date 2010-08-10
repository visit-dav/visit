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
//  File:        ConfigFileReader.h                                          //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _CONFIG_FILE_READER_H_
#define _CONFIG_FILE_READER_H_

/**
 *  This class reads in text files and parses it into sections.
 *  Each section has identifiers and values.  The syntax for
 *  the start of a section is
 *
 *      [sectionname]
 *
 *  The syntax for the identifier value pairs is
 *
 *      identifier = value
 *
 *  Both the LHS and the RHS are parsed and stored as strings.
 *  To search the data structure for a identifier and get its 
 *  value, use the findValue() methods.
 *
 *  Comments are lines that begin with the '#' character.  All
 *  other characters following the '#' to the end of line are
 *  ignored during parsing.
 *
 *      # example comment
 */

class ConfigFileReader : public StarObject
{
public:  // inner class (for throwing exceptions)
    class ParseError {};

public:  // 'structors
                        ConfigFileReader  ();
    virtual             ~ConfigFileReader ();

public:  // ConfigFileApi interface
    virtual const char* filename        () {return mFilename.c_str();}
    virtual bool        parseFile       (const char* filename);
    virtual bool        hasSection      (const char* section);
    virtual bool        hasValue        (const char* id,const char* sect=NULL);
    virtual const char* findValue       (const char* id,const char* sect=NULL);
    virtual int         findIntValue    (const char* id,const char* sect=NULL);
    virtual long long   findLongValue   (const char* id,const char* sect=NULL);
    virtual float       findFloatValue  (const char* id,const char* sect=NULL);
    virtual double      findDoubleValue (const char* id,const char* sect=NULL);
    virtual bool        findBooleanValue(const char* id,const char* sect=NULL);
    virtual void        setExceptionPolicy(bool onWarnings, bool onErrors);

public:  // helper methods
    virtual void        asciiDump       (FILE* outfile=stdout);

protected:  // builder methods used by parser
    virtual void        addSection      (string sectionName);
    virtual void        addIdentifier   (string identifier);
    virtual void        addValue        (string value);

private:   // helper function
    void                readFile        (FILE* infile);

protected: // members
    string  mFilename;
    bool    mWaitingForValue;
    class Section { public:
        Section(string name):name(name){}
        string          name;
        vector<string>  identifiers;
        vector<string>  values;
    };
    vector<Section> mSections;

    bool mWarningsThrowExceptions;
    bool mErrorsThrowExceptions;
    int  mCurrentLine;

private: // disable copy constructor and operator=
    ConfigFileReader(ConfigFileReader&);
    ConfigFileReader& operator=(ConfigFileReader&);
};

#endif // _CONFIG_FILE_READER_H_

