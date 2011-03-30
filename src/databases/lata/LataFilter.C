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

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <LataFilter.h>
#include <list>
#include <Static_Int_Lists.h>
#include <Connectivite_som_elem.h>
#include <LataDB.h>
#include <Lata_tools.h>
#include <Operator.h>
#include <errno.h>
#include <UserFields.h>
#include <string.h>
static const entier cache_info_level = 5;
static const entier filter_info_level = 4;

entier LataOptions::read_int_opt(const Nom & s)
{
  const char *ptr = strstr(s, "=");
  if (!ptr) 
    ptr = s;
  errno = 0;
  char *errorptr = 0;
  entier x = strtol(ptr+1, &errorptr, 0 /* base 10 par defaut */);
  if (errno || *errorptr != 0) {
    Journal() << "LataOptions error reading int parameter: " << s << endl;
    throw;
  }
  return x;
}

double LataOptions::read_float_opt(const Nom & s)
{
  const char *ptr = strstr(s, "=");
  if (!ptr) 
    ptr = s;
  errno = 0;
  char *errorptr = 0;
  double x = strtod(ptr+1, &errorptr);
  if (errno || *errorptr != 0) {
    Journal() << "LataOptions error reading float parameter: " << s << endl;
    throw;
  }
  return x;
}

Nom LataOptions::read_string_opt(const Nom & s)
{
  const char *ptr = strstr(s, "=");
  if (!ptr) 
    return s;
  else
    return Nom(ptr+1);
}

Noms extract_list(const Nom & n) 
{
  Noms liste;
  if (n == "")
    return liste;
  const char *ptr = n;
  Nom tmp("");
  while (*ptr) {
    if (*ptr == ',') {
      liste.add(tmp);
      tmp = "";
    } else {
      tmp += Nom(*ptr);
    }
  }
  liste.add(tmp);
  return liste;
}

void LataOptions::describe()
{
  cerr << "Data processing options (provided by the LataFilter module)" << endl;
  cerr << " reconnect=tolerance : Find duplicate positions, and redefine connections to use" << endl;
  cerr << "           always the same positions. tolerance is the maximum distance between" << endl;
  cerr << "           positions that are considered equal." << endl;
  cerr << "           Useful for results of parallel runs, where the domain could" << endl;
  cerr << "           appear fragmented in as many subdomains as procs used." << endl;
  cerr << "           This operator modifies the loaded domain, does not create a new domain." << endl;
  cerr << " regularize=tolerance : Try to transform the irregular domain in a ijk domain." << endl;
  cerr << "            tolerance is the maximum dx, dy or dz between positions that are" << endl;
  cerr << "            considered to have the same x, y or z." << endl;
  cerr << "            !Attention! regularize recalculates positions, connections, and fields" << endl;
  cerr << "            Except in particularly simple cases, you cannot use a regularized domain" << endl;
  cerr << "            for a field that has not been regularized the same way." << endl;
  cerr << "            Create a new domain DOM -> DOM_IJK" << endl;
  cerr << " regularize_polyedre=N : tells how to convert polyedre elements to VTK:" << endl;
  cerr << "                         N=0 (default): any shape, handled as general VTK_CONVEX_POINT_SET" << endl;
  cerr << "                         N=1: all elements are extruded in the Z direction (faster and nicer)" << endl;
  cerr << " ijk_mesh_nb_parts=N : When loading an IJK mesh, automatically split the mesh into" << endl;
  cerr << "            N blocks (N must be <= number of elements in the K direction, usefull in visit)" << endl;
  cerr << " invalidate : together with regularize, create \"invalid positions\" and \"invalid connections\"" << endl;
  cerr << "            components. Otherwise only set the values of the field to zeroes." << endl;
  cerr << " extend_domain=n : When regularizing, add n nodes in each directions to have a layer" << endl;
  cerr << "           of invalid positions and connections." << endl;
  cerr << " dualmesh: Build and export the dual mesh" << endl;
  cerr << "           (control volume for the velocities in VEF)" << endl;
  cerr << "           data at faces become data at elements on the dual mesh." << endl;
  cerr << "           Create a new domain DOM -> DOM_dual" << endl;
  cerr << " facesmesh: Build and export the faces mesh" << endl;
  cerr << "           (control volume for the velocities in VEF)" << endl;
  cerr << "           data at faces become data at elements on the faces mesh." << endl;
  cerr << "           Create a new domain DOM -> DOM_faces" << endl;

  //cerr << " ncmesh:   Build and export a non-conforming mesh" << endl;
  //cerr << "           (each mesh element has its proprietary nodes, elements are not topologically connected)" << endl;
  //cerr << "           face dependent data become position dependent data on the new mesh." << endl;
  //cerr << "           Create a new domain DOM -> DOM_nc" << endl;
  cerr << " boundarymesh: Build new domains containing the boundaries only" << endl;
  cerr << "            Create a new domain DOM -> DOM_Boundary" << endl;
  //cerr << " clipbox=xmin,ymin,zmin,xmax,ymax,zmax : remove from all meshes all nodes," << endl;
  //cerr << "           connections and faces outside of this box" << endl;
  cerr << " load_virtual_elements: Read the VIRTUAL_ELEMENTS data in the input database, if available" << endl
       << "           and merge nodes, elements and faces to each requested subdomain (using reconnect)" << endl;
  cerr << "           Does not create a new domain, but modifies the loaded domain" << endl;
  cerr << " ijk_virt_layer=N: load N layers of virtual elements if domain is ijk in lata file" << endl;
  cerr << " reconnect_tolerance=: specify tolerance for all reconnect operations" << endl;
  cerr << "           (reconnection is applied with load_virtual_elements)" << endl;
  cerr << " user_fields: activate the User_Fields module (WARN: this module is often application specific," << endl;
  cerr << "           it will fail if some requiered domains/fields are missing in the input database)" << endl;
  cerr << " export_fields_at_faces: tells to export fields located at faces" << endl;
  cerr << endl;
  user_fields_options_.print_help_option();
}

