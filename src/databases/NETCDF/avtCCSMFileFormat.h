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

// ************************************************************************* //
//                            avtCCSMFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_CCSM_FILE_FORMAT_H
#define AVT_CCSM_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>
#include <avtSTSDFileFormat.h>

#include <vector>

class avtFileFormatInterface;
class NETCDFFileObject;
class avtCCSMReader;

// ****************************************************************************
//  Class: avtCCSM_MTSD_FileFormat
//
//  Purpose:
//      Reads in CCSM files with time dimension as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
//  Modifications:
//    Brad Whitlock, Tue Oct 27 12:14:35 PDT 2009
//    I moved the guts to avtCCSMReader.
//
// ****************************************************************************

class avtCCSM_MTSD_FileFormat : public avtMTSDFileFormat
{
public:
   static bool         Identify(NETCDFFileObject *);
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtCCSM_MTSD_FileFormat(const char *);
                       avtCCSM_MTSD_FileFormat(const char *, NETCDFFileObject *);
    virtual           ~avtCCSM_MTSD_FileFormat();

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "CCSM MT"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    avtCCSMReader *reader;
};

// ****************************************************************************
//  Class: avtCCSM_MTSD_FileFormat
//
//  Purpose:
//      Reads in CCSM files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Oct 27 12:15:09 PDT 2009
//
//  Modifications:
//
// ****************************************************************************

class avtCCSM_STSD_FileFormat : public avtSTSDFileFormat
{
public: 
   static bool         Identify(NETCDFFileObject *);
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtCCSM_STSD_FileFormat(const char *);
                       avtCCSM_STSD_FileFormat(const char *, NETCDFFileObject *);
    virtual           ~avtCCSM_STSD_FileFormat();

    virtual int           GetCycle();
    virtual double        GetTime();
    virtual const char   *GetType(void) { return "CCSM ST"; }

    virtual void          ActivateTimestep(void); 
    virtual void          FreeUpResources(void); 

    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);

protected:
    int                   GetCycleFromFilename(const char *f) const;
    void                  PopulateDatabaseMetaData(avtDatabaseMetaData *);

    avtCCSMReader *reader;
};

#endif
