/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef ADIOS_FILE_OBJECT_H
#define ADIOS_FILE_OBJECT_H
#include <string>
#include <vector>
#include <map>
#include <visitstream.h>

//NOTE: #include <mpi.h> *MUST* become before the adios includes.
#ifdef PARALLEL
#include <mpi.h>
#else
#define _NOMPI
#endif

extern "C"
{
#include <adios_read.h>
}

template <class T> static inline void
SwapIndices(int dim, T *arr)
{
    if (dim <= 1) return;
    else if (dim == 2)
    {
        T i0 = arr[0], i1 = arr[1];
        arr[0] = i1; arr[1] = i0;
    }
    else if (dim == 3)
    {
        T i0 = arr[0], i2 = arr[2];
        arr[0] = i2; arr[2] = i0;
    }               
}

class ADIOSVar;
class vtkDataArray;

// ****************************************************************************
//  Class: ADIOSFileObject
//
//  Purpose:
//      Wrapper around an ADIOS file.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 16:15:32 EST 2010
//
// ****************************************************************************

class ADIOSFileObject
{
  public:
    ADIOSFileObject(const char *fname);
    virtual ~ADIOSFileObject();

    bool Open();
    void Close();
    bool IsOpen() const {return fp != NULL;}
    int NumTimeSteps();
    void GetCycles(std::vector<int> &cycles);

    bool ReadVariable(const char *varname,
                      int ts,
                      vtkDataArray **array);
    
    std::map<std::string, ADIOSVar> variables;

  protected:
    std::string fileName;

    ADIOS_FILE *fp;
    ADIOS_GROUP **gps;
};


// ****************************************************************************
//  Class: ADIOSVar
//
//  Purpose:
//      Wrapper around ADIOS variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 16:15:32 EST 2010
//
// ****************************************************************************

class ADIOSVar
{
  public:
    ADIOSVar()
    {
        start[0] = start[1] = start[2] = 0;
        count[0] = count[1] = count[2] = 0;
        global[0] = global[1] = global[2] = 0;
        dim = 0;
        type=-1; groupIdx=-1, varid=-1, timedim=-1;
    }
    ~ADIOSVar() {}
        
    int dim, groupIdx, type, varid, timedim;
    int start[3], count[3], global[3];
    std::string name;
        
    void SwapIndices()
    {
        ::SwapIndices(dim, start);
        ::SwapIndices(dim, count);
        ::SwapIndices(dim, global);
    }
};

#endif