entier LataOptions::parse_option(const Nom & s)
{
  if (s.debute_par("verbosity=")) {
    entier level = read_int_opt(s);
    set_Journal_level(level);
  } else if (s.debute_par("regularize=")) {
    regularize = true;
    regularize_tolerance = read_float_opt(s);
  } else if (s.debute_par("regularize_polyedre=")) {
    regularize_polyedre = read_int_opt(s);
  } else if (s.debute_par("extend_domain=")) {
    extend_domain = read_int_opt(s);
  } else if (s == "invalidate") {
    invalidate = true;
  } else if (s.debute_par("reconnect=")) {
    reconnect = true;
    reconnect_tolerance = read_float_opt(s);   
  } else if (s.debute_par("reconnect_tolerance=")) {
    reconnect_tolerance = read_float_opt(s);   
  } else if (s == "dualmesh") {
    dual_mesh = true;
  } else if (s == "nodualmesh") {
    dual_mesh = false;
  } else if (s == "ncmesh") {
    nc_mesh = true;
  } else if (s == "facesmesh") {
    faces_mesh = true;
  } else if (s == "nofacesmesh") {
    faces_mesh = false;
  } else if (s == "boundarymesh") {
    boundary_mesh = true;
  } else if (s.debute_par("clipbox=")) {
    Noms list = extract_list(((const char*)s)+8);
    if (list.size() != 6) {
      Journal() << "Error : clipbox parameters expects 6 values" << endl;
      throw;
    }
    for (entier i = 0; i < 3; i++) {
      clipbox_min[i] = read_float_opt(list[i]);
      clipbox_max[i] = read_float_opt(list[i+3]);
    }
  } else if (s == "load_virtual_elements") {
    load_virtual_elements = true;
  } else if (s == "user_fields") {
    user_fields_ = true;
    Journal() << "Option: User_fields ON" << endl;
  } else if (s.debute_par("ijk_mesh_nb_parts")) {
    ijk_mesh_nb_parts_ = read_int_opt(s);
  } else if (s == "export_fields_at_faces") {
    export_fields_at_faces_ = 1;
  } else if (s.debute_par("ijk_virt_layer=")) {
    ijk_virt_layer = read_int_opt(s);
  } else
    return user_fields_options_.parse_option(s);;
  return 1;
}

void LataFilterCache::set_cache_properties(entier clear_on_tstep_change, BigEntier mem_limit)
{ 
  clear_cache_on_tstep_change_ = clear_on_tstep_change;
  cache_memory_limit_ = mem_limit;
}

// Description: if an entry with "id" tag and timestep exists in the cache,
//  returns the entry, otherwise returns a reference to an empty DERIV that
//  is stored in the cache and stores the associated "id" and timestep.
//  The entry is locked and given a new last_access_time_ to show that it has
//  been used recently.
//  The entry must be released by release_item() when we are finished working
//  with it.
LataDeriv<LataObject> & LataFilterCache::get_item_(const Nom & id, entier tstep)
{
  entier i;
  const entier n = data_.size();
  for (i = 0; i < n; i++) {
    const DataCacheItem & item = data_[i];
    if (item.id_ == id && item.tstep_ == tstep)
      break;
  }
  if (i == n) {
    // Look for an empty slot:
    for (i = 0; i < n; i++)
      if (data_[i].id_ == "??")
        break;
    // No empty slot: create a new slot:
    if (i == n)
      data_.add();
    DataCacheItem & item = data_[i];
    item.id_ = id;
    item.tstep_ = tstep;
    item.lock_ = 0;
    Journal(cache_info_level) << "LataFilterCache<C>::get " << id << " (new cache entry " << i << ")." << endl;
  } else {
    Journal(cache_info_level) << "LataFilterCache<C>::get " << id << " (existing cache entry " << i << ")." << endl;
  }
  // Mark item and lock it:
  DataCacheItem & item = data_[i];
  item.last_access_time_ = cache_data_access_count_++;
  item.lock_++;
  return item.item_;
}


// Description: tells that if needed the item can be deleted from cache
//  (there is no reference to it anymore outside of the cache).
//  We update the memory size of this item here.
void LataFilterCache::release_item(const Nom & id)
{
  Journal(cache_info_level) << "LataFilterCache::release_item " << id << endl;
  const entier n = data_.size();
  entier i;
  for (i = 0; i < n; i++) {
    const DataCacheItem & item = data_[i];
    if (item.id_ == id)
      break;
  }
  if (i == n) {
    Journal() << "LataFilterCache::release_item internal error: unknown item " << id << endl;
    throw;
  }
  if (data_[i].lock_ <= 0) {
    Journal() << "LataFilterCache::release_item internal error: item is already unlocked" << id << endl;
    throw;
  }
  data_[i].last_access_time_ = cache_data_access_count_++;
  data_[i].lock_--;
  if (data_[i].item_.non_nul())
    data_[i].memory_size_ = data_[i].item_.valeur().compute_memory_size();
  else
    data_[i].memory_size_ = 0;
}

