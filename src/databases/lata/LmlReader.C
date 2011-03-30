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

#define BUFSZ 1000
#include <iostream>
#include <EFichier.h>
#include <LataDB.h>
#include <LataFilter.h>
#include <stdlib.h>
#include <string.h>
// lml files contain double precision values that can overflow or underflow
//  if converted to float. Check for overflow, ignore underflow
static inline float double_to_float(double x)
{
  // Written like this, the code will also stop on NAN values:
  if (!(x < 1.e38 && x > -1.e38)) {
    Journal() << "lml reader: Error converting double value " << x << " to float" << endl;
    throw LataDBError(LataDBError::READ_ERROR);
  }
  return (float) x;
}

// Reads the lml file, fills the lata_db and writes the data (coordinates, elements and
//  fields data) to a unique file data_filename.
// The default format used to write data in the data_filename is lata_db.default_type_*
// data_filename must not contain the path but only a filename with extension.
// The path to the data file must be set by lata_db.set_path_prefix() before.
// If data_filename is a null pointer, data files are not written and file offsets in lata_db will
//  be wrong (useful for just getting metadata)
void lml_reader(const char * lmlfilename, const char * data_filename, LataDB & lata_db)
{
  Nom filename_in_master_file;
  if (!data_filename)
    filename_in_master_file = "DATA_NOT_WRITTEN";
  else
    filename_in_master_file = data_filename;
  
  const entier lmllevel=4;
  EFichier is;
  Journal(lmllevel) << "lml_reader: " << endl;
  is.ouvrir(lmlfilename);
  if (!is.good()) {
    Journal() << "Error: cannot open lml file " << lmlfilename << endl;
    throw;
  }
  char s[BUFSZ+1];
  is.get_istream().getline(s, BUFSZ);
  if (!is.good()) {
    Journal() << "Lml file " << lmlfilename << " is empty" << endl;
    // Just put an empty initial timestep:
    lata_db.add_timestep(-1.);
    return;
  }
  lata_db.header_ = s;
  Journal(lmllevel) << "Header: " << s << endl;
  is.get_istream().getline(s, BUFSZ);
  lata_db.case_ = s;
  Journal(lmllevel) << "Case: " << s << endl;
  is.get_istream().getline(s, BUFSZ);
  lata_db.software_id_ = s;
  Journal(lmllevel) << "Software_id: " << s << endl;

  Noms liste_noms_geoms;
  Noms liste_noms_topo;
  // Create first timestep (global definitions)
  lata_db.add_timestep(-1.);
  // file_offset_blurb:
  // the file offset will be computed by LataDB::write_data(),
  // but we must tell write_data() if it must put the data at the beginning
  //  (file_offset==0) or append the data at the end of the file (file_offset!=0)
  // file_offset is 0 for the first data block and it is incremented for each block.
  entier file_offset = 0;
  LataDBField sommets;
  FloatTab nodes;
  while(1) {
    const entier tstep = lata_db.nb_timesteps() - 1;
    Motcle motlu;
    is >> motlu;
    if (!is.good()) break;
    if (motlu == "GRILLE") {
      LataDBGeometry geom;
      sommets.name_ = "SOMMETS";
      geom.timestep_ = sommets.timestep_ = tstep;
      sommets.filename_ = filename_in_master_file;
      Nom tmp;
      is >> tmp;
      geom.name_ = ((const char*)tmp)+7; // retire GRILLE_ du nom
      Journal(lmllevel) << "lml_reader: GRILLE " << geom.name_ << endl;
      is >> sommets.nb_comp_;
      is >> sommets.size_;
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      sommets.geometry_ = geom.name_;
      sommets.uname_ = Field_UName(sommets.geometry_, sommets.name_, "");
      sommets.datatype_ = lata_db.default_type_float();
      sommets.datatype_.file_offset_ = file_offset++; // see file_offset_blurb
      nodes.resize(sommets.size_, sommets.nb_comp_);
      for (entier i = 0; i < sommets.size_; i++)
        for (entier j = 0; j < sommets.nb_comp_; j++) {
          double x;
          is >> x;
          if (!is.good())
            throw LataDBError(LataDBError::READ_ERROR);
          nodes(i,j) = double_to_float(x);
        }
      Journal(lmllevel+1) << "Finished reading nodes" << endl;
      
      lata_db.add_geometry(geom);
      // Write nodes to disk later: in 2D they will be cropped
    } else if (motlu == "TOPOLOGIE") {
      LataDBField elements;
      elements.name_ = "ELEMENTS";
      elements.timestep_ = tstep;
      elements.filename_ = filename_in_master_file;
      elements.datatype_ = lata_db.default_type_int_;
      elements.datatype_.file_offset_ = file_offset++; // see file_offset_blurb
      Nom ident;
      is >> ident; // Topologie_MAILLAGE_VOLUMIQUE_XXX
      Nom tmp;
      is >> tmp;
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      elements.geometry_ = ((const char*)tmp)+7; // retire GRILLE_ du nom
      elements.uname_ = Field_UName(elements.geometry_, elements.name_, "");
      liste_noms_geoms.add(elements.geometry_);
      liste_noms_topo.add(ident);
      is >> motlu;
      if (motlu != "MAILLE") {
        Journal() << "Error reading TOPOLOGIE: expected MAILLE" << endl;
        throw;
      }
      is >> elements.size_;
      is >> motlu;
      int borne_index_min=0;
      if (motlu == "TETRA4") {
        lata_db.set_elemtype(tstep, elements.geometry_, "TETRAEDRE");
        elements.nb_comp_ = 4;
      } else if (motlu == "VOXEL8") {
        elements.nb_comp_ = 8;
        lata_db.set_elemtype(tstep, elements.geometry_, "HEXAEDRE");
      } else if (motlu == "PRISM6") {
        lata_db.set_elemtype(tstep, elements.geometry_, "PRISM6");
        elements.nb_comp_ = 6;
      } else if (motlu.debute_par("POLYEDRE_")) {
        lata_db.set_elemtype(tstep, elements.geometry_, motlu);
        elements.nb_comp_ = atoi(((const char *)motlu) + strlen("polyedre_"));
        borne_index_min=-1;
      } else {
        Journal() << "Error reading TOPOLOGIE: unknown element type" << endl;
        throw;
      }

      Journal(lmllevel+1) << " " << elements.size_ << " elements " << motlu << endl;
      IntTab elems;
      elems.resize(elements.size_, elements.nb_comp_);
      for (entier i = 0; i < elements.size_; i++) {
        if (i != 0) {
          is >> motlu; // element type
          if (!is.good())
            throw LataDBError(LataDBError::READ_ERROR);
        }
        entier j;
        for (j = 0; j < elements.nb_comp_; j++) {
          is >> elems(i,j);
          if (!is.good())
            throw LataDBError(LataDBError::READ_ERROR);
          elems(i,j)--;
          if (elems(i,j) < borne_index_min || elems(i,j) >= sommets.size_ ) {
            Journal() << "Error reading TOPOLOGIE: bad node number elem(" << i << "," << j << ")=" << elems(i,j) << endl;
            throw;
          }
        }
      }
      Journal(lmllevel+1) << " finished reading elements" << endl;
      lata_db.add_field(sommets);
      if (data_filename)
        lata_db.write_data(tstep, sommets.uname_, nodes);
      lata_db.add_field(elements);
      if (data_filename)
        lata_db.write_data(tstep, elements.uname_, elems);
    } else if (motlu == "FACE") {
      int n;
      is >> n;
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      Journal(lmllevel+1) << " faces " << n << endl;      
    } else if (motlu == "TEMPS") {
      double t;
      is >> t;
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      lata_db.add_timestep(t);
      Journal(lmllevel+1) << " new time: " << t << endl;      
    } else if (motlu == "CHAMPMAILLE" || motlu == "CHAMPPOINT") {
      LataDBField field;
      is >> field.name_;
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      Journal(lmllevel+1) << " new field: " << field.name_ << endl;      
      field.timestep_ = tstep;
      field.filename_ = filename_in_master_file;
      if (motlu == "CHAMPMAILLE")
        field.localisation_ = "ELEM";
      else
        field.localisation_ ="SOM";
      Nom nom_topo;
      is >> nom_topo;
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);

      const entier rang_topo = liste_noms_topo.rang(nom_topo);
      if (rang_topo < 0) {
        Journal() << "Error reading lml file : unknown topology name " << nom_topo << endl;
        throw;
      }
      field.geometry_ = liste_noms_geoms[rang_topo];
      Motcle tmp(field.name_);
      Motcle tmp2("_");
      tmp2 += field.localisation_;
      tmp2 += "_";
      tmp2 += field.geometry_;
      tmp.prefix(tmp2); // Retire _SOM_dom du nom
      field.name_ = tmp;
      field.uname_ = Field_UName(field.geometry_, field.name_, field.localisation_);
      double t;
      is >> t; // Unused time value
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      is >> motlu; // Repeat fieldname
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      is >> field.nb_comp_;
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      Nom unit;
      is >> unit;
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      field.unites_.add(unit);
      is >> motlu; // type0
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      is >> field.size_;
      if (!is.good())
        throw LataDBError(LataDBError::READ_ERROR);
      // By default, 3 components fields are vectors:
      if (field.nb_comp_ == 3) {
        Journal(lmllevel+1) << " 3 components=> say it's a vector" << endl;
        field.nature_ = LataDBField::VECTOR;
      } else {
        field.nature_ = LataDBField::SCALAR;
      }
      field.datatype_ = lata_db.default_type_float();
      field.datatype_.file_offset_ = file_offset++; // see file_offset_blurb
      FloatTab tab;
      tab.resize(field.size_, field.nb_comp_);
      for (entier i = 0; i < field.size_; i++) {
        entier n;
        is >> n;
        if (!is.good())
          throw LataDBError(LataDBError::READ_ERROR);
        for (entier j = 0; j < field.nb_comp_; j++) {
          double x;
          is >> x;
          if (!is.good())
            throw LataDBError(LataDBError::READ_ERROR);
          tab(i,j) = double_to_float(x);
        }
      }
      Journal(lmllevel+1) << " finished reading field " << field.name_ << endl;
      lata_db.add_field(field);
      if (data_filename)
        lata_db.write_data(tstep, field.uname_, tab);
    } else if (motlu == "FIN") {
      break;
    } else {
      Journal() << "Error reading lml file, unknown keyword " << motlu << endl;
      throw;
    }
  }
}

