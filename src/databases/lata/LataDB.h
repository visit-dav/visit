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

#ifndef LataDB_include_
#define LataDB_include_
#include <Lata_tools.h>
#include <sstream>

// This file describes the LataDB class and all associated data structures.
//  LataDB stores all the meta contained in the .lata master file, and not more.
//  It provides services to add meta-data, read data to a user specified array,
//  and write data to a lata file.

typedef BigEntier FileOffset;

// .Description: LataDBError is the type used for thrown exceptions in LataDBxxx classes
class LataDBError
{
public:
  // READ_ERROR: low level io error while reading .lata file
  // BAD_HEADER: the header in the .lata file is not correct
  // BAD_TIMESTEP: request for a non existant timestep
  // NAME_NOT_FOUND: request for a non existant domain or field name
  // DATA_ERROR: low level io error while reading a data bloc file
  // FILE_NOT_FOUND: a file (.lata or data) couldn't be opened on disc.
  // INVALID_OPERATION: trying to read from a modified database, etc...
  // INTEGER_OVERFLOW: trying to convert an integer to a too small data type
  //  (if error when reading a data file, you must recompile with typedef long long entier,
  //   if error when writing a data file, you must use INT64 type_ for data blocks)
  enum ErrType { READ_ERROR, BAD_HEADER, BAD_TIMESTEP, NAME_NOT_FOUND, DATA_ERROR,
                 FILE_NOT_FOUND, BAD_ELEM_TYPE, INVALID_OPERATION, INTEGER_OVERFLOW };
  LataDBError(ErrType err) : err_(err) {};
  ErrType err_;
  const char *describe() const;
};

// .Description: This is the data type for a specific part of a data bloc.
//  In order to read a data bloc, we need a LataDBDataType for the "bloc size" id (this is an integer),
//  and a LataDBDataType for the bloc content. LataDBGeometry blocs need two types, one for
//  the node coordinates and one for the elements
class LataDBDataType
{
public:
  enum MSB { UNKNOWN_MSB, MSB_BIG_ENDIAN, MSB_LITTLE_ENDIAN, ASCII };
  MSB msb_;
  enum Type { UNKNOWN_TYPE, INT32, INT64, REAL32, REAL64 };
  Type type_;
  // Array index is ignored if type_ is REAL.
  // NOT_AN_INDEX: array does not contain indexes.
  // C_INDEXING: If array contains indexes to other items, 0 <= array[i] < nb_items
  // F_INDEXING: 1 <= array[i] <= nb_items (Fortran index)
  // See LataDB::read_data
  enum ArrayIndex { UNKNOWN_ARRAYINDEX, NOT_AN_INDEX, C_INDEXING, F_INDEXING };
  ArrayIndex array_index_;
  // C_ORDERING: If multidimensionnal array is read, data ordering is like in C
  //               (all components for first node, then all components for second node, etc)
  // F_ORDERING: like in fortran (first all values for component 0 then all values for compo 1 etc)
  enum DataOrdering { UNKNOWN_ORDERING, C_ORDERING, F_ORDERING };
  DataOrdering data_ordering_;

  //   _NO_BLOC:         no fortran bloc marker
  //   _SINGLE_WRITE:    all data written in one fortran write instruction
  //   _MULTIPLE_WRITES: one fortran write instruction for each component
  enum FortranBlocMarkers { UNKNOWN_MARKERS, NO_BLOC_MARKER, BLOC_MARKERS_SINGLE_WRITE, BLOC_MARKERS_MULTIPLE_WRITES };
  FortranBlocMarkers fortran_bloc_markers_;

  // The data type for fortran bloc markers
  Type bloc_marker_type_;

  // Data is located at this offset in the file
  FileOffset file_offset_;

  LataDBDataType() : msb_(UNKNOWN_MSB), type_(UNKNOWN_TYPE), array_index_(UNKNOWN_ARRAYINDEX), 
                     data_ordering_(UNKNOWN_ORDERING), fortran_bloc_markers_(UNKNOWN_MARKERS), bloc_marker_type_(UNKNOWN_TYPE),
                     file_offset_(0)
  {};
  static MSB machine_msb_;
};

// .Description: Description of a geometry (= a mesh)
class LataDBGeometry
{
public:
  LataDBGeometry() { timestep_ = -1; }
  // Item name
  Nom name_;
  // Type of elements
  Motcle elem_type_;
  entier timestep_;
};