// Description: removes from the cache the oldest items until the total
//  memory used by the cache is below max_mem_size (in bytes), and
//  if tstep_to_keep > 0, also removes all timesteps except 0 and tstep_to_keep
void LataFilterCache::cleanup_cache(entier tstep_to_keep)
{
  if (clear_cache_on_tstep_change_ && tstep_to_keep > 0) {
    Journal(cache_info_level) << "LataFilterCache::clear_cache_tsteps except 0 and " << tstep_to_keep << endl;
    const entier n = data_.size();
    for (entier i = 0; i < n; i++) {
      DataCacheItem & item = data_[i];
      if (item.id_ != "??") {
        if (item.tstep_ == 0 || item.tstep_ == tstep_to_keep) {
          Journal(cache_info_level+1) << " item " << item.id_ << " timestep " << item.tstep_ << " kept" << endl;
        } else if (item.lock_) {
          Journal(cache_info_level+1) << " item " << item.id_ << " locked" << endl;
        } else {
          Journal(cache_info_level) << " deleting item " << item.id_ << " " << item.tstep_ << endl;
          item.item_.reset();
          item.id_ = "??";
          item.tstep_ = -1;
        }
      }
    }
  }
  if (cache_memory_limit_ >= 0) {
    Journal(cache_info_level) << "LataFilterCache::clear_cache_memory " << cache_memory_limit_ << endl;
    do {
      const entier n = data_.size();
      // Scan cached data, looking for the oldest item and summing up memory
      BigEntier total_memsize = 0;
      entier oldest = -1;
      BigEntier oldest_time = cache_data_access_count_;
      for (entier i = 0; i < n; i++) {
        const DataCacheItem & item = data_[i];
        if (item.id_ != "??") {
          total_memsize += item.memory_size_;
          if (!item.lock_ && item.last_access_time_ < oldest_time) {
            oldest_time = item.last_access_time_;
            oldest = i;
          }
        }
      }
      if (oldest < 0 || total_memsize < cache_memory_limit_) 
        break;
      
      DataCacheItem & item = data_[oldest];
      Journal(cache_info_level) << " deleting item " << item.id_ << " " << item.tstep_ << endl;
      item.item_.reset();
      item.id_ = "??";
      item.tstep_ = -1;
    } while(1);
  }
}

// Description: Cleanup everything, associate the lata_db and fills metadata information.
void LataFilter::initialize(const LataOptions & opt, const LataDB & lata_db)
{
  opt_ = opt;
  data_cache_.reset();
  lataDB__ = &lata_db;
  if (opt_.user_fields_) {
    user_fields_.instancie(UserFields);
    user_fields_.valeur().set_options(opt_.user_fields_options_);
  }

  get_all_metadata(geoms_metadata_, fields_metadata_);
}

// Description: Return the number of timesteps in the database
//   (=number of physical timesteps + one containing global definitions at timestep 0)
entier LataFilter::get_nb_timesteps() const
{
  return lataDB().nb_timesteps();
}

// Description: Return the physical time for this timestep.
//  returns -1.0 for timestep 0 (global definitions)
double LataFilter::get_timestep(entier i) const
{
  if (i == 0)
    return -1.0;
  else
    return lataDB().get_time(i);
}

static void add_fields_to_metadata_list(const LataDB & lataDB, 
                                        const Nom & lata_geom, 
                                        const Nom & dest_geom, 
                                        const Nom & options,
                                        entier dim,
                                        LataVector<LataFieldMetaData> & fields_data,
                                        const Motcle & source,
                                        const Nom & source_domain)
{
  // Query for existing fields in the latadb :
  Field_UNames lata_fields = lataDB.field_unames(1, lata_geom, "*", LataDB::FIRST_AND_CURRENT);
  const entier nb_fields = lata_fields.size();
  for (entier i_field = 0; i_field < nb_fields; i_field++) {
    const LataDBField & lata_field = lataDB.get_field(1, lata_fields[i_field], LataDB::FIRST_AND_CURRENT);
    LataField_base::Elem_som loc = LataField_base::localisation_from_string(lata_field.localisation_);

    // Hidden special fields
    if (Motcle(lata_field.name_) == "INVALID_CONNECTIONS")
      continue;
   if (Motcle(lata_field.name_) == "ELEMENTS")
      continue;
   if (Motcle(lata_field.name_) == "FACES")
      continue;
   if (Motcle(lata_field.name_) == "ELEM_FACES")
     continue;
    LataFieldMetaData data;
    data.name_ = lata_field.name_;
    data.geometry_name_ = dest_geom;
    data.component_names_ = lata_field.component_names_;
    data.nb_components_ = lata_field.nb_comp_;
    data.source_localisation_ = lata_field.localisation_;

    if (options.find("to_vector")>=0) {
      data.is_vector_ = 1;
      data.nb_components_ = dim;
    } else 
      data.is_vector_ = (lata_field.nature_ == LataDBField::VECTOR);

    if (options.find("to_elem")>=0)
      data.localisation_ = LataField_base::ELEM;
    else if (options.find("to_som")>=0)
      data.localisation_ = LataField_base::SOM;
    else if (options.find("to_faces")>=0)
      data.localisation_ = LataField_base::FACES;
    else
      data.localisation_ = loc;

    data.source_ = source;
    data.uname_ = Field_UName(data.geometry_name_, 
                              data.name_, 
                              LataField_base::localisation_to_string(data.localisation_));
    data.source_field_ = Field_UName(source_domain,
                                     data.name_,
                                     lata_fields[i_field].get_localisation());

    if ((loc == LataField_base::ELEM && options.find("from_elem")>=0)
        || (loc == LataField_base::SOM && options.find("from_som")>=0)
        || (loc == LataField_base::FACES && options.find("from_faces")>=0)) {
      Journal(filter_info_level) << " register field metadata: " << data.uname_ << endl;
      fields_data.add(data);
    }
  }
}

