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
//  File:        avtSTARFileFormat.h                                 //
//  Programmer:  Andrew Foulks <rafoulks@cs.unh.edu>                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef AVT_STAR_FILE_FORMAT_H
#define AVT_STAR_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#define DEBUG_TO_STDERR
#define TRACE(...)

// stardata includes
#include "StarObject.h"

// C++ includes
#include <string>
#include <vector>

// class predeclarations
class DataManagerAPI;
class MultiresGrid;
class ResolutionMap;

/**
 *      The STARFileFormat is a plugin for visit to
 *      read multiresolution scientific data files produced
 *      by a tool called 'stargen'.  'stargen' converts OpenGGCM
 *      files called '3df' files into a binary representation
 *      that includes multiple resolutions.  This plugin
 *      can read the data files with extension 'mrd'.  The
 *      OperatorAttribute plugin called 'MultiresControl',
 *      in a different directory, provides the widget that 
 *      allows the user to change the resolution of the data 
 *      while running VisIt.
 *
 *      @author Andrew Foulks
 **/

class avtSTARFileFormat : public avtMTMDFileFormat, public StarObject
{
public: // 'structors
                       avtSTARFileFormat(const char *);
    virtual           ~avtSTARFileFormat();

public: // this class api
    virtual void            changeResolution(int resolution);
    virtual void            processCommand(std::string command);

public: // inherited from base class
    virtual void            GetCycles(std::vector<int> &);
    virtual void            GetTimes(std::vector<double> &);
    virtual bool            CanCacheVariable(const char *varname);
    virtual bool            HasInvariantMetaData(void) const;

    virtual int             GetNTimesteps(void);
    virtual void            ActivateTimestep(int ts);

    virtual const char*     GetType(void)   { return "STAR"; };
    virtual void            FreeUpResources(void); 

    virtual vtkDataSet*     GetMesh(int, int, const char *);
    virtual vtkDataArray*   GetVar(int, int, const char *);
    virtual vtkDataArray*   GetVectorVar(int, int, const char *);

    virtual void            RegisterDataSelections(
                                     const std::vector<avtDataSelection_p>& sels,
                                     std::vector<bool>* selectionsApplied);

protected: // called by parent class
    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *,int);

private:  // helper functions
    void addDerivedVariables(avtDatabaseMetaData *md, int timeState);

private: // class data members
    int                 mResolution;                // current resolution
    int                 mNumFiles;                  // number of files
    std::string         mCurrentVarName;
    int                 mCurrentTimeState;
    std::string         mFilename;                  // name of metadata file
    DataManagerAPI*     mDataManager;               // multiple or single file
    MultiresGrid*       mMultiresGrid;              // locations of data vals
    ResolutionMap*      mResolutionMap;             // current resolution table
};


#endif
