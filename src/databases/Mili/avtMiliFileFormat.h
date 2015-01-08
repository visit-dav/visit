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
//                             avtMiliFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MILI_FILE_FORMAT_H
#define AVT_MILI_FILE_FORMAT_H

#include <list>
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
    
    virtual void          GetCycles(std::vector<int> &);
    virtual void          GetTimes(std::vector<double> &);
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
    char filepath[512];

    std::vector<Famid>    dbid;
    int                   ntimesteps;
    int                   ndomains;
    int                   nmeshes;
    bool                  setTimesteps;
    std::vector<double>   times;

    std::map<int, int>    warn_map;
    std::vector<bool>     validateVars;
    std::vector<bool>     readMesh;
    int                   dims;

    // Free node variables
    int                                   *free_nodes;
    int                                    free_nodes_ts;
    int                                    num_free_nodes;

    // Particle node variables
    int                                   *part_nodes;
    int                                    part_nodes_ts;
    int                                    num_part_nodes;

    std::vector<std::vector<int> >          nnodes;
    std::vector<std::vector<int> >          ncells;
    int                                    qty_nodes, qty_cells;

    std::vector<std::vector<vtkUnstructuredGrid *> > connectivity;

    std::vector<std::vector<Subrecord> >    sub_records;
    std::vector<std::vector<int> >          sub_record_ids;
    std::map<std::string, State_variable>  svars;
    std::map<std::string, State_variable>::iterator svars_iter;

    std::vector<std::vector<std::string> >  element_group_name;
    std::vector<std::vector<int> >          connectivity_offset;
    std::vector<std::vector<int> >          group_mesh_associations;

    std::vector<std::string>               known_param_arrays;
    std::vector<std::string>               vars;
    std::vector<std::string>               vars_dir;
    std::vector<std::string>               descr;       
    std::vector<avtCentering>              centering;
    std::vector<std::vector<std::vector<bool> > > vars_valid;
    std::vector<std::vector<std::vector<int> > > var_size;
    std::vector<avtVarType>                vartype;
    std::vector<int>                       var_dimension;
    std::vector<int>                       var_mesh_associations;

    std::vector<int>                       nmaterials;
    std::vector<std::vector<avtMaterial *> > materials;

    //************************************************
    // Added May 17, 2010: I. R. Corey
    //       New data structures to support GrizIt.
    //************************************************

    typedef struct __mili_zones_type {
            int qty, index;
            int *labels;
            int conn_count;
            int *conns;
            int *mats;
            int *parts;
            int block_qty, *block_list;
            int *ids;
    } mili_zones_type;

    typedef struct __mili_elem_class_type {
            std::string short_name, long_name;
            int sclass;
            int qty;
            mili_zones_type *zone_data;
            int *ids;
            bool mlClass;
            bool addedToMetadata;
    } mili_elem_class_type;

    typedef struct __mili_nodes_type {
            int qty;
            int *labels;
    } mili_nodes_type;

    typedef struct __mili_node_class_type {
            std::string short_name, long_name;
            int qty;
            mili_nodes_type *node_data;            
    } mili_node_class_type;

    typedef struct __mili_primal_type {
            std::vector<std::string> class_names;
            int dataType, vecLen;
            bool node_centered;
            std::vector<std::string> comp_names;
    } mili_primal_type; 

    typedef struct __mili_ti_type {
            std::string fieldName;
            int dataType;
            int dataLen;
    } mili_ti_type;

    bool ti_data_found;
    bool labelsFound;
    int  numML;
    bool labelsAddedToMetadata;
    
    std::vector<std::string>                mili_classes;
    std::vector<std::string>                mili_mlclasses;

    std::map<std::string, mili_primal_type >  mili_primal_fields;  
    std::map<std::string, mili_primal_type >::iterator mili_primal_fields_iter; 
 
    std::map<std::string, std::list<std::string> > mili_primal_components;
    std::map<std::string, std::list<std::string> > mili_primal_classes;

    std::map<std::string, std::list<std::string> >::iterator mili_primal_components_iter;
    std::map<std::string, std::list<std::string> >::iterator mili_primal_classes_iter;

    std::map<std::string, mili_elem_class_type>  mili_elem_class;
    std::map<std::string, mili_node_class_type>  mili_node_class;
    std::map<std::string, int>  mili_elem_class_globalElemId;
 
    std::map<std::string, mili_elem_class_type> ::iterator mili_elem_class_iter; 
    std::map<std::string, mili_node_class_type> ::iterator mili_node_class_iter; 

    std::map<std::string, mili_ti_type> mili_ti_fields;

    //************************************************
    // Added May 17, 2010: I. R. Corey
    //       New data structures to support GrizIt.
    //************************************************

    //************************************************

    void                  IssueWarning(const char *msg, int key);
    void                  ReadMesh(int dom);
    void                  ValidateVariables(int dom);
    avtMaterial *         ConstructMaterials(std::vector<std::vector<int*> >&,
                                             std::vector<std::vector<int> >&,
                                             int);
    int                   GetVariableIndex(const char *);
    int                   GetVariableIndex(const char *, int mesh_id);
    void                  GetSizeInfoForGroup(const char *, int &, int &, int);

    vtkFloatArray        *RestrictVarToFreeNodes(vtkFloatArray *, int ts) const;

    void                  DecodeMultiMeshVarname(const std::string &, 
                                                 std::string &, int &);

    void                  DecodeMultiLevelVarname(const std::string &, std::string &);
    bool                  isVecVar(const std::string &, std::string &);
    inline void           OpenDB(int dom);

    void                  ParseDynaPart();
    void                  LoadMiliInfo(const char *fname);
    void                  LoadMiliInfoTest(const char *fname);
    char                 *ReadMiliFileLine(ifstream &in,
                              const char *commentSymbol, const char *kw,
                              int lineN, bool *lineReturned, bool *eof);

    bool                  readPartInfo;
    std::string           dynaPartFilename;

    // The following functions are new to support GrizIt
    inline void           PopulateMiliClassData(int, int);
    inline void           PopulateMiliVarData(int, int);
    inline void           PopulatePrimalMetaData(int dom, avtDatabaseMetaData *md);
    inline void           PopulateClassMetaData( int dom, avtDatabaseMetaData *md);

    vtkFloatArray        *GetClassDomains(const char *elemClass);
    vtkFloatArray        *GetNodeLabels();
    vtkFloatArray        *GetNodeIds();
    vtkFloatArray        *GetClassLabels(const char *elemClass);
    vtkFloatArray        *GetClassElemIds(const char *elemClass);
    vtkFloatArray        *GetClassMats(const char *elemClass);
    vtkFloatArray        *GetClassTypes(const char *elemClass);

    vtkFloatArray        *GetMiliResult(int ts, const char *var_name);
    vtkFloatArray        *GetMiliVar(int ts, const char *var_name);
    void                  CreateCompIndex(const char *comp_name, int comp_len,
                                          int comp_index,
                                          char *comp_index_string);

    std::list<std::string> GetClassesForVar(char *varName);
    void                  AddClassSubsets(char *meshname, avtDatabaseMetaData *md);
    vtkFloatArray        *GetClassSubsets();

};


#endif


