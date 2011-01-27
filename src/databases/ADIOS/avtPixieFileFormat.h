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
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Pixie_FILE_FORMAT_H
#define AVT_Pixie_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <vector>
#include <string>
#include <map>

class ADIOSFileObject;
class avtFileFormatInterface;
class vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtPixieFileFormat
//
//  Purpose:
//      Reads in Pixie-ADIOS files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Handle time varying variables and add expressions.
//
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

class avtPixieFileFormat : public avtMTMDFileFormat
{
  public:
    static bool        Identify(ADIOSFileObject *);
    static avtFileFormatInterface *CreateInterface(ADIOSFileObject *f,
                                                   const char *const *list,
                                                   int nList,
                                                   int nBlock);
    avtPixieFileFormat(const char *);
    avtPixieFileFormat(const char *, ADIOSFileObject *);
    virtual  ~avtPixieFileFormat();

    virtual void        GetCycles(std::vector<int> &);
    virtual void        GetTimes(std::vector<double> &);
    virtual int         GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "ADIOS-Pixie"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    ADIOSFileObject *file;
    
    bool             initialized;
    void             Initialize();

    virtual void     PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    
    bool             HasCoordinates(const std::string &var, std::string *coords);
                                  
    struct VarInfo
    {
        std::string fileVarName, mesh;
        bool isTimeVarying;
    };
    typedef std::map<std::string, VarInfo> VarInfoMap;

    struct MeshInfo
    {
        uint64_t    dims[3];
        bool        isRectilinear;
        std::string coords[3];
    };
    typedef std::map<std::string, MeshInfo> MeshInfoMap;

    std::vector<int> timecycles;
    VarInfoMap       variables;
    MeshInfoMap      meshes;
    std::string      rawExpression;

    bool             IsVariable(const std::string &vname);
    std::string      GetVarName(const std::string &vname, bool &isTimeVarying);
    std::string      GetVarMesh(const std::string &vname, MeshInfo &mi);
    bool             GetTimeStep(const std::string &vname, int &ts);
};
#endif
