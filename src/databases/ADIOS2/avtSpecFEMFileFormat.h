/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_SpecFEM_FILE_FORMAT_H
#define AVT_SpecFEM_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <vector>
#include <string>
#include <map>
#include <adios2.h>

class avtFileFormatInterface;
class vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtSpecFEMFileFormat
//
//  Purpose:
//      Reads in SpecFEM-ADIOS files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
// ****************************************************************************

class avtSpecFEMFileFormat : public avtMTMDFileFormat
{
  public:
    static bool        Identify(const char *fname);
    static avtFileFormatInterface *CreateInterface(const char *const *list,
                                                   int nList,
                                                   int nBlock);
    static bool        GenerateFileNames(const std::string &nm,
                                         std::string &meshNm, std::string &dataNm);
    static bool        IsMeshFile(const std::string &fname);
    static bool        IsDataFile(const std::string &fname);

    avtSpecFEMFileFormat(const char *);
    virtual  ~avtSpecFEMFileFormat();

    virtual void        GetCycles(std::vector<int> &);
    virtual void        GetTimes(std::vector<double> &);
    virtual int         GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "ADIOS-SpecFEM"; };
    virtual void           FreeUpResources(void);

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    std::shared_ptr<adios2::ADIOS> adiosMesh, adiosData;
    adios2::IO meshIO, dataIO;
    adios2::Engine meshReader, dataReader;

    int ngllx, nglly, ngllz, numBlocks;

    std::vector<int> regions;
    bool             initialized;

    void             Initialize();
    virtual void     PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    vtkDataSet *     GetWholeMesh(int ts, int dom, bool xyzMesh);
    vtkDataSet *     GetRegionMesh(int ts, int dom, int region, bool xyzMesh);
    void             AddRegionMesh(int ts, int dom, int region, vtkDataSet *ds,
                                   bool xyzMesh, int ptOffset=0);
    vtkDataSet *     GetHotSpotsMesh(bool xyzMesh);
    vtkDataSet *     GetVolcanoMesh(bool xyzMesh);
    vtkDataSet *     GetContinents(bool xyzMesh);
    vtkDataSet *     GetPlates(bool xyzMesh);
    vtkDataSet *     LatLonClip(vtkDataSet *ds);

    vtkDataArray *   GetVarRegion(std::string &nm, int ts, int dom);
    vtkDataArray *   GetVectorVarRegion(std::string &nm, int ts, int dom);

    std::vector<std::string> variables;
    std::vector<std::pair<std::string, int> > domainVarPaths;
    bool kernelFile;

    int GetRegion(const std::string &str);
    std::string GetVariable(const std::string &str);
    static int NUM_REGIONS;
};
#endif