// Process the content of the source LataDB structure and builds the metadata for
//  all geometries and fields that the filter can export (depending on options,
//  for example, provide dual mesh geometry and fields only if dualmesh option is on).
void LataFilter::get_all_metadata(LataVector<LataGeometryMetaData> & geoms_data, LataVector<LataFieldMetaData> & fields_data)
{
  geoms_data.reset();
  fields_data.reset();
  entier current_tstep = 1;
  // If no real timestep, just check timestep 0
  if (lataDB().nb_timesteps() < 2)
    current_tstep = 0;
  Noms lata_geoms_names = lataDB().geometry_names(current_tstep, LataDB::FIRST_AND_CURRENT);
  const entier nb_geoms = lata_geoms_names.size();
  for (entier i_geom = 0; i_geom < nb_geoms; i_geom++) {
    // Name of the current geometry (from lataDB)
    const Nom & lata_geom_name = lata_geoms_names[i_geom];
    const LataDBGeometry & lata_geom = lataDB().get_geometry(current_tstep, lata_geom_name, LataDB::FIRST_AND_CURRENT);
    // Query properties from LataDB:
    // Is it a dynamic mesh ?
    const entier dynamic = lata_geom.timestep_ > 0;
    // Element type ?
    Domain::Element element_type = Domain::element_type_from_string(lata_geom.elem_type_);
    // It is regularizable ?
    entier regularizable = (element_type == Domain::quadri || element_type == Domain::hexa)
      && (lata_geom.elem_type_ != "HEXAEDRE_AXI") && (lata_geom.elem_type_ != "RECTANGLE_AXI");
    Journal(filter_info_level) << " metadata: geometry " << lata_geom_name << " element type says regularizable=" << regularizable << endl;
    // Query for dimension
    const entier domain_already_ijk = lataDB().field_exists(current_tstep, lata_geom_name, "SOMMETS_IJK_I", LataDB::FIRST_AND_CURRENT);

    // Do we have faces ?
    const entier have_faces = 
      domain_already_ijk ||
      (lataDB().field_exists(current_tstep, lata_geom_name, "FACES", LataDB::FIRST_AND_CURRENT)
       && lataDB().field_exists(current_tstep, lata_geom_name, "ELEM_FACES", LataDB::FIRST_AND_CURRENT));

    entier dim = 1;
    // Query for number of blocks in the lata file:
    entier nblocks = 1;
    if (domain_already_ijk) {
      if (lataDB().field_exists(current_tstep, lata_geom_name, "SOMMETS_IJK_K", LataDB::FIRST_AND_CURRENT))
        dim = 3;
      else
        dim = 2;
      nblocks = opt_.ijk_mesh_nb_parts_;
      Nom nom_sommets;
      if (dim == 2)
        nom_sommets = "SOMMETS_IJK_J";
      else
        nom_sommets = "SOMMETS_IJK_K";
      const LataDBField & coord = lataDB().get_field(current_tstep, lata_geom_name, nom_sommets, "", LataDB::FIRST_AND_CURRENT);
      // Nombre d'elements dans la direction du decoupage parallele:
      const entier nelem = coord.size_ - 1;
      // Si les tranches sont trop petites diminuer le nombre de blocs
      if (nblocks > (nelem + 3) / 4)
        nblocks = (nelem + 3) / 4;
    } else {
      dim = lataDB().get_field(current_tstep, lata_geom_name, "SOMMETS", "*", LataDB::FIRST_AND_CURRENT).nb_comp_;
      if (lataDB().field_exists(current_tstep, lata_geom_name, "JOINTS_SOMMETS", LataDB::FIRST_AND_CURRENT))
        nblocks = lataDB().get_field(current_tstep, lata_geom_name, "JOINTS_SOMMETS", "*", LataDB::FIRST_AND_CURRENT).size_;
    }

    // Initialize data common to all domains:
    LataGeometryMetaData data;
    data.dynamic_ = dynamic;
    data.dimension_ = dim;
    data.element_type_ = element_type;
    // If we reconnect all subdomains, always load all of them:
    if (!opt_.reconnect)
      data.nblocks_ = nblocks;
    else
      data.nblocks_ = 1;

    data.internal_name_ = lata_geom_name;
    data.displayed_name_ = lata_geom_name;
    data.source_ = "latadb";
    Journal(filter_info_level) << " metadata: adding geometry " << lata_geom_name << " displayed name=" << lata_geom_name << endl;
    geoms_data.add(data);
    // Add fields at som and elem:
    add_fields_to_metadata_list(lataDB(), lata_geom_name, data.internal_name_,
                                "from_elem,from_som,from_faces", dim, fields_data,
                                "latadb",
                                "??");
    if (regularizable && ((opt_.regularize_tolerance < 0) || (!opt_.regularize))) {
      regularizable = 0;
      Journal(filter_info_level) << " regularize option not set: don't build ijk domain" << endl;
    }
    if (regularizable && domain_already_ijk) {
      regularizable = 0;
      Journal(filter_info_level) << " domain is already IJK: do not regularize" << endl;
    }

    // opt_.regularize == 2 means: provide ijk only if faces are present
    if (regularizable && opt_.regularize == 2) {
      if (!have_faces) {
        Journal(filter_info_level) << " regularize option==2 and no faces => do not regularize" << endl;
        regularizable = 0;
      }
    }
    if (regularizable) {
      data.internal_name_ = lata_geom_name;
      data.internal_name_ += "_IJK";
      data.displayed_name_ = lata_geom_name;
      data.source_ = "operator_ijk";
      data.source_domain_ = lata_geom_name;
      geoms_data.add(data);
      Journal(filter_info_level) << " metadata: adding geometry " << data.internal_name_ << " displayed name=" << data.displayed_name_ << endl;
      // Add fields at som and elem:
      add_fields_to_metadata_list(lataDB(), lata_geom_name, data.internal_name_,
                                  "from_elem,from_som,from_faces", dim, fields_data,
                                  "operator_ijk",
                                  data.source_domain_);
    }

    // Provide dual mesh
    if (opt_.dual_mesh && have_faces) {
      data.internal_name_ = lata_geom_name;
      
      // If it's quadri or hexa, we need the regular mesh
      data.source_domain_ = data.internal_name_;
      if (regularizable) {
        data.internal_name_ += "_IJK";
        data.source_domain_ += "_IJK";
      }
      data.internal_name_ += "_dual";
      data.displayed_name_ += "_dual";

      data.source_ = "operator_dual";
      geoms_data.add(data);
      Journal(filter_info_level) << " metadata: adding geometry " << data.internal_name_ << " displayed name=" << data.displayed_name_ << endl;
      // Add fields at faces, localisation will be at elements,
      //  forced to vector type if vdf:
      Nom options("from_faces,to_elem");
      if (regularizable)
        options += ",to_vector";
      add_fields_to_metadata_list(lataDB(), lata_geom_name, data.internal_name_,
                                  options, dim, fields_data,
                                  "operator_dual",
                                  data.source_domain_);      
    }

    // Provide nc mesh if possible
    if (opt_.nc_mesh && have_faces && !regularizable /* doesn't work for vdf */) {
      data.internal_name_ = lata_geom_name;
      data.internal_name_ += "_nc";
      data.displayed_name_ = data.internal_name_;
      data.source_ = "operator_nc";
      data.source_domain_ = lata_geom_name;
      geoms_data.add(data);
      Journal(filter_info_level) << " metadata: adding geometry " << data.internal_name_ << " displayed name=" << data.displayed_name_ << endl;
      // Add fields at faces, localisation will be at nodes
      Nom options("from_faces,to_som");
      add_fields_to_metadata_list(lataDB(), lata_geom_name, data.internal_name_,
                                  options, dim, fields_data,
                                  "operator_nc",
                                  data.source_domain_);
    }
    // Provide faces mesh if possible
    if (opt_.faces_mesh && have_faces && !regularizable /* doesn't work for vdf */ &&  !(domain_already_ijk) ) {
   
      data.internal_name_ = lata_geom_name;
      data.internal_name_ += "_centerfaces";
      data.displayed_name_ = data.internal_name_;
      data.source_ = "operator_faces";
      data.source_domain_ = lata_geom_name;
      if (data.element_type_ == Domain::triangle)
        data.element_type_=Domain::line;
      else if ( data.element_type_ == Domain::tetra)
        data.element_type_=Domain::triangle;

      geoms_data.add(data);
      Journal(filter_info_level) << " metadata: adding geometry " << data.internal_name_ << " displayed name=" << data.displayed_name_ << endl;
      // Add fields at faces, localisation will be at nodes
      Nom options("from_faces,to_elem");
      add_fields_to_metadata_list(lataDB(), lata_geom_name, data.internal_name_,
                                  options, dim, fields_data,
                                  "operator_faces",
                                  data.source_domain_);
    }
    // Provide boundary mesh
    if (opt_.boundary_mesh && (element_type == Domain::hexa || element_type == Domain::tetra)) {
      data.internal_name_ = lata_geom_name;
      data.internal_name_ += "_Boundary";
      data.displayed_name_ = data.internal_name_;
      data.source_ = "operator_boundary";
      data.source_domain_ = lata_geom_name;
      geoms_data.add(data);
      Journal(filter_info_level) << " metadata: adding geometry " << data.internal_name_ << " displayed name=" << data.displayed_name_ << endl;
      Nom options("from_elem,from_som");
      add_fields_to_metadata_list(lataDB(), lata_geom_name, data.internal_name_,
                                  options, dim, fields_data,
                                  "operator_boundary",
                                  data.source_domain_);
      options = "from_faces,to_elem";
      add_fields_to_metadata_list(lataDB(), lata_geom_name, data.internal_name_,
                                  options, dim, fields_data,
                                  "operator_boundary",
                                  data.source_domain_);
    }
  }

  if (user_fields_.non_nul())
    user_fields_.valeur().new_fields_metadata(*this, fields_data);
}

