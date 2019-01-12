/*****************************************************************************
*
* Copyright (c) 2011, CEA
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of CEA, nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#ifndef LATA_H_INCLUDE
#define LATA_H_INCLUDE

#include <LataDB.h>
#include <LataStructures.h>
#include <UserFields.h>

typedef Field<FloatTab> FieldFloat;

// This file provides the LataFilter class: it is a dynamic mesh
//  and field generator which is able to load data from a lata file,
//  apply operators. Once computed, the data is kept in a data cache
//  to speed up further access to the same data.

// This class holds the LataFilter configuration (determines which 
//   combination of operators should be applied to the data)
class LataOptions 
{
public :
  static void extract_path_basename(const char * s, Nom & path_prefix, Nom & basename);
  static entier read_int_opt(const Nom & s);
  static double read_float_opt(const Nom & s);
  static Nom read_string_opt(const Nom & s);

  Nom basename; // Name of the case. 
  Nom path_prefix; // Path for the case.

  // Generate de the following meshes and associated data, if the flag is set.
  bool dual_mesh;
  bool nc_mesh;
  bool faces_mesh;
  bool boundary_mesh;

  bool   reconnect;  // Do we want to reconnect multiblock meshes
  float  reconnect_tolerance;
  int regularize_polyedre ; // if 1 Treate polyedre as poyledre extruder
  int    regularize;    // Do we want to force regularize the domain ie convert the mesh to a structured ijk (not necessary except for dual-mesh vdf)
                        // special value 2 means "regularize if faces present and vdf"
  int    extend_domain; // Extend the regularized domaine by n layers of cells
  float  regularize_tolerance;
  bool   invalidate; // invalidate unused positions and connections;
  bool   load_virtual_elements; // Do we want to extend the loaded mesh subblocks with a layer of virtual elements
  bool   export_fields_at_faces_; // Should we show these fields in exportable fields
  
  // When loading ijk regular meshes, virtually create this number of blocks in the K direction:
  int    ijk_mesh_nb_parts_;
  // When loading ijk regular meshes, merge N layers of virtual elements (default=1)
  int    ijk_virt_layer;

  bool   user_fields_; //activate user fields ?

  ArrOfDouble clipbox_min;
  ArrOfDouble clipbox_max;

  UserFields_options user_fields_options_;

  LataOptions();
  virtual entier parse_option(const Nom &);
  virtual void describe();
  virtual ~LataOptions() {};
};

class Operator : public LataObject
{
public:
  virtual void build_field(const Domain & src_domain, const LataField_base & src_field, 
                           const Domain & dest_domain, LataDeriv<LataField_base> & dest) = 0;
  virtual void build_geometry(const Domain & src_domain, LataDeriv<Domain> & dest) = 0;
protected:
};

struct LataGeometryMetaData
{
  Nom internal_name_; // Internal full name (eg DOM_IJK)
  Nom displayed_name_; // Short name showed to the user (DOM for DOM_IJK, ?? if the geometry should not be exported)
  entier dynamic_; // Is the geometry changing at each timestep ?
  entier dimension_; // spatial dimension of coordinates
  Domain::Element element_type_;
  entier nblocks_; // Number of sub_blocks in the geometry (parallel computation)
  Motcle source_; // How to build this domain ("latadb", "operator_ijk", "operator_dual", "operator_boundary", "user_fields")
  Nom source_domain_;
};

struct LataFieldMetaData
{
  Field_UName uname_;
  Nom name_;
  Nom geometry_name_;
  Noms component_names_;
  entier nb_components_;
  entier is_vector_; // Yes => nb_components is equal to spatial dimension
  LataField_base::Elem_som localisation_;
  Nom source_localisation_; // Localisation of source field (for displayed name in visit)
  Motcle source_; // How to build this field ("latadb", "operator_ijk", "operator_dual", "operator_boundary", "user_fields")
  Field_UName source_field_;
};

class DataCacheItem
{
public:
  DataCacheItem() : tstep_(-1), last_access_time_(0), lock_(0), memory_size_(0) {}
  LataDeriv<LataObject> item_; // The cached item
  Nom id_; // The id for this item
  entier tstep_; // The timestep of the cached data (for cache cleanup)
  BigEntier last_access_time_; // Last time this item has been accessed (for cache cleanup)
  // Is the item locked ? => cannot be deleted by clear_cache()
  // This is a counter: get_item increases, release_item dereases.
  // (this is when we simultaneously need several items, we must lock them to be sure)
  entier lock_; 
  // The memory size is computed when the item is released
  BigEntier memory_size_;
};

class LataFilterCache 
{
public:
  LataFilterCache() : cache_data_access_count_(0),
                      clear_cache_on_tstep_change_(1), cache_memory_limit_(-1) {};
  void reset() { data_.reset(); cache_data_access_count_ = 0; }
  void set_cache_properties(entier clear_on_tstep_change, BigEntier mem_limit);
  template<class C> C & get_item(const Nom & id, entier tstep)
  {
    LataDeriv<LataObject> & obj = get_item_(id, tstep);
    if (obj.non_nul())
      return obj.refcast(C);
    else
      return obj.instancie(C);
  }
  void release_item(const Nom & id);
  void remove_item(const Nom & id);
  void cleanup_cache(entier tstep_to_keep);
protected:
  LataDeriv<LataObject> & get_item_(const Nom & id, entier tstep);
  // Stored data (depends on caching strategy)
  // data_ grows when needed. 
  LataVector<DataCacheItem> data_;
  BigEntier cache_data_access_count_;
  // If nonzero, whenever we ask a timestep,
  //  remove all cached data from other timesteps
  entier clear_cache_on_tstep_change_;
  // If before getting a new geometry or field, the data cache
  //  uses more than the limit, remove old data until we are below.
  // -1 means "no limit"
  BigEntier cache_memory_limit_; // Limit in bytes
};

// Description: This is the MAIN class for the lata filter tool:
//  It reads data from a lata database on disk (initialize), 
//   and proposes several geometries and fields (get_exportable...) to the user.
//  The user can get them with get_geometry and get_field.
//  He must then call release_geometry and release_field to free the memory.
//  The user can also get metadata information (available without loading all
//   the data from disk) for geometries and fields and also timestep informations.
//  Timestep 0 contains global geometry and field definitions, timestep 1..n
//  are associated with each "TEMPS" entry in the lata file.
//
// LataFilter uses a data cache internally: it keeps fields and geometries after
// the user calls release_xxx(). The cache is controlled by set_cache_properties()
class LataFilter {
public:
  LataFilter() : lataDB__(0) {};
  void initialize(const LataOptions & opt, const LataDB & db);
  void set_cache_properties(BigEntier max_memory, const entier keep_all_timesteps);
  Noms get_exportable_geometry_names() const;
  const LataGeometryMetaData & get_geometry_metadata(const char * geometry) const;
  LataVector<Field_UName> get_exportable_field_unames(const char * geometry) const;
  const LataFieldMetaData & get_field_metadata(const Field_UName & uname) const;
  entier get_nb_timesteps() const;
  double get_timestep(entier i) const;

  const Domain &     get_geometry(const Domain_Id &);  

  void               release_geometry(const Domain &);
  const LataField_base & get_field(const Field_Id &);
  const FieldFloat & get_float_field(const Field_Id &) ;

  void               release_field(const LataField_base &);

  const LataDB & get_lataDB() const { return lataDB(); }

  const LataOptions & get_options() const { return opt_; }
protected:
  Operator &              get_set_operator(const Domain_Id & id);
  LataDeriv<LataField_base> & get_cached_field(const Field_Id&);
  LataDeriv<Domain> &     get_cached_domain(const Domain_Id&);
  LataDeriv<Operator> &   get_cached_operator(const Domain_Id&);
  void release_cached_domain(const Domain_Id&);
  void release_cached_field(const Field_Id&);
  void release_cached_operator(const Domain_Id&);
  const Domain & get_geom_field_(const Field_Id & id, LataRef<const LataField_base> & field_result);
  void get_all_metadata(LataVector<LataGeometryMetaData> & geoms_data, LataVector<LataFieldMetaData> & fields_data);
  // LataDB       & lataDB() { return lataDB__; }
  const LataDB & lataDB() const { assert(lataDB__); return *lataDB__; }

  // We store in the cache objects of type:
  //  LataDeriv<Domain>
  //  derived types of Operator
  //  LataDeriv<LataField_base>
  LataFilterCache data_cache_;

  // LataV2 masterfile database 
  const LataDB *lataDB__;
  LataOptions opt_;
  // Metadata information for all fields and geometries (built in initialize)
  LataVector<LataGeometryMetaData> geoms_metadata_;
  LataVector<LataFieldMetaData>    fields_metadata_;

  LataDeriv<UserFields> user_fields_;
};

struct LataError
{
  enum ErrorCode { NEED_REGULAR, NO_FACES, WRONG_ELT_TYPE, INVALID_TSTEP, INVALID_COMPONENT, 
                   INVALID_DOMAIN, OTHER};
  LataError(ErrorCode code, const char * msg) : code_(code), msg_(msg) {};
  ErrorCode code_;
  const char *msg_;
};

struct InternalError
{
  InternalError(const char *msg) : msg_(msg) {};
  const char *msg_;
};

void LataDB_apply_input_filter(const LataDB & lata_db, LataDB & filtered_db, 
                               const ArrOfInt & input_timesteps_filter,
                               const Noms & input_domains_filter,
                               const Noms & input_components_filter);
#endif

