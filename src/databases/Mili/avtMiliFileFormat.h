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
//                             avtMiliFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MILI_FILE_FORMAT_H
#define AVT_MILI_FILE_FORMAT_H

#include <map>
#include <vector>
#include <string>
#include <visitstream.h>

extern "C" {
#include <mili.h>
}

#include <avtMTMDFileFormat.h>
#include <avtTypes.h>

class avtMaterial;
class vtkDataArray;
class vtkUnstructuredGrid;
class vtkFloatArray;

using std::vector;
using std::map;

// ****************************************************************************
//  Class: avtMiliFileFormat
//
//  Purpose:
//      A file format reader for Mili.
//
//  Notes:       Much of the code was taken from Doug Speck's GRIZ reader.
//      
//  Programmer:  Hank Childs
//  Creation:    April  11, 2003
//
//  Modifications:
//    Akira Haddox, Fri May 23 08:30:01 PDT 2003
//    Added in support for multiple meshes within a Mili database.
//    Changed into a MTMD file format.
//
//    Akira Haddox, Tue Jul 22 09:21:39 PDT 2003
//    Added meshId argument to ConstructMaterials.
//    Added reading in of times. Added FreeUpResources.
//    Changed sub_records to hold mili type 'Subrecord'.
//
//    Akira Haddox, Fri Jul 25 11:09:13 PDT 2003
//    Added var_dimension.
//
//    Akira Haddox, Mon Aug 18 14:31:55 PDT 2003
//    Added dyna partition support for ghostzones.
//
//    Hank Childs, Mon Oct 20 10:07:00 PDT 2003
//    Added GetTimes and times data member.
//
//    Hank Childs, Tue Jul 20 14:47:31 PDT 2004
//    Added an array for the variable type.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to PopulateDatabaseMetaData to satisfy new interface
//
//    Mark C. Miller, Mon Jul 18 13:41:13 PDT 2005
//    Added CanCacheVariable since we handle caching of "param arrays" here
//    in the plugin and added data members to handle free nodes mesh
//
//    Mark C. Miller, Wed Jan  4 16:51:34 PST 2006
//    Added IssueWarning private method and warn_map data member
// ****************************************************************************

class avtMiliFileFormat : public avtMTMDFileFormat
{
  public:
                          avtMiliFileFormat(const char *);
    virtual              ~avtMiliFileFormat();
    
    virtual const char   *GetType(void) { return "Mili File Format"; };
    
    virtual void          GetCycles(vector<int> &);
    virtual void          GetTimes(vector<double> &);
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);
    virtual vtkDataArray *GetVectorVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    virtual void         *GetAuxiliaryData(const char *var, int, int,
                                           const char *type, void *args,
                                           DestructorFunction &);

    virtual void          FreeUpResources(void);

    virtual bool          CanCacheVariable(const char *varname);

  protected:
    char *famroot;
    char *fampath;
    
    vector<Famid>         dbid;
    int                   ntimesteps;
    int                   ndomains;
    int                   nmeshes;
    bool                  setTimesteps;
    vector<double>        times;

    map<int, int>         warn_map;
    vector<bool>          validateVars;
    vector<bool>          readMesh;
    int                   dims;

    int                                   *free_nodes;
    int                                    free_nodes_ts;
    int                                    num_free_nodes;

    vector<vector<int> >                   nnodes;
    vector<vector<int> >                   ncells;
    vector<vector<vtkUnstructuredGrid *> > connectivity;

    vector<vector< Subrecord > >           sub_records;
    vector<vector< int > >                 sub_record_ids;

    vector<vector< std::string > >         element_group_name;
    vector<vector< int > >                 connectivity_offset;
    vector<vector< int > >                 group_mesh_associations;

    vector< std::string >                  known_param_arrays;
    vector< std::string >                  vars;
    vector< avtCentering >                 centering;
    vector< vector< vector<bool> > >       vars_valid;
    vector< vector< vector<int> > >        var_size;
    vector< avtVarType >                   vartype;
    vector< int >                          var_dimension;
    vector< int >                          var_mesh_associations;

    vector<int>                            nmaterials;
    vector<vector< avtMaterial * > >       materials;

    void                  IssueWarning(const char *msg, int key);
    void                  ReadMesh(int dom);
    void                  ValidateVariables(int dom);
    avtMaterial *         ConstructMaterials(vector< vector<int*> >&,
                                            vector< vector<int> > &, int);
    int                   GetVariableIndex(const char *);
    int                   GetVariableIndex(const char *, int mesh_id);
    void                  GetSizeInfoForGroup(const char *, int &, int &, int);

    vtkFloatArray        *RestrictVarToFreeNodes(vtkFloatArray *, int ts) const;

    void                  DecodeMultiMeshVarname(const std::string &, 
                                                 std::string &, int &);

    inline void           OpenDB(int dom);

    void                  ParseDynaPart();

    bool                  readPartInfo;
    std::string           dynaPartFilename;
};


#endif