// This is a unique identifier for fields
//  at this time, contains domain name, field name and localisation,
//  might be further extended if needed
class Field_UName
{
public:
  Field_UName();
  Field_UName(const char *domain_name, const char *field_name, const char *loc);
  Field_UName(const Field_UName &);
  int operator==(const Field_UName &) const;
  Field_UName & operator=(const Field_UName &);
  Nom build_string() const;
  const Motcle & get_localisation() const { return loc_; }
  const Motcle & get_field_name() const { return field_name_; }
  const Motcle & get_geometry() const { return geometry_; }
  void        set_field_name(const Nom &);
protected:
  Motcle geometry_;
  Motcle field_name_;
  Motcle loc_;
};

std::ostream & operator<<(std::ostream &, const Field_UName &);

typedef LataVector<Field_UName> Field_UNames;
class EFichier;

// .Description: Description of a field
class LataDBField
{
public:
  LataDBField() { timestep_ = -1; nb_comp_ = -1; nature_ = UNKNOWN; size_ = -1; }

  // Unique identifier
  Field_UName uname_;
  // Field name (without localisation spec)
  Nom name_;
  // Where is it ?
  int timestep_;
  // Filename containing the data
  // Special names: memory_buffer_file() => data stored in the LataDB memory buffer.
  Nom filename_;
  // Number of components
  entier nb_comp_;
  // LataDBGeometry
  Nom geometry_;
  // Name of the components
  Noms component_names_;
  Noms unites_;
  // Scalar or vector ?
  enum Nature { UNKNOWN, SCALAR, VECTOR };
  Nature nature_;
  // Type and formatting info of the data
  LataDBDataType datatype_;
  // Localisation (elem, som, faces, ...)
  Motcle localisation_;
  // Ref
  Nom reference_;
  // Size (number of lines) 
  entier size_;
  
  static const char * memory_buffer_file();
};

// .Description: Description of one timestep (contains a vector of items)
class LataDBTimestep
{
public:
  LataDBTimestep() { time_ = -1.; }
  double time_;
protected:
  friend class LataDB;
  LataVector<LataDBGeometry> geoms_;
  LataVector<LataDBField> fields_;
};

class LataDataFile;
class ArrOfInt;
class LataDB
{
public:
  LataDB() : internal_data_buffer_(std::ios::in | std::ios::out | std::ios::app | std::ios::binary) { old_style_lata_ = 0; path_prefix_ = ""; write_master_file_to_call_ = 0; }
  LataDB(const LataDB & src) :
    header_(src.header_),
    case_(src.case_),
    software_id_(src.software_id_),
    default_type_int_(src.default_type_int_),
    default_float_type_(src.default_float_type_),
    timesteps_(src.timesteps_),    
    path_prefix_(src.path_prefix_),
    old_style_lata_(src.old_style_lata_),
    write_master_file_to_call_(src.write_master_file_to_call_) {
    // Note B.M. il faudrait copier internal_data_buffer_ pour faire marcher lml->lata mais je ne sais pas faire...
  }
  virtual ~LataDB();
  void reset();
  virtual void read_master_file(const char * path_prefix_, const char * filename);
  static Nom   read_master_file_options(const char * filename);
  
  virtual void filter_db(const LataDB & source,
                         const Motcles & geometry_names,
                         const Motcles & field_names,
                         const ArrOfInt & timesteps);

  entier                 nb_timesteps() const;
  double                 get_time(entier tstep) const;
  enum TStepSelector { CURRENT, FIRST_AND_CURRENT };
  Noms                   geometry_names(entier tstep, TStepSelector which_tstep = CURRENT) const;
  Field_UNames           field_unames(entier tstep, const char * geometry, const char * name, TStepSelector which_tstep = CURRENT) const;
  const LataDBGeometry & get_geometry(entier tstep, const char * name, TStepSelector which_tstep = CURRENT) const;
  entier                 field_exists(entier tstep, const char *geom, const char *name, TStepSelector which_tstep = CURRENT) const;
  const LataDBField &    get_field(entier tstep, const Field_UName & uname, TStepSelector which_tstep = CURRENT) const;
  const LataDBField &    get_field(entier tstep, const char *geom, const char *name, const char *loc, TStepSelector which_tstep = CURRENT) const;
  const Nom &            path_prefix() const { return path_prefix_; };
  void                   set_path_prefix(const char * s);

