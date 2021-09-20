/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
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

// ************************************************************************* //
//                            avtWDataFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_WData_FILE_FORMAT_H
#define AVT_WData_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>
#include <complex>

typedef std::complex<float> Complex;

#include "wdata.h"

class WDataVariable
{
public:
    WDataVariable(wdata_metadata *wdmd, int varid, int precdowngrade);
    virtual ~WDataVariable() { ; };

    virtual bool getVariable(const char *_varname, int cycleid, float *data_for_visit) { ; };

    int numberOfVariables() { return varname.size(); };
    std::string getIthVariableName(int ith) { return varname[ith]; };
    std::string getIthVariableUnit(int ith) { return varunit[ith]; };

    virtual bool isScalar() { ; };
    virtual bool isVector() { ; };

protected:
    int loadCycle(int cycleid);

    int vid;            // index of varaiable in md->vars table
    int d2f;            // downgrade double to float
    wdata_metadata *md; // variable info
    int loadedcycle;

    void *data; // each varaiable has data

    std::vector<std::string> varname; // list of var names that can be generated from this variable
    std::vector<std::string> varunit; // corresponding units
};

class WDataRealVariable : public WDataVariable
{
public:
    WDataRealVariable(wdata_metadata *wdmd, int varid, int precdowngrade);
    ~WDataRealVariable() { free(data); };

    bool getVariable(const char *_varname, int cycleid, float *data_for_visit);

    bool isScalar() { return true; };
    bool isVector() { return false; };

protected:
    float *dataR;
};

enum cplxtrans
{
    cabs,
    carg,
    cre,
    cim
};
class WDataComplexVariable : public WDataVariable
{
public:
    WDataComplexVariable(wdata_metadata *wdmd, int varid, int precdowngrade);
    ~WDataComplexVariable() { free(data); };

    bool getVariable(const char *_varname, int cycleid, float *data_for_visit);

    bool isScalar() { return true; };
    bool isVector() { return false; };

protected:
    Complex *dataC;
    std::vector<cplxtrans> trans; // transformations
};

class WDataVectorVariable : public WDataVariable
{
public:
    WDataVectorVariable(wdata_metadata *wdmd, int varid, int precdowngrade);
    ~WDataVectorVariable() { free(data); };

    bool getVariable(const char *_varname, int cycleid, float *data_for_visit);

    bool isScalar() { return false; };
    bool isVector() { return true; };

protected:
    float *dataVx;
    float *dataVy;
    float *dataVz;
};

// ****************************************************************************
//  Class: avtWDataFileFormat
//
//  Purpose:
//      Reads in WData files as a plugin to VisIt.
//
//  Programmer: gabrielw -- generated by xml2avt
//  Creation:   Wed Jul 29 19:18:00 PST 2020
//
// ****************************************************************************

class avtWDataFileFormat : public avtMTSDFileFormat
{
public:
    avtWDataFileFormat(const char *);
    virtual ~avtWDataFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int timestep,
    //                                     const char *type, void *args,
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void GetCycles(std::vector<int> &);
    virtual void GetTimes(std::vector<double> &);

    virtual int GetNTimesteps(void);

    virtual const char *GetType(void) { return "WData"; };
    virtual void FreeUpResources(void);

    virtual vtkDataSet *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

protected:
    // DATA MEMBERS

    wdata_metadata wdmd;

    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    std::vector<WDataVariable *> variable;

    std::string dbcomment;
};

#endif