void lml_to_lata(const char *lmlname, const char *latafilename, 
                 entier ascii, entier fortran_blocs, entier fortran_ordering, entier fortran_indexing)
{
  const entier lmllevel=4;
  Journal(lmllevel) << "lml_to_lata " << lmlname << " -> " << latafilename << endl;
  LataDB lata_db;
  Nom dest_prefix, dest_name;
  LataOptions::extract_path_basename(latafilename, dest_prefix, dest_name);
  // Nom du fichier .data a ecrire (sans le chemin)
  Nom datafile(dest_name);
  datafile += ".lata.data";
  lata_db.set_path_prefix(dest_prefix);
  // Nom complet du fichier lml a lire
  LataDBDataType type;
  if (ascii)
    type.msb_ = LataDBDataType::ASCII;
  else
    type.msb_ = LataDBDataType::machine_msb_;
  type.type_ = LataDBDataType::INT32;
  type.array_index_ = fortran_indexing ? LataDBDataType::F_INDEXING : LataDBDataType::C_INDEXING;
  type.data_ordering_ = fortran_ordering ? LataDBDataType::F_ORDERING : LataDBDataType::C_ORDERING;
  type.fortran_bloc_markers_ = fortran_blocs ? LataDBDataType::BLOC_MARKERS_SINGLE_WRITE : LataDBDataType::NO_BLOC_MARKER;
  type.bloc_marker_type_ = LataDBDataType::INT32;
  type.file_offset_ = 0;
  lata_db.default_type_int_ = type;
  lata_db.default_float_type_ =  LataDBDataType::REAL32;
  
  lml_reader(lmlname, datafile, lata_db);
  Journal(lmllevel) << "lml_to_lata writing lata master file" << endl;
  lata_db.write_master_file(latafilename);
}