  // First line in the .lata file
  Nom header_;
  // Second line in the .lata file
  Nom case_;
  // Third line in the .lata file
  Nom software_id_;

  LataDBDataType default_type_float() const; // Everything same as int, but type_=default_float_type_
  LataDBDataType default_type_int_;
  LataDBDataType::Type default_float_type_;

  virtual void read_data(const LataDBField &, DoubleTab & data, entier debut = 0, entier n = -1) const;
  virtual void read_data(const LataDBField &, FloatTab & data, entier debut = 0, entier n = -1) const;
  virtual void read_data(const LataDBField &, IntTab & data, entier debut = 0, entier n = -1) const;
  virtual void read_data(const LataDBField &, DoubleTab & data, const ArrOfInt & lines_to_read) const;
  virtual void read_data(const LataDBField &, FloatTab & data, const ArrOfInt & lines_to_read) const;
  virtual void read_data(const LataDBField &, IntTab & data, const ArrOfInt & lines_to_read) const;

  enum Element { line, triangle, quadri, tetra, hexa, triangle_3D, quadri_3D, polyedre, unspecified };
  static Element element_type_from_string(const Motcle & type_elem);

  // Tools to create/update the database and write lata data to disk
  void   change_all_data_types(const LataDBDataType & old_type, const LataDBDataType & new_type);
  void   change_all_data_filenames(const Nom & old_prefix, const Nom & new_prefix);
  void   check_all_data_fileoffsets(entier split_files);
  entier add_timestep(double time);
  void   add_geometry(const LataDBGeometry & geom);
  void   set_elemtype(entier tstep, const char *geom_name, const char *elem_type);  
  entier check_duplicate_filename(const char *filename) const;
  void   add_field(const LataDBField & field);
  void   write_master_file(const char *filename) const;
  FileOffset write_data(entier tstep, const Field_UName &, const DoubleTab &);
  FileOffset write_data(entier tstep, const Field_UName &, const FloatTab &);
  FileOffset write_data(entier tstep, const Field_UName &, const IntTab &);

protected:
  LataDBField & getset_field(entier tstep, const Field_UName & uname, TStepSelector which_tstep = CURRENT);
  void          read_master_file_header(const char *filename, EFichier & is);
  static entier lata_v1_dim_from_elem_type(const Motcle & elem_type);
  static entier lata_v1_get_nb_comp(const Nom & fieldname, const Motcle & localisation, const LataDBGeometry & dom, entier dim, LataDBField::Nature & nature, LataDBDataType::DataOrdering &);
  static void get_element_data(const Motcle & elemtype, entier & dimension, entier & elem_shape, entier & face_shape, entier & nb_elem_faces);

  const LataDBTimestep & get_tstep(entier i) const;
  void add(entier tstep, const LataDBGeometry & item);
  void add(entier tstep, const LataDBField & item);
  template <class C_Tab> void read_data_(const LataDBField &, C_Tab & data, entier debut, entier n) const;
  template <class C_Tab> void read_data_(const LataDBField &, C_Tab & data, const ArrOfInt & lines_to_read) const;
  template <class C_Tab> void read_data2_(LataDataFile & f, const LataDBField & fld, C_Tab * const data, entier debut = 0, entier n = -1, const ArrOfInt *lines_to_read = 0) const;

  template <class C_Tab> FileOffset write_data_(entier tstep, const Field_UName & uname, const C_Tab &);

  // Timestep 0 contains global domains and field definition
  // Timestep 1..size()-1 contain the data for each "TEMPS" entry
  LataVector<LataDBTimestep> timesteps_;

  // Path prefix for all data blocks (used by read_data() and write_data())
  Nom path_prefix_;

  // Is this an old-style lata file ? (with INTERFACES special files and 2D elements expanded to 3D elements)
  entier old_style_lata_;

  // This flag tells if some write_data calls have been made since the last write_master_file
  // If yes, issue a message to say that's wrong !
  mutable entier write_master_file_to_call_;

  // This is a memory buffer where data can be written to create a temporary data base
  mutable std::stringstream internal_data_buffer_;
};
#endif
