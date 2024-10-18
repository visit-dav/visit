// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
// avtBlueprintFileFormat.h
// ************************************************************************* //

#ifndef AVT_BLUEPRINT_FILE_FORMAT_H
#define AVT_BLUEPRINT_FILE_FORMAT_H

#include <set>

// NOTE: This is from avtblueprint lib
#include "avtConduitBlueprintDataAdaptor.h"
// NOTE: This is from avtmfem lib
#include "avtMFEMDataAdaptor.h"

#include "avtSTMDFileFormat.h"

#include "conduit.hpp"

#include <string>
#include <vector>
#include <map>

class     avtMaterial;
class     avtSpecies;
class     DBOptionsAttributes;

// ****************************************************************************
//  Class: avtBlueprintFileFormat
//
//  Purpose:
//      Reads in Blueprint files as a plugin to VisIt.
//
//  Programmer: harrison37 -- generated by xml2avt
//  Creation:   Wed Jun 15 16:25:28 PST 2016
//
//  modifications:
//    Cyrus Harrison, Wed Feb 17 10:43:50 PST 2021
//    Added GetAuxiliaryData() to support material volume fractions 
//    and mixed-var data.
// 
//    Justin Privitera, Mon Apr 11 18:20:19 PDT 2022
//    Added `m_new_refine`, a boolean to toggle between using new MFEM LOR or 
//    legacy MFEM LOR. When true, the new scheme is enabled.
// 
//    Justin Privitera, Wed Aug 24 11:08:51 PDT 2022
//    Added includes for new avtmfem and avtblueprint libs.
// 
//    Justin Privitera, Wed Mar 22 15:48:01 PDT 2023
//    Included set and added a set called m_curve_names.
//
//    Cyrus Harrison, Thu Nov  2 11:25:15 PDT 2023
//    Added AugmentBlueprintIndex helper.
// 
//    Justin Privitera, Fri Sep 27 11:51:59 PDT 2024
//    Added support for species sets.
// ****************************************************************************

class avtBlueprintTreeCache;

class avtBlueprintFileFormat : public avtSTMDFileFormat
{
  public:
                           avtBlueprintFileFormat(const char *, DBOptionsAttributes*);
    virtual               ~avtBlueprintFileFormat();

    virtual const char    *GetType(void) 
                                { return "Blueprint"; }

    // Standard Mesh Access
    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    // Other types of Mesh Data (Materials, etc)
    virtual void          *GetAuxiliaryData(const char *var,
                                            int domain,
                                            const char *type,
                                            void *args,
                                            DestructorFunction &);

    // VisIt can't cache for us b/c we need to implement LOD support. 
    virtual bool           CanCacheVariable(const char *var)
                                { return false; }

    // Used to enable support for avtResolutionSelection
    virtual void           RegisterDataSelections(
                                const std::vector<avtDataSelection_p> &selList,
                                std::vector<bool> *selectionsApplied);
    // cycle and time support
    virtual int            GetCycle(void);
    virtual double         GetTime(void);

    
    virtual void           FreeUpResources(void); 

  protected:

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    bool                   HasInvariantMetaData(void) const
                                { return false; }

    bool                   HasInvariantSIL(void) const
                                { return false; }

  private:
    static const std::string DISPLAY_NAME;

    void                   AddBlueprintMeshAndFieldMetadata(avtDatabaseMetaData *md,
                                                            std::string const &mesh_name, 
                                                            const conduit::Node &n_mesh_info);

    void                   AddBlueprintMaterialsMetadata(avtDatabaseMetaData *md,
                                                         std::string const &mesh_name,
                                                         const conduit::Node &n_mesh_info);

    void                   AddBlueprintSpeciesMetadata(avtDatabaseMetaData *md,
                                                       std::string const &mesh_name,
                                                       const conduit::Node &n_mesh_info);

    void                   ReadRootFile();

    void                   ReadRootIndexItems(const std::string &root_fname,
                                              const std::string &root_protocol,
                                              conduit::Node &root_info);

    void AugmentBlueprintIndex(conduit::Node &blueprint_index);

    void                   ReadBlueprintMesh(int domain,
                                             const std::string &abs_meshname,
                                             conduit::Node &out);

    void                   ReadBlueprintField(int domain,
                                              const std::string &abs_varname,
                                              conduit::Node &out);

    bool                   DetectHOMaterial(const std::string &mesh_name,
                                            const std::string &topo_name,
                                            const std::vector<std::string> &matNames,
                                            std::map<std::string, std::string> &matFields,
                                            std::string &freeMatName) const;

    void                   ReadBlueprintMatset(int domain,
                                               const std::string &abs_matsetname,
                                               conduit::Node &out);

    void                   ReadBlueprintSpecset(int domain,
                                                const std::string &abs_matsetname,
                                                conduit::Node &out);

    void                   FetchMeshAndTopoNames(const std::string &name_name_full,
                                                 std::string &mesh_name,
                                                 std::string &topo_name);

    avtMaterial           *GetMaterial(int domain, const char *mat_name);

    avtSpecies            *GetSpecies(int domain, const char *spec_name);

    const conduit::Node   *GetBlueprintIndexForField(const conduit::Node &fields,
                                                     const std::string &abs_varname) const;

    conduit::Node          m_root_node;
    
    std::string            m_protocol;
    
    avtBlueprintTreeCache  *m_tree_cache;
  
    int                    m_selected_lod;

    conduit::Node          m_mesh_and_topo_info;
    conduit::Node          m_matset_info;
    conduit::Node          m_specset_info;

    std::map<std::string,bool> m_mfem_mesh_map;
    std::map<std::string,std::pair<std::string,int> > m_mfem_material_map;

    std::set<std::string>  m_curve_names;

    bool                   m_new_refine;

};


#endif