// Description: Return a list of domain names available from get_geometry_metadata()
//  and that we want to show to the user.
//  This includes the domains loaded from the lata file plus new constructed domains
//  (nc_mesh, dual_mesh, boundary_mesh, etc) if requested in options and if available
//  (depending if requiered data (eg faces, ...)
Noms LataFilter::get_exportable_geometry_names() const
{
  Noms names;
  entier i;
  for (i = 0; i < geoms_metadata_.size(); i++)
    names.add(geoms_metadata_[i].internal_name_);

  // If an IJK domain is here, don't show the original domain:
  Noms names2;
  for (i = 0; i < names.size(); i++) {
    Nom n(names[i]);
    n += "_IJK";
    if (names.rang(n) < 0)
      names2.add(names[i]);
  }
  return names2;
}

// Description: the same, with field names...
//  If geometry=="*", returns all fields
//  Currently, doesn't show fields located at faces...
Field_UNames LataFilter::get_exportable_field_unames(const char * geometry) const
{
  Field_UNames unames;
  Motcle geom(geometry);
  for (entier i = 0; i < fields_metadata_.size(); i++)
    if (geom == fields_metadata_[i].geometry_name_ || geom == "*") 
      // Do not show faces located fields to the user...
      if (fields_metadata_[i].localisation_ != LataField_base::FACES || opt_.export_fields_at_faces_)
        unames.add(fields_metadata_[i].uname_);

  return unames;
}

