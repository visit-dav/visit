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
//                            avtTecplotBinaryFileFormat.h                   //
// ************************************************************************* //

#ifndef AVT_TECPLOTBINARY_FILE_FORMAT_H
#define AVT_TECPLOTBINARY_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <string>
#include <vector>
#include <map>

class TecplotFile;
class vtkPoints;

// ****************************************************************************
//  Class: avtTecplotBinaryFileFormat
//
//  Purpose:
//      Reads in TecplotBinary files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Jun 6 15:32:40 PST 2008
//
// ****************************************************************************

class avtTecplotBinaryFileFormat : public avtSTMDFileFormat
{
public:
                       avtTecplotBinaryFileFormat(const char *);
    virtual           ~avtTecplotBinaryFileFormat() {;};

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int domain,
    //                                     const char *type, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the cycle number, overload this function.
    // Otherwise, VisIt will make up a reasonable one for you.
    //
    // virtual int         GetCycle(void);
    virtual double         GetTime();

    virtual const char    *GetType(void)   { return "TecplotBinary"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    static bool MatchFormat(const char *filename);

protected:
    std::map<std::string, std::vector<int> > zoneNameToZoneList;
    std::map<std::string, std::string>       scalarToZone;
    void Initialize(TecplotFile *f,
                    avtDatabaseMetaData *md,
                    std::map<std::string, std::vector<int> > &zoneToZoneIds,
                    std::map<std::string, std::string> &s2z);

    TecplotFile *File();
    vtkPoints   *GetPoints(int zoneId, int ndims = 3);
    vtkDataSet  *GetCurvilinearMesh(int zoneId);
    vtkDataSet  *GetUnstructuredMesh(int zoneId);
    vtkDataSet  *GetPolyMesh(int zoneId);
    vtkDataSet  *GetCurve(int zoneId);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
private:
    TecplotFile *tec;
};


#endif
