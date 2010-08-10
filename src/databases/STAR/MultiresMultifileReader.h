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
//  File:        MultiresMultifileReader.h                                   //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _MULTIRES_MULTIFILE_READER_H_
#define _MULTIRES_MULTIFILE_READER_H_

#include <string>
using std::string;

#define DEBUG_TO_STDERR

#include "DataManagerAPI.h"

// class predeclarations
class ConfigFileReader;
class MultiresFileReader;

/**
 *      The MultiresMultifileReader provides an implementation
 *      to a dataset described by many @see MultiresFileReader.
 *      Typically, this is for a multiple attribute dataset,
 *      in which each attribute is in a separate subdirectory
 *      with separate files for each attribute / timestep.
 *
 *      @author Andrew Foulks
 **/

class MultiresMultifileReader : public DataManagerAPI
{
public:  // constructors
            MultiresMultifileReader();
            MultiresMultifileReader(const char* filename);
    virtual ~MultiresMultifileReader();

public:  // api 
    virtual const char*     filename        () const {return mFilename.c_str();}
    virtual float           fileVersion     () const {return mFileVersion;}
    virtual int             numFiles        () const;
    virtual int             numResolutions  () const;
    virtual string          gridFilename    () const;
    virtual vector<int>     timeStepList    () const;

    // data rank: scalars, vectors, tensors
    virtual int             numVariables        () const;
    virtual string          variableNameAt      (int index) const;
    virtual bool            isScalar            (const string& name) const;
    virtual char            isVectorComponent   (const string& name) const;
    virtual bool            isVector            (const string& name) const;
    virtual bool            isTensor            (const string& name) const;
    virtual int             numVectorExpressions() const;
    virtual string          vectorExpressionAt  (int index) const;

    // data size and structure
    virtual int             width               (int resolution) const;
    virtual int             height              (int resolution) const;
    virtual int             depth               (int resolution) const;
    virtual int             numXchunks          (int resolution=-1) const;
    virtual int             numYchunks          (int resolution=-1) const;
    virtual int             numZchunks          (int resolution=-1) const;
    virtual bool            hasMinVal           () const {return false;}
    virtual bool            hasMaxVal           () const {return false;}
    virtual float           minVal              () const {return 0.0;}
    virtual float           maxVal              () const {return 0.0;}

    // data access
    virtual const float*    rawData             (const string& varName,
                                                 int resolution,
                                                 int fileIndex,
                                                 int chunkIndex=0);
public:
    virtual void            parseFile           (const char* filename);

private: // helper functions
    int                     indexOfVariableName (const string& name) const;
    string                  getFilename         (const string& variableName,
                                                 int fileIndex);
    MultiresFileReader*     findInCache         (const string& variableName,
                                                 int fileIndex);
    void                    addToCache          (MultiresFileReader* mf);
    float                   parseFileVersion    (const char* filename);
    void                    freeRawDataMemory   (const string& variableName,
                                                 int fileIndex);

// test functionality
public: static int TEST(int argc, char** argv);

private: // inner data structures
    struct Variable { 
        Variable(string name, int rank, char component) :
            name(name), rank(rank), component(component) {}
        string name;
        int rank;        // 0-scalar, 1-vector, 2-tensor
        char component;  // 'x','y', or 'z', if part of a vector
    };

private: // members
    string                  mFilename;      // name of 'mrm' file.
    float                   mFileVersion;   // version of the 'mrm' file read
    ConfigFileReader*       mFileReader;    // class to parse text config file
    vector<Variable>        mVariables;     // all variables and components
    vector<string>          mVectorExpressions;// vectors made up of components
    vector<                                 // keep track of all data files
      vector<MultiresFileReader*> > mCache; // indexing: [varIndex][fileIndex]

private: // disable copy constructor and operator=
    MultiresMultifileReader(MultiresMultifileReader&);
    MultiresMultifileReader& operator=(MultiresMultifileReader&);
};

#endif // _MULTIRES_MULTIFILE_READER_H_