// Description: fill "data" for the requested "geometry". "geometry" must be a name
//  returned by get_exportable_geometry_names()
const LataGeometryMetaData & LataFilter::get_geometry_metadata(const char * geometry) const
{
  Motcle geom(geometry);
  for (entier i = 0; i < geoms_metadata_.size(); i++)
    if (geom == geoms_metadata_[i].internal_name_)
      return geoms_metadata_[i];

  Journal() << "Error in LataFilter::get_geometry_metadata: unknown geometry " << geometry << endl;
  throw;
  return geoms_metadata_[0];
}

// Description: fill "data" for the requested "geometry/field". "geometry"  and "field" must be names
//  returned by get_exportable_geometry_names() and get_exportable_field_names()
const LataFieldMetaData & LataFilter::get_field_metadata(const Field_UName & uname) const
{
  for (entier i = 0; i < fields_metadata_.size(); i++)
    if (fields_metadata_[i].uname_ == uname)
      return fields_metadata_[i];
  
  Journal() << "Error in LataFilter::get_field_metadata: unknown field " << uname << endl;
  throw;
  return fields_metadata_[0];
}

// Description: 
//  Returns a reference to the requested geometry.
//  If the geometry is not found at the requested timestep, it
//  is seached in the first timestep.
//  If the geometry does not exist in the cache, all needed data is loaded
//   and the geometry is allocated and built in the internal cache.
//  The reference is valid until the user calls release_geometry()
//  The user MUST call release_geometry() to allow the data to be
//   removed from the data cache.
const Domain & LataFilter::get_geometry(const Domain_Id & id)
{
  Journal(filter_info_level) << "LataFilter::get_geometry " 
                             << id.name_ << " time=" << id.timestep_ 
                             << " bloc=" << id.block_ << endl;
  data_cache_.cleanup_cache(id.timestep_);

  Domain_Id requested_id(id);
  // Get the real timestep where this domain is stored:
  const LataGeometryMetaData & geom_metadata = get_geometry_metadata(id.name_);
  if (geom_metadata.dynamic_) 
    requested_id.timestep_ = id.timestep_;
  else
    requested_id.timestep_ = 0;

  LataDeriv<Domain> & dom_ptr = get_cached_domain(requested_id);
  if (!dom_ptr.non_nul()) {
    if (geom_metadata.source_ == "latadb") {
      // Request for a native domain : load it from lataDB
      // If reconnect and loading all subdomains, go ! Don't store the operator in cache since it's
      //  not required to process fields.
      
      // Is it a structured or unstructured mesh ?
      if (lataDB().field_exists(requested_id.timestep_, requested_id.name_, "SOMMETS")) 
        {
          DomainUnstructured & dom = dom_ptr.instancie(DomainUnstructured);
      
          if (opt_.reconnect) {
            // Bloc demande, peut etre le bloc 0 ou le bloc -1:
            const entier req_block = requested_id.block_;
            if (requested_id.block_ > 0) {
              Cerr << "Error: requesting block " << requested_id.block_ << " with reconnect option" << endl;
              exit(-1);
            }
            requested_id.block_ = -1; // load all blocks
            dom.fill_domain_from_lataDB(lataDB(), requested_id, 1 /* faces */, 0);
            Reconnect::reconnect_geometry(dom, opt_.reconnect_tolerance);
            dom.id_.block_ = req_block;
          } else {
            dom.fill_domain_from_lataDB(lataDB(), requested_id, 1 /* faces */, opt_.load_virtual_elements ? 1 : 0);
            if (opt_.load_virtual_elements && dom.nb_virt_items(LataField_base::ELEM) > 0) {
              Reconnect::reconnect_geometry(dom, opt_.reconnect_tolerance,
                                            dom.nb_nodes() - dom.nb_virt_items(LataField_base::SOM));
            }
          }
        }
      else
        {
          // Structured ijk:
          DomainIJK & dom = dom_ptr.instancie(DomainIJK);
          if (opt_.reconnect || requested_id.block_ < 0) {
            dom.fill_domain_from_lataDB(lataDB(), requested_id, 1 /* parallel splitting */, 
                                        0 /* no virtual elements */);
          } else {
            const entier nparts = opt_.ijk_mesh_nb_parts_;
            const entier virtual_size = opt_.load_virtual_elements ? opt_.ijk_virt_layer : 0;
            dom.fill_domain_from_lataDB(lataDB(), requested_id, nparts /* parallel splitting */, 
                                        virtual_size /* with virtual elements */);
          }
        }
    } else if (geom_metadata.source_.debute_par("OPERATOR")) {
      const Domain & src_domain = get_geometry(Domain_Id(geom_metadata.source_domain_, 
                                                         requested_id.timestep_,
                                                         requested_id.block_));
      Operator & op = get_set_operator(requested_id);
      op.build_geometry(src_domain, dom_ptr);
      dom_ptr.valeur().id_ = requested_id;
      release_cached_operator(requested_id);
      release_geometry(src_domain);
    } else {
      Journal() << "Unknown source in geometry metadata " << geom_metadata.source_ << endl;
      throw;
    }
  }

  return dom_ptr.valeur();
}