// Reads lml or lata file into lata_db. lml data is loaded in an internal memory buffer
//  file: full name with path
//  path_prefix: the path (used to access lata data files)
// If dest_file_if_lml is not null, puts lml data into this file...
// In this case, you must set lata_db.default_type* to tell which format to use.
void read_any_format(const char * file, const Nom & path_prefix, LataDB & lata_db)
{
  // Is it an lml ?
  Motcle motcle_nom_fic(file);
  if (motcle_nom_fic.finit_par(".lml")) {
    Journal(1) << "Detected lml file : " << file << endl;
    // Nom complet du fichier lml a lire
    Journal(1) << "Reading lml file to memory buffer" << endl;
    // data will be put in an internal memory buffer.
    // choose appropriate data format:
    LataDBDataType type;
    type.msb_ = LataDBDataType::machine_msb_;
    type.type_ = LataDBDataType::INT32;
    type.array_index_ = LataDBDataType::C_INDEXING;
    type.data_ordering_ = LataDBDataType::C_ORDERING;
    type.fortran_bloc_markers_ = LataDBDataType::NO_BLOC_MARKER;
    type.bloc_marker_type_ = LataDBDataType::INT32;
    type.file_offset_ = 0;
    lata_db.default_type_int_ = type;
    lata_db.default_float_type_ =  LataDBDataType::REAL32;
    lml_reader(file, LataDBField::memory_buffer_file(), lata_db);
  } else {
    Journal(1) << "Detected lata file : " << file << endl;
    lata_db.read_master_file(path_prefix, file);
  }
}

// Description: if the file is a lata file, read the third line and interprets it as options
//  if lml format, do nothing
//  otherwise, error.
void read_any_format_options(const char * file, LataOptions & opt)
{
  Motcle nom_fic(file);
  if (nom_fic.finit_par(".lml")) {
    // do nothing
  } else if (nom_fic.finit_par(".lata")) {
    Journal(1) << "Lata file: Interpreting LataFilter options on third line" << endl;
    Nom ligne = LataDB::read_master_file_options(file);
    const char *s = ligne;
    while (*s) {
      Nom toto("");
      while ((*s) != ' ' && (*s) != 0) {
        toto += Nom(*s);
        s++;
      }
      if (toto != "Trio_U") {
        if (!opt.parse_option(toto)) {
          Journal(0) << "Interpreting option: " << toto <<"  Failed." << endl;
          throw LataDBError::BAD_HEADER;
        } else
          Journal(1) << "Interpreting option: " << toto <<"  Success." << endl;
      }
      while ((*s) == ' ')
        s++;
    }
  } else {
    Journal(0) << "read_any_format_options: file " << nom_fic << " has unsupported extension" << endl;
    throw LataDBError::BAD_HEADER;
  }
}
