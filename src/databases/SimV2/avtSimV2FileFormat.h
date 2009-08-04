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
//                            avtSimV2FileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_SIMV2_FILE_FORMAT_H
#define AVT_SIMV2_FILE_FORMAT_H

#include <database_exports.h>

#include <avtSTMDFileFormat.h>
#include <avtSimulationInformation.h>
#include <avtMaterial.h>

#include <vector>
#include <set>
#include <string>

class avtSpecies;

// ****************************************************************************
//  Class: avtSimV2FileFormat
//
//  Purpose:
//      Reads in a .sim2 file for VisIt in the MDServer.
//      Reads in simulation data as input to VisIt in the Engine.
//
//  Notes: This reader is based on SimV1 but is extended to support additional
//         data in the SimV2 structures.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Feb  5 11:59:19 PST 2009
//
//  Modifications:
//
// ****************************************************************************

class avtSimV2FileFormat : public avtSTMDFileFormat
{
  public:
                       avtSimV2FileFormat(const char *);
    virtual           ~avtSimV2FileFormat() {;};

    virtual const char    *GetType(void)   { return "SimV2"; };
    virtual void           FreeUpResources(void); 
    virtual int            GetCycle() { return -1; }

    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);
    virtual avtMaterial   *GetMaterial(int, const char *);
    virtual vtkDataSet    *GetCurve(const char *);

    virtual void          *GetAuxiliaryData(const char *var, int domain,
                                            const char *type, void *,
                                            DestructorFunction &df);

    virtual void           PopulateIOInformation(avtIOInformation& ioInfo);

  protected:
    avtSpecies            *GetSpecies(int, const char *);

    avtSimulationInformation simInfo;
    std::set<std::string>    curveMeshes;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
};


#endif
