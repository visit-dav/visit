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
//                          avtExodusFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_EXODUS_FILE_FORMAT_H
#define AVT_EXODUS_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>
#include <avtTypes.h>

#include <map>
#include <string>
#include <vector>


class     avtVariableCache;
class     DBOptionsAttributes;

// ****************************************************************************
//  Class: avtExodusFileFormat
//
//  Purpose:
//      An Exodus file format reader.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Sat Apr 17 07:42:40 PDT 2004
//    Added support for times.
//
//    Hank Childs, Thu Jul 22 11:29:31 PDT 2004
//    Make materials go throw the standard generic database path.  Also add
//    support for registering file lists.
//
//    Mark C. Miller, Mon Aug  9 19:12:24 PDT 2004
//    Added a ReadMesh mehtod
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Removed GetCycles method since all it did was the same thing the
//    default implementation did. Added timeState arg to
//    PopulateDatabaseMetaData
//
//    Eric Brugger, Fri Mar  9 14:42:04 PST 2007
//    Added support for element block names.
//
//    Mark C. Miller, Mon Sep 16 15:04:12 PDT 2013
//    Removed SetTimestep method.
//
//    Mark C. Miller, Thu Dec 18 12:57:59 PST 2014
//    Added database options. Added SetZeroIndexInstance,
//    CopyFromZeroIndexInstance.
// ****************************************************************************

class avtExodusFileFormat : public avtMTSDFileFormat
{
  public:
                                avtExodusFileFormat(const char *, DBOptionsAttributes *rdatts);
    virtual                    ~avtExodusFileFormat();
 
    virtual void                FreeUpResources(void);
    const char                 *GetType(void) { return "Exodus File Format"; };

    virtual void                GetTimes(std::vector<double> &);
    virtual int                 GetNTimesteps(void);
 
    virtual vtkDataSet         *GetMesh(int, const char *);
    virtual vtkDataArray       *GetVar(int, const char *);
    virtual vtkDataArray       *GetVectorVar(int, const char *);

    virtual void                PopulateDatabaseMetaData(avtDatabaseMetaData*, int);

    virtual void               *GetAuxiliaryData(const char *var, int, 
                                    const char *type, void *args,
                                    DestructorFunction &);

    static int                  RegisterFileList(const char *const *, int);
    void                        SetFileList(int fl) { fileList = fl; };

    void                        SetZeroIndexInstance(avtExodusFileFormat *zidx) { zidxInstance = zidx; };
    void                        CopyFromZeroIndexInstance()
                                {
                                    if (this == zidxInstance) return;
                                    numBlocks = zidxInstance->numBlocks;
                                    matCount = zidxInstance->matCount;
                                    matConvention = zidxInstance->matConvention;
                                };


  private:
    int                         GetFileHandle();
    void                        GetTimesteps(int *ntimes, std::vector<double> *times);
    void                        AddVar(avtDatabaseMetaData *md, char const *vname,
                                    int topo_dim, int ncomps, avtCentering centering);
    vtkDataArray*               GetEBDecompAsEnumScalar();

    int                         numBlocks;
    int                         numNodes; // this 'domain'
    int                         numElems; // this 'domain'
    std::vector<int>            blockId;
    std::vector<std::string>    blockName;
    int                         fileList;
    avtExodusFileFormat        *zidxInstance;
    int                         ncExIIId;
    std::map<int, int>          blockIdToMatMap;
    bool                        autoDetectCompoundVars;
    int                         matConvention;
    int                         matCount;
    std::string                 matVolFracNamescheme;
    std::string                 matVarSpecNamescheme;

    // Note: this needs to be a pointer because there are issues with 
    // constructors being called in shared libraries for static objects.
    static std::vector< std::vector<std::string> > *globalFileLists;

};


#endif

