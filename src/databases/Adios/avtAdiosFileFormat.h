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
//                            avtAdiosFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Adios_FILE_FORMAT_H
#define AVT_Adios_FILE_FORMAT_H

#ifdef PARALLEL
#else
#define NOMPI
#endif


#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>
#include <map>

#include <vtkRectilinearGrid.h>

extern "C"
{
#include <adios_read.h>
}


// ****************************************************************************
//  Class: avtAdiosFileFormat
//
//  Purpose:
//      Reads in Adios files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

class avtAdiosFileFormat : public avtMTMDFileFormat
{
  public:
                       avtAdiosFileFormat(const char *);
    virtual            ~avtAdiosFileFormat();

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
    virtual void        GetCycles(std::vector<int> &);
    // virtual void        GetTimes(std::vector<double> &);
    //

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "Adios"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:

    void                   OpenFile();
    void                   CloseFile();
    void                   DoDomainDecomposition();

    std::string            filename;
    bool                   fileOpened;
    
    class meshInfo
    {
      public:
        meshInfo()
        {
            start[0] = start[1] = start[2] = 0;
            count[0] = count[1] = count[2] = 0;
            global[0] = global[1] = global[2] = 0;
            dim = 0;
        }
        ~meshInfo() {}

        int dim, start[3], count[3], global[3];
        std::string name;
    };

    class varInfo : public meshInfo
    {
      public:
        varInfo() : meshInfo() {type=0; groupIdx=0, timeVarying=false;}
        ~varInfo() {}
        
        std::string meshName;
        int groupIdx, type;
        bool timeVarying;
    };

    std::map<std::string, meshInfo> meshes;
    std::map<std::string, varInfo> variables;

    vtkDataArray          *GetADIOSVar(int timestate, const char *varname);
    vtkRectilinearGrid    *CreateUniformGrid(const meshInfo &minfo);
    
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    // Global variables for one opened file
    int64_t         fh;            // file handle
    int             numTimeSteps;  // number of timesteps
    BP_FILE_INFO    finfo;         // file information structure
    BP_GROUP_INFO   *ginfos;       // group information structure
    int64_t         *ghs;          // group handlers
};


#endif
