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
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_ADIOS_BASIC_FILE_FORMAT_H
#define AVT_ADIOS_BASIC_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <vector>
#include <string>
#include <map>

class ADIOSFileObject;
class avtFileFormatInterface;
class vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtADIOSBasicFileFormat
//
//  Purpose:
//      Reads in ADIOS files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
// ****************************************************************************

class avtADIOSBasicFileFormat : public avtMTMDFileFormat
{
  public:
    static bool        Identify(ADIOSFileObject *);
    static avtFileFormatInterface *CreateInterface(ADIOSFileObject *f,
                                                   const char *const *list,
                                                   int nList,
                                                   int nBlock);
    avtADIOSBasicFileFormat(const char *);
    avtADIOSBasicFileFormat(const char *, ADIOSFileObject *);
    virtual  ~avtADIOSBasicFileFormat();

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

    virtual const char    *GetType(void)   { return "ADIOS"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    ADIOSFileObject *fileObj;
    bool             initialized;


    void                   Initialize();
    std::string            GenerateMeshName(const ADIOSVar &v);
    void                   DoDomainDecomposition();

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

        int dim;
        int start[3], count[3], global[3];
        std::string name;

        void SwapIndices()
        {
            ::SwapIndices(dim, start);
            ::SwapIndices(dim, count);
            ::SwapIndices(dim, global);
        }
    };

    std::map<std::string, meshInfo> meshes;

    vtkRectilinearGrid    *CreateUniformGrid(const int *start,
                                             const int *count);
    
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    static void ComputeStartCount(int *globalDims,
                                  int dim,
                                  int *start,
                                  int *count);
};

#endif
