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

#ifndef BASIC_NETCDF_FILE_FORMAT_H
#define BASIC_NETCDF_FILE_FORMAT_H
#include <avtSTSDFileFormat.h>
#include <avtMTSDFileFormat.h>
#include <vectortypes.h>
#include <map>

class NETCDFFileObject;
class avtFileFormatInterface;
class avtBasicNETCDFReader;

// ****************************************************************************
//  Class: avtBasic_MTSD_NETCDFFileFormat
//
//  Purpose:
//      Reads in NETCDF files with time dimension as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
//  Modifications:
//
// ****************************************************************************

class avtBasic_MTSD_NETCDFFileFormat : public avtMTSDFileFormat
{
public:
   static bool         Identify(NETCDFFileObject *);
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtBasic_MTSD_NETCDFFileFormat(const char *);
                       avtBasic_MTSD_NETCDFFileFormat(const char *, NETCDFFileObject *);
    virtual           ~avtBasic_MTSD_NETCDFFileFormat();

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "Basic MT NETCDF"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    avtBasicNETCDFReader *reader;
};

// ****************************************************************************
//  Class: avtBasic_STSD_NETCDFFileFormat
//
//  Purpose:
//      Reads in NETCDF files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 10 15:21:14 PST 2005
//
//  Modifications:
//    Brad Whitlock, Wed Apr 26 17:39:32 PST 2006
//    Added a new flag.
//
//    Brad Whitlock, Tue May 16 14:00:05 PST 2006
//    Added override of GetCycleFromFilename.
//
//    Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//    Added args to ReturnValidDimensions and procNum/Count data members
//    to support on-the-fly domain decomp
//
//    Brad Whitlock, Thu Oct 29 16:14:17 PDT 2009
//    I moved the guts to avtBasicNETCDFReader.
//
// ****************************************************************************

class avtBasic_STSD_NETCDFFileFormat : public avtSTSDFileFormat
{
public: 
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtBasic_STSD_NETCDFFileFormat(const char *filename, 
                                                NETCDFFileObject *);
                       avtBasic_STSD_NETCDFFileFormat(const char *filename);
    virtual           ~avtBasic_STSD_NETCDFFileFormat();

    virtual double         GetTime();

    virtual const char    *GetType(void) { return "Basic ST NETCDF"; }
    virtual void           ActivateTimestep(void); 
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);

protected:
    virtual int            GetCycleFromFilename(const char *f) const;
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
private:
    avtBasicNETCDFReader *reader;
};

#endif
