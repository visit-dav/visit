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
//                             avtMiliFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MILI_FILE_FORMAT_H
#define AVT_MILI_FILE_FORMAT_H

#include <list>
#include <map>
#include <vector>
#include <string>
#include <visitstream.h>

#include <Expression.h>

extern "C" {
#include <mili.h>
}

#include <avtMTMDFileFormat.h>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

class avtMaterial;
class vtkDataArray;
class vtkUnstructuredGrid;
class vtkFloatArray;

class MiliVariableMetaData;
class MiliClassMetaData;
class avtMiliMetaData;

using std::vector;
using std::string;

// ****************************************************************************
//  Class: avtMiliFileFormat
//
//  Purpose:
//      A file format reader for Mili.
//
//  Notes:  This filter was largely re-written in Jan 2019 in order to better 
//          handle scalability, extensibility, and the in-coming requests from
//          mili users. Methods that retain significant portions of the original
//          code contain notes about such in their docstrings. The original 
//          filter was first created by Hank Childs on April 11, 2003. 
//
//  Programmer:  Alister Maguire
//  Creation:    Jan 16, 2019
//
//  Modifications:
//
//    Matt Larsen, Wed May 31  08:15:42 PDT 2017
//    Adding functions and data structures for node and zone labels
//
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

    avtMiliMetaData     **miliMetaData;

    char                 *famroot;
    char                 *fampath;
    char                  filepath[512];

    int                   nTimesteps;
    int                   nDomains;
    int                   nMeshes;

    vector<Famid>         dbid;
    bool                  setTimesteps;
    vector<int>           cycles;
    vector<double>        times;

    std::map<int, int>    warn_map;
    vector<bool>          validateVars;
    vector<bool>          readMesh;
    int                   dims;

    vector<vector<vtkUnstructuredGrid *> > datasets;
    vector<vector<avtMaterial *> >         materials;

    void                  IssueWarning(const char *msg, 
                                       int key);

    void                  ReadMesh(int dom);

    void                  PopulateSubrecordInfo(int dom, 
                                                int meshId);

    void                  DecodeMultiMeshVarname(const string &, 
                                                 string &, int &);

    void                  DecodeMultiLevelVarname(const string &, 
                                                  string &);
    inline void           OpenDB(int dom);


    //TODO: move the json reading to meta data
    int                   CountJsonClassVariables(const rapidjson::Document &);

    void                  ExtractJsonVariable(MiliVariableMetaData *,
                                              const rapidjson::Value &,
                                              string,
                                              string,
                                              string,
                                              int);

    void                  ExtractJsonVectorComponents(const rapidjson::Value &,
                                                      const rapidjson::Value &,
                                                      string,
                                                      int);

    void                  ExtractJsonClasses(rapidjson::Document &,
                                             int);
    void                  LoadMiliInfoJson(const char *fname);

    //TODO: re-work the expression generators
    Expression            CreateGenericExpression(const char *, 
                                                  const char *,
                                                  Expression::ExprType);

    Expression            ScalarExpressionFromVec(const char *,
                                                  const char *, 
                                                  int);

    Expression            ScalarExpressionFromElementSet(const char *,
                                                         const char *,
                                                         int *);

    //TODO: inspect label integration
    struct Label_mapping
    {
        vector<int> label_ranges_begin;
        vector<int> label_ranges_end;
        vector<int> el_ids_begin;
        vector<int> el_ids_end;
    };

    vector<int>                             max_zone_label_lengths;
    vector<int>                             max_node_label_lengths;
    vector<vector<string> >                 zoneLabels;
    vector<vector<string> >                 nodeLabels;
    vector<std::map<string,Label_mapping> > zone_label_mappings;
    vector<std::map<string,Label_mapping> > node_label_mappings;

    void PopulateNodeLabels(const int fam_id, const int mesh_id, 
                            char *short_name, const int dom);

    void PopulateZoneLabels(const int fam_id, const int mesh_id, 
                            char *short_name, const int dom, 
                            const int elems_in_group);
};

#endif