Operator & LataFilter::get_set_operator(const Domain_Id & id)
{
  LataDeriv<Operator> & op_ptr  = get_cached_operator(id);
  if (!op_ptr.non_nul()) {
    // Operator not in the cache ? Build it:
    if (id.name_.finit_par("_IJK")) {
      OperatorRegularize & op = op_ptr.instancie(OperatorRegularize);
      op.set_tolerance(opt_.regularize_tolerance);
      op.set_extend_layer(opt_.extend_domain);
    } else if (id.name_.finit_par("_dual")) {
      op_ptr.instancie(OperatorDualMesh);
    } else if (id.name_.finit_par("_Boundary")) {
      op_ptr.instancie(OperatorBoundary);
    } else if (id.name_.finit_par("_centerfaces")) {
      op_ptr.instancie(OperatorFacesMesh);
    } else {
      Journal() << "Internal error in LataFilter::get_operator: forgot to implement operator choice for " << id.name_ << endl;
      throw;
    }
  }
  return op_ptr.valeur();
}


// Description: returns the requested field, computing it if it is not
//  already in the cache. You MUST call release_field() on the returned field
//  when you don't need it any more...
//  See also class Field_Id
const LataField_base & LataFilter::get_field(const Field_Id & id)
{
  Journal(filter_info_level) << "LataFilter::get_field " 
                             << id.uname_ << " time=" << id.timestep_ 
                             << " bloc=" << id.block_ << endl;

  data_cache_.cleanup_cache(id.timestep_);

  const LataFieldMetaData & field_metadata = get_field_metadata(id.uname_);

  LataDeriv<LataField_base> & field_ptr = get_cached_field(id);
  if (!field_ptr.non_nul()) {
    if (field_metadata.source_ == "latadb") {
      // Request for a native field : load it from lataDB
      const Domain & dom = get_geometry(id);
      dom.fill_field_from_lataDB(lataDB(), id, field_ptr);
      release_geometry(dom);
    } else if (field_metadata.source_.debute_par("OPERATOR")) {
      const Field_Id src_id(field_metadata.source_field_,
                            id.timestep_,
                            id.block_);
      const Domain & src_domain = get_geometry(src_id);
      const LataField_base & src_field = get_field(src_id);
      const Domain & dest_domain = get_geometry(id);
      Operator & op = get_set_operator(dest_domain.id_);
      op.build_field(src_domain, src_field, dest_domain, field_ptr);
      field_ptr.valeur().id_ = Field_Id(field_metadata.uname_, src_field.id_.timestep_, src_field.id_.block_);
      release_field(src_field);
      release_geometry(src_domain);
      release_geometry(dest_domain);
      release_cached_operator(dest_domain.id_);
    } else if (field_metadata.source_ == "user_fields") {
      Field<FloatTab> & f = field_ptr.instancie(Field<FloatTab> );
      f = user_fields_.valeur().get_field(id);
      // Force field id to correct value:
      f.id_ = id;
      f.component_names_ = field_metadata.component_names_;
      f.nature_ = field_metadata.is_vector_ ? LataDBField::VECTOR : LataDBField::SCALAR;
      f.localisation_ = field_metadata.localisation_;
    }
  }

  return field_ptr.valeur();
}

// Description: returns the requested float field, computing it if it is not
//  already in the cache. You MUST call release_field() on the returned field
//  when you don't need it any more...
//  See also class Field_Id
const FieldFloat & LataFilter::get_float_field(const Field_Id & id) 
{
  const LataField_base & field = get_field(id);
  const FieldFloat * float_field_ptr = dynamic_cast<const FieldFloat*>(&field);
  if (! float_field_ptr) { assert(! float_field_ptr);abort();}
  const FieldFloat & fld = *float_field_ptr;
  return fld;
}
void LataFilter::release_geometry(const Domain & dom)
{
  Journal(filter_info_level) << "LataFilter::release_geometry " 
                             << dom.id_.name_ << " time=" << dom.id_.timestep_ 
                             << " bloc=" << dom.id_.block_ << endl;
  release_cached_domain(dom.id_);
}

void LataFilter::release_field(const LataField_base & field)
{
  Journal(filter_info_level) << "LataFilter::release_field " 
                             << field.id_.uname_ << " time=" << field.id_.timestep_ 
                             << " bloc=" << field.id_.block_ << endl;
  release_cached_field(field.id_);
}
  
void build_mangeld_domain_name(const Domain_Id & id, Nom & name)
{
  name = id.name_;
  name += "_";
  name += Nom(id.timestep_);
  name += "_";
  name += Nom(id.block_);
  name.majuscule();
}

void build_mangeld_field_name(const Field_Id & id, Nom & name)
{
  name = id.uname_.build_string();
  name += "_";
  name += Nom(id.timestep_);
  name += "_";
  name += Nom(id.block_);
  name.majuscule();
}

