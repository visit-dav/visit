/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
//                            avtVTK_STSDFileFormat.h                        //
// ************************************************************************* //

#ifndef AVT_VTK_FILE_FORMAT_H
#define AVT_VTK_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>
#include <avtVTKFileReader.h>

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtVTK_STSDFileFormat
//
//  Purpose:
//      Reads in VTK_STSD files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 22 16:11:00 PST 2012
//
// ****************************************************************************

class avtVTK_STSDFileFormat : public avtSTSDFileFormat
{
public:
                       avtVTK_STSDFileFormat(const char *filename, 
                                             DBOptionsAttributes *);
                       avtVTK_STSDFileFormat(const char *filename, 
                                             DBOptionsAttributes *,
                                             avtVTKFileReader *r);
    virtual           ~avtVTK_STSDFileFormat();

    virtual const char    *GetType(void);
    virtual void           FreeUpResources(void); 

    int                    GetCycleFromFilename(const char *f) const;
    virtual int            GetCycle(void);
    virtual double         GetTime(void);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

    virtual void          *GetAuxiliaryData(const char *var, 
                              const char *type, void *, DestructorFunction &df);

protected:
    avtVTKFileReader *reader;
};

#include <avtSTMDFileFormat.h>

// ****************************************************************************
//  Class: avtVTK_STMDFileFormat
//
//  Purpose:
//      Reads in VTK_STMD files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 22 16:11:00 PST 2012
//
// ****************************************************************************

class avtVTK_STMDFileFormat : public avtSTMDFileFormat
{
public:
                       avtVTK_STMDFileFormat(const char *filename,
                                             DBOptionsAttributes *);
                       avtVTK_STMDFileFormat(const char *filename, 
                                             DBOptionsAttributes *,
                                             avtVTKFileReader *r);
    virtual           ~avtVTK_STMDFileFormat();

    virtual const char    *GetType(void);
    virtual void           FreeUpResources(void); 

    int                    GetCycleFromFilename(const char *f) const;
    virtual int            GetCycle(void);
    virtual double         GetTime(void);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual vtkDataSet    *GetMesh(int domain, const char *);
    virtual vtkDataArray  *GetVar(int domain, const char *);
    virtual vtkDataArray  *GetVectorVar(int domain, const char *);

    virtual void          *GetAuxiliaryData(const char *var, int domain,
                              const char *type, void *, DestructorFunction &df);

protected:
    avtVTKFileReader *reader;
};


#endif
