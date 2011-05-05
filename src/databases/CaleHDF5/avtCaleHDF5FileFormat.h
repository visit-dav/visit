/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                            avtCaleHDF5FileFormat.h                        //
// ************************************************************************* //

#ifndef AVT_CaleHDF5_FILE_FORMAT_H
#define AVT_CaleHDF5_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>

#include "hdf5.h"

// ****************************************************************************
//  Class: avtCaleHDF5FileFormat
//
//  Purpose:
//      Reads in CaleHDF5 files as a plugin to VisIt.
//
//  Programmer: Rob Managan
//  Creation:   Thu Apr 21 15:29:31 PST 2011
//
// ****************************************************************************

class avtCaleHDF5FileFormat : public avtSTSDFileFormat
{
  public:
                       avtCaleHDF5FileFormat(const char *filename);
    virtual           ~avtCaleHDF5FileFormat() {FreeUpResources();};

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    virtual void      *GetAuxiliaryData(const char *var, const char *type,
                                        void *args, DestructorFunction &);

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    virtual bool      ReturnsValidCycle() const { return true; };
    virtual int       GetCycle(void);
    virtual int       GetCycleFromFilename(const char *f) const;
    virtual bool      ReturnsValidTime() const { return true; };
    virtual double    GetTime(void);

    virtual const char    *GetType(void)   { return "CaleHDF5"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

    static void Identify(const char *filename);
    static hid_t ReadHDF_Entry(hid_t group, const char* name, void* ptr);

  protected:
    // DATA MEMBERS

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    void                   GetUsedMeshLimits (void);
    hid_t                  GetHDF5File();
    int  removezoneghost_index(int in,int zk,int zl, int lp);
    hid_t   hdffile ;

    int kminmesh, kmaxmesh, lminmesh, lmaxmesh, nnallsmesh ;
};


#endif