LataDeriv<LataField_base> & LataFilter::get_cached_field(const Field_Id& id)
{
  Nom n;
  build_mangeld_field_name(id, n);
  return data_cache_.get_item<LataDeriv<LataField_base> >(n, id.timestep_);
}
LataDeriv<Domain> &     LataFilter::get_cached_domain(const Domain_Id& id)
{
  Nom n;
  build_mangeld_domain_name(id, n);
  return data_cache_.get_item<LataDeriv<Domain> >(n, id.timestep_);
}
LataDeriv<Operator> &   LataFilter::get_cached_operator(const Domain_Id& id)
{
  Nom n;
  build_mangeld_domain_name(id, n);
  n += "_OP";
  return data_cache_.get_item<LataDeriv<Operator> >(n, id.timestep_);  
}
void LataFilter::release_cached_domain(const Domain_Id& id)
{
  Nom n;
  build_mangeld_domain_name(id, n);
  data_cache_.release_item(n);
}
void LataFilter::release_cached_field(const Field_Id& id)
{
  Nom n;
  build_mangeld_field_name(id, n);
  data_cache_.release_item(n);
}
void LataFilter::release_cached_operator(const Domain_Id& id)
{
  Nom n;
  build_mangeld_domain_name(id, n);
  n += "_OP";
  data_cache_.release_item(n);  
}

void LataOptions::extract_path_basename(const char * s, Nom & path_prefix, Nom & basename)
{
  int i;
  for (i=strlen(s)-1;i>=0;i--)
    if ((s[i]==PATH_SEPARATOR) ||(s[i]=='\\'))
      break;
  path_prefix = "";
  int j;
  for (j = 0; j <= i; j++)
    path_prefix += Nom(s[j]);
  
  // Parse basename : if extension given, remove it
  int n = strlen(s);
  if (n > 5 && strcmp(s+n-5,".lata") == 0)
    n -= 5;
  basename = "";
  for (j = i+1; j < n; j++)
    basename += Nom(s[j]);
  //  Journal(9)<<" prefix "<<path_prefix<< " "<<i<<endl;
}

LataOptions::LataOptions()
{
  dual_mesh = false;
  faces_mesh=false;
  nc_mesh = false;
  boundary_mesh = false;
  reconnect = false;
  reconnect_tolerance = 1e-6;
  regularize = false;
  extend_domain = 0;
  regularize_tolerance = 1e-6;
  invalidate = false;
  load_virtual_elements = false;
  user_fields_ = false;
  ijk_mesh_nb_parts_ = 1;
  ijk_virt_layer = 1;
  export_fields_at_faces_ = 0;
  regularize_polyedre=0;
}

void build_geometry_(Operator & op, const Domain & src, LataDeriv<Domain> & dest)
{
  Journal() << "Error in an operator: build_geometry not coded for this Operator/Domain" << endl;
  throw;
}

void build_field_(Operator & op, const Domain & src, const Domain & dest, 
                  const LataField_base & srcf, LataField_base & destf)
{
  Journal() << "Error in an operator: build_field not coded for this Operator/Domain/Field" << endl;
  throw;  
}

void LataDB_apply_input_filter(const LataDB & lata_db, LataDB & filtered_db, 
                        const ArrOfInt & input_timesteps_filter,
                        const Noms & input_domains_filter,
                        const Noms & input_components_filter)
{
  ArrOfInt timesteps_filter(input_timesteps_filter);
  Noms domains_filter(input_domains_filter);
  Noms components_filter(input_components_filter);

  // Build a list of all available geometries and components
  Noms list_all_domains = lata_db.geometry_names(lata_db.nb_timesteps()-1, LataDB::FIRST_AND_CURRENT);
  Noms list_all_fields;
  {
    Field_UNames fields = lata_db.field_unames(lata_db.nb_timesteps()-1, "*", "*", LataDB::FIRST_AND_CURRENT);
    for (entier i = 0; i < fields.size(); i++) {
      const Nom & n = fields[i].get_field_name();
      if (list_all_fields.rang(n) < 0)
        list_all_fields.add(n);
    }
  }

  if (timesteps_filter.size_array() == 0) {
    // Add all timesteps, timestep 0 is implicitely added.
    entier n = lata_db.nb_timesteps();
    timesteps_filter.resize_array(n-1);
    for (entier i = 1; i < n; i++)
      timesteps_filter[i-1] = i;
    Journal(3) << " Exporting all " << n-1 << " timesteps" << endl;
  } else if (timesteps_filter[0] < 0) {
    timesteps_filter.resize_array(0);
    Journal(3) << " Request timestep -1: Exporting only global time independent data" << endl;
  }
  if (domains_filter.size() == 0) {
    // Add all geometries
    domains_filter = list_all_domains;
    Journal(3) << " Exporting all geometries" << endl;
  } 
  if (components_filter.size() == 0) {
    // Add all fields of the selected geometries
    components_filter = list_all_fields;
    Journal(3) << " Exporting all fields:" << endl;
  } else {
    // Add all known geometry data fields
    components_filter.add("SOMMETS");
    components_filter.add("ELEMENTS");
    components_filter.add("FACES");
    components_filter.add("ELEM_FACES");
    components_filter.add("JOINTS_SOMMETS");
    components_filter.add("JOINTS_ELEMENTS");
    components_filter.add("JOINTS_FACES");
    components_filter.add("VIRTUAL_ELEMENTS");
    // these are for ijk meshs:
    components_filter.add("SOMMETS_IJK_I");
    components_filter.add("SOMMETS_IJK_J");
    components_filter.add("SOMMETS_IJK_K");
    components_filter.add("INVALID_CONNECTIONS");
  }
  filtered_db.filter_db(lata_db,
                        noms_to_motcles(domains_filter),
                        noms_to_motcles(components_filter),
                        timesteps_filter);
}
