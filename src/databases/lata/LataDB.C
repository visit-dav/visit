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

#include <errno.h>
#include <LataDB.h>
#include <stdio.h>
#include <EFichier.h>
#include <LataV1_field_definitions.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
// Verbose level for which main lata file interpretation should be printed:
//  Dump one line for the whole file at verb_level-1
//  Dump one line for each Geometry, Temps, or Champ at verb_level
//  Detailed Geometry and Champ metadata is printed out at verb_level+1
#define verb_level 4
// Verbose level for data bloc reading:
//  Dump one line for each read data bloc at this level
//  Dump detailed subbloc interpretation at level+1
#define verb_level_data_bloc 5

typedef int LataDBInt32;
typedef long long int LataDBInt64;

void arch_check()
{
  if (sizeof(LataDBInt32)!=4)
    Journal() << "Error in LataDB arch_check : wrong LataDBInt32" << endl;
  if (sizeof(LataDBInt64)!=8)
    Journal() << "Error in LataDB arch_check : wrong LataDBInt64" << endl;
}

const char * LataDBField::memory_buffer_file()
{
  return "MEMORY_BUFFERED_DATA";
}

Field_UName::Field_UName()
{
}

Field_UName::Field_UName(const Field_UName & f) :
  geometry_(f.geometry_), field_name_(f.field_name_), loc_(f.loc_)
{
}

Field_UName::Field_UName(const char *domain_name, const char *field_name, const char *loc) :
  geometry_(domain_name), field_name_(field_name), loc_(loc)
{
  if (loc_ == "??")
    loc_ = "";
}

Field_UName & Field_UName::operator=(const Field_UName & f)
{
  geometry_ = f.geometry_;
  field_name_ = f.field_name_;
  loc_ = f.loc_;
  return *this;
}

Nom Field_UName::build_string() const
{
  Nom n(geometry_);
  n += "_";
  n += field_name_;
  if (loc_ != "" && loc_ != "??") {
    n += "_";
    n += loc_;
  }
  return n;
}

void Field_UName::set_field_name(const Nom & n)
{
  field_name_ = n;
}

int Field_UName::operator==(const Field_UName & f) const
{
  return (geometry_ == f.geometry_) && (field_name_ == f.field_name_) && (loc_ == f.loc_);
}

std::ostream & operator<<(std::ostream & os, const Field_UName & uname)
{
  os << uname.build_string();
  return os;
}

// This is a duplicate of Domain... only used for old latav1 compatibility
// (otherwise, LataDB should not have to know about element types !)
LataDB::Element LataDB::element_type_from_string(const Motcle & type_elem)
{
  Element type;
  if (type_elem == "HEXAEDRE")
    type=hexa;
  else if (type_elem == "HEXAEDRE_AXI")
    type=hexa;
  else if (type_elem == "HEXAEDRE_VEF")
    type=hexa;
  else if (type_elem == "QUADRANGLE")
    type=quadri;
  else if (type_elem == "QUADRANGLE_3D")
    type=quadri;
  else if (type_elem == "RECTANGLE")
    type=quadri;
  else if (type_elem == "RECTANGLE_2D_AXI")
    type=quadri;
  else if (type_elem == "RECTANGLE_AXI")
    type=quadri;
  else if (type_elem == "SEGMENT")
    type=line;
  else if (type_elem == "SEGMENT_2D")
    type=line;
  else if (type_elem == "TETRAEDRE")
    type=tetra;
  else if (type_elem == "TRIANGLE")
    type=triangle;
  else if (type_elem == "TRIANGLE_3D")
    type=triangle;
  else if (type_elem.debute_par("POLYEDRE"))
    type=polyedre;
  else {
    Journal() << "Error in elem_type_from_string: unknown element type " << type_elem << endl;
    throw(LataDBError(LataDBError::BAD_ELEM_TYPE));
  }
  return type;
}

void LataDB::get_element_data(const Motcle & elemtype, entier & dimension, entier & elem_shape, entier & face_shape, entier & nb_elem_faces)
{
  Element elem = element_type_from_string(elemtype);
  switch(elem) {
  case line:        dimension = 2; elem_shape=2; face_shape=1; nb_elem_faces=2; break;
  case triangle:    dimension = (elemtype=="TRIANGLE") ? 2 : 3; 
    elem_shape=3; face_shape=2; nb_elem_faces=3; break;
  case quadri:      dimension = (elemtype=="QUADRANGLE_3D") ? 3 : 2;
    elem_shape=4; face_shape=2; nb_elem_faces=4; break;
  case tetra:       dimension = 3; elem_shape=4; face_shape=3; nb_elem_faces=4; break;
  case hexa:        dimension = 3; elem_shape=8; face_shape=4; nb_elem_faces=6; break;
  default:
    Journal() << "LataDB::get_element_data element is unspecified" << endl;
    throw(LataDBError(LataDBError::BAD_ELEM_TYPE));
  }    
}

// Description: in lata v1 format, the number of components in a Champ entry
//  is implicitely defined by the field name and the discretisation
entier LataDB::lata_v1_get_nb_comp(const Nom & fieldname, const Motcle & localisation, 
                                   const LataDBGeometry & dom, entier dim, LataDBField::Nature & nature,
                                   LataDBDataType::DataOrdering & ordering)
{
  // Search component name in std_components
  entier nb_comp = latav1_component_shape(fieldname);
  Motcle maj_field(fieldname);
  ordering = LataDBDataType::C_ORDERING;
  nature = LataDBField::SCALAR;
  Journal(verb_level+1) << " LataV2 known component name found : " << fieldname << endl;
  if (nb_comp == -1) {
    // This is a vector component. If it's not a VDF faces, nb_comp = dimension of the problem
    Element elt = element_type_from_string(dom.elem_type_);
    if (localisation == "FACES" && (elt == quadri || elt == hexa)) {
      nb_comp = 1;
      Journal(verb_level+1) << " Vector field. Detected a VDF faces discretisation => nb_comp=1" << endl;
    } else {
      nb_comp = dim;
      nature = LataDBField::VECTOR;
      ordering = LataDBDataType::F_ORDERING;
      Journal(verb_level+1) << " Vector field. nb_comp = dimension = " << nb_comp << endl;
      Journal(verb_level+1) << " Assume fortran ordering" << endl;
    }
  } else if (nb_comp == -2) {
    // This is the vorticity: scalar in 2D, vector in 3D
    if (dim == 2)
    {
      nb_comp = 1;
      Journal(verb_level+1) << " Scalar field, nb_comp=" << nb_comp << endl; 
    }
    else 
    {
      nb_comp = dim;
      nature = LataDBField::VECTOR;
      ordering = LataDBDataType::F_ORDERING;
      Journal(verb_level+1) << " Vector field. nb_comp = dimension = " << nb_comp << endl;
      Journal(verb_level+1) << " Assume fortran ordering" << endl;      
    }
  } else {
    //if (maj_field == "K_EPS") {
    if (nb_comp>1) {
      ordering = LataDBDataType::F_ORDERING;
      Journal(verb_level+1) << " Special K_EPS => Assume fortran ordering" << endl;
    }
    Journal(verb_level+1) << " Scalar field, nb_comp=" << nb_comp << endl; 
  }
  return nb_comp;
}

// Description: in lata v1 format, the localisation is implicitely defined by the
//  file name of the data file:
void lata_v1_get_localisation(const char * filename, Nom & localisation_)
{
  if (strstr(filename, ".SOM."))
    localisation_ = "SOM";
  else if (strstr(filename, ".ELEM."))
    localisation_ = "ELEM";
  else if (strstr(filename, ".FACES."))
    localisation_ = "FACES";
  else {
    Journal() << "Error in lata_v1_get_localisation. Unable to find localisation in filename\n"
              << filename << endl;
    throw(LataDBError(LataDBError::READ_ERROR));
  }
}

class LataDataFile
{
public:
  enum Mode { READ, WRITE, APPEND };
  LataDataFile(std::iostream & mem_buffer, const char *prefix, const char *name, const LataDBDataType::MSB& msb, Mode mode = READ)
  {
    exception_ = 1;
    msb_=msb;
    if (strcmp(name, LataDBField::memory_buffer_file()) == 0) {
      stream_ = &mem_buffer;
      fname_ = name;
      Journal(verb_level_data_bloc) << "LataDB: opening internal memory_buffer for read/write" << endl;
      if (mode == READ)
        (*stream_).seekg(0, std::ios::beg);
      else if (mode == WRITE) 
        (*stream_).seekp(0, std::ios::beg);
      else
        (*stream_).seekp(0, std::ios::end);
      Journal(verb_level_data_bloc+1) << " current position: " << position() << endl;
      if (!fstream_.good()) {
        Journal() << "LataDataFile: Memory stream error" << endl;
        throw LataDBError(LataDBError::DATA_ERROR);
      }
    } else {
      stream_ = &fstream_;
      fname_ = prefix;;
      fname_ += name;
      Journal(verb_level_data_bloc) << "LataDB: opening data file " << fname_ << endl;
      if (msb_ == LataDBDataType::ASCII) {
      switch(mode) {
      case READ: fstream_.open(fname_, std::fstream::in); break;
      case WRITE: fstream_.open(fname_, std::fstream::out); break;
      case APPEND: fstream_.open(fname_, std::fstream::out | std::fstream::app); break;
      }
      }
      else 
        {
          if (msb_ != LataDBDataType::machine_msb_) {
            Journal() << "LataDB LataDataFile::write(int) not coded for reverse binary msb" << endl;
            throw;
          }
          switch(mode) {
          case READ: fstream_.open(fname_, std::fstream::in|std::fstream::binary); break;
          case WRITE: fstream_.open(fname_, std::fstream::out|std::fstream::binary); break;
          case APPEND: fstream_.open(fname_, std::fstream::out | std::fstream::app|std::fstream::binary); break;
          }
        }
      if (!fstream_.good()) {
        Journal() << "File not found " << fname_ << endl;
        throw LataDBError(LataDBError::FILE_NOT_FOUND);
      }
    }
  }
  void set_exception(int i) { exception_ = i; }
  FileOffset position() { return (*stream_).tellp(); };
  enum SeekType { ABSOLUTE, RELATIVE };
  void seek(FileOffset pos, SeekType seekt) {
    Journal(verb_level_data_bloc+1) << "Seeking file " << fname_ 
                                    << ((seekt == ABSOLUTE) ? " absolute position " : " relative position ")
                                    << pos << endl;
    if (seekt == ABSOLUTE)
      (*stream_).seekg(pos, std::ios::beg);
    else
      (*stream_).seekg(pos, std::ios::cur);
  
    if (exception_ && !(*stream_).good()) {
      Journal() << "Error seeking to position " << pos << " in file " << fname_ << endl;
      throw LataDBError(LataDBError::DATA_ERROR);
    }
  }
  void set_encoding(LataDBDataType::MSB msb, LataDBDataType::Type type) { msb_ = msb; type_ = type; };
  void set_err_message(const char *message) { message_ = message; };
  LataDataFile & operator>>(LataDBInt32 & x) { read(&x, 1); return *this; };
  LataDataFile & operator>>(float & x) { read(&x, 1); return *this; };
  LataDataFile & operator>>(Nom & n) {
    n = "";
    if (msb_ != LataDBDataType::ASCII) {
      for (;;) {
        char c[2];
        c[1] = 0;
        (*stream_).get(c[0]);

        if (!(*stream_).good())
          break;

        if (c[0] == '\0')
          break;

        n += c;
      }
    } else {
      std::string tmp;
      (*stream_) >> tmp;
      if ((*stream_).good())
        n = tmp.c_str();
    }
    if (exception_ && !(*stream_).good()) {
      Journal() << "Error reading string in file " << fname_ << endl;
      throw LataDBError(LataDBError::DATA_ERROR);
    }
    return *this;
  }
  void read(LataDBInt32 *ptr, BigEntier n);
  void read(float *ptr, BigEntier n);
  LataDataFile & operator<<(LataDBInt32 & x) { write(&x, 1, 1); return *this; };
  LataDataFile & operator<<(float & x) { write(&x, 1, 1); return *this; };
  void write(const LataDBInt32 *ptr, BigEntier n, BigEntier col);
  void write(const float *ptr, BigEntier n, BigEntier col);

protected:
  Nom fname_;
  const char * message_; // Message printed if error.
  std::fstream fstream_;
  std::iostream *stream_; // Points to fstream_ or mem_buffer passed to constructor
  LataDBDataType::MSB msb_;
  LataDBDataType::Type type_;
  int exception_;
};

void LataDataFile::read(LataDBInt32 *ptr, BigEntier n)
{
  if (type_ != LataDBDataType::INT32 && type_ != LataDBDataType::INT64) {
    Journal() << "Error in lataDB bloc read: trying to read non integer data into integer array" << endl;
    throw LataDBError(LataDBError::DATA_ERROR); 
  }
  if (msb_ == LataDBDataType::ASCII) {
    BigEntier i;
    if (ptr)
      Journal(verb_level_data_bloc+1) << "Reading ascii int data bloc size=" << n << endl;
    else
      Journal(verb_level_data_bloc+1) << "Skipping ascii int data bloc size=" << n << endl;
    LataDBInt32 toto;
    for (i = 0; i < n; i++) {
      if (ptr)
        (*stream_) >> ptr[i];
      else
        (*stream_) >> toto;
      if (exception_ && !(*stream_).good()) {
        Journal() << "Error reading ascii file " << fname_ << " LataDBInt32[" << n << "] at index " 
                  << i << endl << (message_?message_:"") << endl;
        throw LataDBError(LataDBError::DATA_ERROR);
      }
    }
  } else {
    if (type_ != LataDBDataType::INT32) {
      Journal() << "Internal error in LataDB.cpp LataDataFile::read(LataDBInt32) : size conversion not coded" << endl;
      throw;
    }
    if (ptr) {
      Journal(verb_level_data_bloc+1) << "Reading binary int data bloc size=" << n << endl;
      (*stream_).read((char*)ptr, n * sizeof(LataDBInt32));
    } else {
      Journal(verb_level_data_bloc+1) << "Skipping binary int data bloc size=" << n << endl;
      seek(n * sizeof(LataDBInt32), RELATIVE);
    }
    if (exception_ && !(*stream_).good()) {
      Journal() << "Error reading binary file " << fname_ << " LataDBInt32[" << n << "]" 
                << endl << (message_?message_:"") << endl;
      throw LataDBError(LataDBError::DATA_ERROR);
    }
    if (msb_ != LataDBDataType::machine_msb_) {
      Journal() << "LataDB LataDataFile::read(LataDBInt32) not coded for reverse binary msb" << endl;
      throw;
      // Put code here (and test !) to reverse bytes in the binary bloc:
    }
  } 
}

void LataDataFile::read(float *ptr, BigEntier n)
{
  if (type_ != LataDBDataType::REAL32) {
    Journal() << "Error in lataDB bloc read: trying to read non float data into float array" << endl;
    throw LataDBError(LataDBError::DATA_ERROR); 
  }
  if (msb_ == LataDBDataType::ASCII) {
    BigEntier i;
    if (ptr)
      Journal(verb_level_data_bloc+1) << "Reading ascii float data bloc size=" << n << endl;
    else
      Journal(verb_level_data_bloc+1) << "Skipping ascii float data bloc size=" << n << endl;
    float toto;
    for (i = 0; i < n; i++) {
      if (ptr)
        (*stream_) >> ptr[i];
      else
        (*stream_) >> toto;
      if (exception_ && !(*stream_).good()) {
        Journal() << "Error reading ascii file " << fname_ << " float[" << n << "] at index " 
                  << i << endl << message_ << endl;
        throw LataDBError(LataDBError::DATA_ERROR);
      }
    }
  } else {
    if (ptr) {
      Journal(verb_level_data_bloc+1) << "Reading binary float data bloc size=" << n << endl;
      (*stream_).read((char*)ptr, n * sizeof(float));
    } else {
      Journal(verb_level_data_bloc+1) << "Skipping binary float data bloc size=" << n << endl;
      seek(n * sizeof(float), RELATIVE);
    }
    if (exception_ && !(*stream_).good()) {
      Journal() << "Error reading binary file " << fname_ << " float[" << n << "]" 
                << endl << message_ << endl;
      throw LataDBError(LataDBError::DATA_ERROR);
    }
    if (msb_ != LataDBDataType::machine_msb_) {
      Journal() << "LataDB LataDataFile::read(float) not coded for reverse binary msb" << endl;
      throw;
      // Put code here (and test !) to reverse bytes in the binary bloc:
    }
  } 
}

void LataDataFile::write(const LataDBInt32 *ptr, BigEntier n, BigEntier columns)
{
  Journal(verb_level_data_bloc+1) << "Writing int data bloc size=" << n << endl;
  if (type_ != LataDBDataType::INT32) {
    Journal() << "Error in lataDB bloc write: trying to write integer data to non integer file block" << endl;
    throw LataDBError(LataDBError::DATA_ERROR); 
  }
  if (msb_ == LataDBDataType::ASCII) {
    for (BigEntier i = 0; i < n; i+=columns) {
      BigEntier j;
      for (j = 0; j < columns-1; j++)
        (*stream_) << ptr[i+j] << " ";
      (*stream_) << ptr[i+j] << endl;
    }
  } else {
    if (msb_ != LataDBDataType::machine_msb_) {
      Journal() << "LataDB LataDataFile::write(int) not coded for reverse binary msb" << endl;
      throw;
      // Put code here (and test !) to reverse bytes in the binary bloc:
    }
    (*stream_).write((char*)ptr, n * sizeof(LataDBInt32));    
  }
  (*stream_).seekg(0, std::ios::end);
  if (exception_ && !(*stream_).good()) {
    Journal() << "Error writing file " << fname_ << " int[" << n << "]" 
              << endl << message_ << endl;
    throw LataDBError(LataDBError::DATA_ERROR);
  }
}

void LataDataFile::write(const float *ptr, BigEntier n, BigEntier columns)
{
  Journal(verb_level_data_bloc+1) << "Writing float data bloc size=" << n << endl;
  if (type_ != LataDBDataType::REAL32) {
    Journal() << "Error in lataDB bloc write: trying to write float data to non float file block" << endl;
    throw LataDBError(LataDBError::DATA_ERROR); 
  }
  if (msb_ == LataDBDataType::ASCII) {
    for (BigEntier i = 0; i < n; i+=columns) {
      BigEntier j;
      for (j = 0; j < columns-1; j++)
        (*stream_) << ptr[i+j] << " ";
      (*stream_) << ptr[i+j] << endl;
    }
  } else {
    if (msb_ != LataDBDataType::machine_msb_) {
      Journal() << "LataDB LataDataFile::write(float) not coded for reverse binary msb" << endl;
      throw;
      // Put code here (and test !) to reverse bytes in the binary bloc:
    }
    (*stream_).write((char*)ptr, n * sizeof(float));    
  }
  (*stream_).seekg(0, std::ios::end);
  if (exception_ && !(*stream_).good()) {
    Journal() << "Error writing file " << fname_ << " float[" << n << "]" 
              << endl << message_ << endl;
    throw LataDBError(LataDBError::DATA_ERROR);
  }
}

// Description: skips a fortran bloc size descriptor.
void skip_blocksize(LataDataFile & f, const LataDBDataType & type)
{
  if (type.fortran_bloc_markers_ == LataDBDataType::NO_BLOC_MARKER)
    return;
  f.set_err_message("Error reading fortran blocsize");
  f.set_encoding(type.msb_, type.bloc_marker_type_);
  int i;
  f >> i;
  Journal(verb_level_data_bloc+1) << "Skipping blocsize marker value=" << i << endl;
}

template<class DEST_TYPE>
DEST_TYPE int_conversion(LataDBInt64 x, const char * err_msg = 0)
{
  DEST_TYPE result = (DEST_TYPE) x;
  if ((LataDBInt64) result != x) {
    if (err_msg)
      Journal() << "LataDB integer conversion failed: " << err_msg << endl;
    else 
      Journal() << "LataDB integer conversion failed: " << endl;
    throw(LataDBError(LataDBError::INTEGER_OVERFLOW));
  }
  return result;
}

void write_blocksize(LataDataFile & f, const LataDBDataType & type, entier sz)
{
  if (type.fortran_bloc_markers_ == LataDBDataType::NO_BLOC_MARKER)
    return;

  Journal(verb_level_data_bloc+1) << "Writing blocsize marker value=" << sz << endl;
  f.set_err_message("Error writing fortran blocsize");
  f.set_encoding(type.msb_, type.bloc_marker_type_);
  f << sz;
}

void bloc_read_skip(LataDataFile & f, LataDBDataType::MSB msb, LataDBDataType::Type type, BigEntier size)
{
  f.set_encoding(msb, type);
  switch(type) {
  case LataDBDataType::INT32: f.read((LataDBInt32*) 0, size); break;
  case LataDBDataType::REAL32: f.read((float*) 0, size); break;
  default:
    Journal() << "Internal error: bloc read skip not code for this type" << endl;
//    exit(-1);
  }
}

// Description: Read "tab.size_array()" values from file "f" at current file location
//  into the "tab" array. "msb" and "type" must match the data type written in the file.
void bloc_read(LataDataFile & f, LataDBDataType::MSB msb, LataDBDataType::Type type,
               ArrOfInt & tab)
{
  f.set_encoding(msb, type);
  f.read(tab.addr(), tab.size_array());
}

void bloc_read(LataDataFile & f, LataDBDataType::MSB msb, LataDBDataType::Type type,
               ArrOfFloat & tab)
{
  f.set_encoding(msb, type);
  f.read(tab.addr(), tab.size_array());
}

void bloc_write(LataDataFile & f, LataDBDataType::MSB msb, LataDBDataType::Type type,
                const ArrOfInt & tab, int columns)
{
  f.set_encoding(msb, type);
  f.write(tab.addr(), tab.size_array(), columns);
}

void bloc_write(LataDataFile & f, LataDBDataType::MSB msb, LataDBDataType::Type type,
                const ArrOfFloat & tab, int columns)
{
  f.set_encoding(msb, type);
  f.write(tab.addr(), tab.size_array(), columns);
}

LataDBDataType::MSB LataDBDataType::machine_msb_ = (mymachine_msb) ? LataDBDataType::MSB_BIG_ENDIAN : LataDBDataType::MSB_LITTLE_ENDIAN;

void LataDB::add(entier tstep, const LataDBGeometry & item)
{
  Noms names = geometry_names(tstep);
  if (names.rang(item.name_) >= 0) {
    Journal() << "Error in LataDBTimestep::add(const LataDBGeometry &): duplicate geometry name " << item.name_ << endl;
    throw(LataDBError(LataDBError::READ_ERROR));
  }
  timesteps_[tstep].geoms_.add(item);
}

void LataDB::add(entier tstep, const LataDBField & item)
{
  LataDBField & field = timesteps_[tstep].fields_.add(item);
  field.timestep_ = tstep;
  field.uname_ = Field_UName(item.geometry_, item.name_, item.localisation_);
  Journal(verb_level+1) << "LataDB::add " << tstep << " " << field.uname_ << endl;
}

// Description: returns the number of timesteps in the database
//  (timestep 0 contains geometries and fields defined before the first TEMPS entry,
//   hence nb_timesteps() == number of TEMPS entries plus 1)
// Exceptions: BAD_TIMESTEP
entier LataDB::nb_timesteps() const
{ 
  return timesteps_.size(); 
}

// Description: returns the physical time for this timestep
// Exceptions: BAD_TIMESTEP
double LataDB::get_time(entier tstep) const
{
  return get_tstep(tstep).time_;
}

// Description: returns the requested geometry in the requested timestep
//  "where" tells where to seach this geometry (in the current timestep or
//  also in the first timestep.
// Exceptions: BAD_TIMESTEP NAME_NOT_FOUND
const LataDBGeometry & LataDB::get_geometry(entier tstep, const char* name,
                                            TStepSelector where) const
{
  if (!name)
    throw(LataDBError(LataDBError::NAME_NOT_FOUND));
  while (1) {
    const LataDBTimestep & t = get_tstep(tstep);
    const entier n = t.geoms_.size();
    for (entier i = 0; i < n; i++) {
      const LataDBGeometry & geom = t.geoms_[i];
      if (geom.name_ == name)
        return geom;
    }
    if (where == FIRST_AND_CURRENT && tstep > 0)
      tstep = 0;
    else
      break;
  }
  throw(LataDBError(LataDBError::NAME_NOT_FOUND));
}

// Description: returns the requested field in the requested timestep.
// Exceptions: BAD_TIMESTEP NAME_NOT_FOUND
const LataDBField & LataDB::get_field(entier tstep, const Field_UName & uname,
                                      TStepSelector where) const
{
  while (1) {
    const LataDBTimestep & t = get_tstep(tstep);
    const entier n = t.fields_.size();
    for (entier i = 0; i < n; i++) {
      const LataDBField & field = t.fields_[i];
      if (field.uname_ == uname)
        return field;
    }
    if (where == FIRST_AND_CURRENT && tstep > 0)
      tstep = 0;
    else
      break;
  }
  throw(LataDBError(LataDBError::NAME_NOT_FOUND));
}

// Description: shortcut, works only if the specified field exists and is unique.
const LataDBField & LataDB::get_field(entier tstep, const char *geom, const char *name, const char *loc,
                                      TStepSelector which_tstep) const
{
  Field_UNames fields = field_unames(tstep, geom, name, which_tstep);
  if (fields.size() > 1)
    cerr << "get_field(char *geom, char *name, ...) returned more than one field !" << endl;
  if (fields.size() != 1)
    throw(LataDBError(LataDBError::NAME_NOT_FOUND));
  return get_field(tstep, fields[0], which_tstep);
}

// Description: return 1 if the field exists AND is unique. (means you can call get_field with the
//  same parameters)
entier LataDB::field_exists(entier tstep, const char *geom, const char *name, 
                            TStepSelector which_tstep) const
{
  Field_UNames fields = field_unames(tstep, geom, name, which_tstep);
  return fields.size() == 1;
}


LataDBField & LataDB::getset_field(entier tstep, const Field_UName & uname, TStepSelector which_tstep)
{
  return (LataDBField&) get_field(tstep, uname, which_tstep);
}

// Description: returns the names of all geometries defined in the timestep
//  which_tstep tell where to search for geometries.
// Exceptions: BAD_TIMESTEP 
Noms LataDB::geometry_names(entier tstep, TStepSelector which_tstep) const
{
  Noms names;
  const LataDBTimestep & t = get_tstep(tstep);
  entier n = t.geoms_.size();
  for (entier i = 0; i < n; i++)
    names.add(t.geoms_[i].name_);
  if (which_tstep == FIRST_AND_CURRENT && tstep > 0) {
    const LataDBTimestep & t = get_tstep(0);
    entier n = t.geoms_.size();
    for (entier i = 0; i < n; i++)
      // add if not:
      if (names.rang(t.geoms_[i].name_) < 0)
        names.add(t.geoms_[i].name_);
  }
  return names;
}

// Description: returns the unique_identifiers of all fields defined in the timestep and for which
//  the associated geometry is "geometry" and the name is "name". Some fields may have no associated geometry,
//  give a null pointer or empty string to get these fields.
//  If geometry="*", returns list for all geometries
//  If name="*", returns list for all field names
// Exceptions: BAD_TIMESTEP 
LataVector<Field_UName> LataDB::field_unames(entier tstep, const char * geometry, const char * name, TStepSelector which_tstep) const
{
  LataVector<Field_UName> unames;
  if (!geometry)
    geometry = "";
  for (;;) {
    const LataDBTimestep & t = get_tstep(tstep);
    entier n = t.fields_.size();
    for (entier i = 0; i < n; i++) {
      const LataDBField & field = t.fields_[i];
      if ((field.geometry_ == geometry || strcmp(geometry, "*")==0 )
          && (field.name_ == name || strcmp(name,"*")==0 ))
        unames.add(field.uname_);
    }
    if (tstep == 0 || which_tstep != FIRST_AND_CURRENT)
      break;
    tstep = 0;
  }
  return unames;
}

void check(Entree & is, const char * msg)
{
  if (!is.good()) {
    Journal() << "LataDB::read_master_file " << msg << endl;
    throw(LataDBError(LataDBError::READ_ERROR));
  }
}
void read_keyword_nom(Entree & is, Nom& motlu)
{
  is >> motlu;
  if (is.eof()) {
    Journal(verb_level) << "LataDB::read_master_file end of file" << endl;
    motlu = "Fin";
  } else {
    check(is, "read string error but not eof !");
  }
  
}

void read_keyword(Entree & is, Nom& nomlu, Motcle& motlu)
{
  read_keyword_nom(is,nomlu);
  motlu=nomlu;
}

// On suppose que motlu contient "blabla=VALEUR". On extrait valeur et on la met dans "param".
// Bidouille: pour traiter le cas "blabla= VALEUR", s'il n'y a rien apres "=" dans motlu, on 
//  relit un mot dans is.
void read_int_param(Entree & is, const Motcle & motlu, LataDBInt64 & param, const char * err_msg)
{
  // Cherche le "="
  const char *s = motlu;
  while (((*s) != ('=')) && ((*s) != 0))
    s++;
  if (s==0) {
    Journal() << "LataDB::read_master_file error: " << err_msg << endl;
    throw(LataDBError(LataDBError::READ_ERROR));
  }
  s++;
  Nom tmp;
  if (*s==0) {
    // il y a une espace entre le = et le parametre ?
    read_keyword_nom(is, tmp);
    s = tmp;
  }
  errno = 0;
  char *errorptr = 0;
  param = strtoll(s, &errorptr, 0 /* base 10 par defaut */);
  if (errno || *errorptr != 0) {
    Journal() << "LataDB::read_master_file error: " << err_msg << endl
              << "Error converting a string to type long int : string = " << s << endl;
    throw(LataDBError(LataDBError::READ_ERROR));
  }
}

void read_int_param(Entree & is, const Motcle & motlu, LataDBInt32 & param, const char * err_msg)
{
  LataDBInt64 i;
  read_int_param(is, motlu, i, err_msg);
  param = int_conversion<LataDBInt32>(i, err_msg);
}


// Idem que read_int_param pour des chaines de caracteres.
void read_string_param(Entree & is, const Nom & motlu, Nom & param, const char * err_msg)
{
  // Cherche le "="
  const char *s = motlu;
  while (((*s) != ('=')) && ((*s) != 0))
    s++;
  if (s==0) {
    Journal() << "LataDB::read_master_file error: " << err_msg << endl;
    throw(LataDBError(LataDBError::READ_ERROR));
  }
  s++;
  param = s;
  // S'il n'y a rien apres =, on lit un mot de plus.
  if (param == "")
    read_keyword_nom(is, param);
}


void read_noms_param(Entree & is, const Nom & motlu, Noms & param, const char * err_msg)
{
  Nom tmp;
  read_string_param(is,motlu,tmp,err_msg);
  /*

  a faire extraire pour de vrai les différents mots de motlu
  Nom motlu2(tmp);
  int nb_comp=1;
  {
    const char *s = tmp;
    int p=0;
    while ( ((*s) != 0))
      {
        if ((*s) == (','))
          {
            nb_comp++;
            //    motlu2[p]='\0';
          }
        p++;
        s++;
      }
  }
  //  cerr<<nb_comp<<" "<<motlu2<<endl;
  // provisoire non fini
  param=Noms(nb_comp);
  {
    const char *s=motlu2;
    for (int i=0;i<nb_comp;i++)
      {
        
        int j=motlu2.find(",");
        if (j==-1) j=0;
        param[i]=(s+j);
        cerr<<param[i]<<endl;
      }
  }
  */
}

// Description: internal tool: checks for valid i and returns the timestep
// Exceptions: BAD_TIMESTEP
const LataDBTimestep & LataDB::get_tstep(entier i) const
{
  if (i < 0 || i >= timesteps_.size()) {
    Journal() << "LataDB::timestep(" << i << ") : wrong timestep" << endl;
    throw(LataDBError(LataDBError::BAD_TIMESTEP));
  }
  return timesteps_[i];
}

// Description: clears the database
void LataDB::reset()
{
  path_prefix_ = "";
  header_ = "";
  case_ = "";
  software_id_ = "";
  timesteps_.reset();
  std::string empty;
  internal_data_buffer_.str(empty);
}

// We update only fields found in the string
// A string can contain both an int type and a float type: we get both in int_type and float_type
static void read_format_string(const Motcle & n, LataDBDataType & data_type, 
                               LataDBDataType::Type & int_type,
                               LataDBDataType::Type & float_type)
{
  int_type = LataDBDataType::UNKNOWN_TYPE;
  float_type = LataDBDataType::UNKNOWN_TYPE;

  if (n.find("ASCII")>=0)
    data_type.msb_ = LataDBDataType::ASCII;
  if (n.find("BIG_ENDIAN")>=0)
    data_type.msb_ = LataDBDataType::MSB_BIG_ENDIAN;
  if (n.find("LITTLE_ENDIAN")>=0)
    data_type.msb_ = LataDBDataType::MSB_LITTLE_ENDIAN;

  if (n.find("INT32")>=0) {
    int_type = data_type.type_ = LataDBDataType::INT32;
    data_type.bloc_marker_type_ = LataDBDataType::INT32;
  }
  if (n.find("INT64")>=0) {
    int_type = data_type.type_ = LataDBDataType::INT64;
    data_type.bloc_marker_type_ = LataDBDataType::INT64;
  }
  if (n.find("REAL32")>=0)
    float_type = data_type.type_ = LataDBDataType::REAL32;
  if (n.find("REAL64")>=0)
    float_type = data_type.type_ = LataDBDataType::REAL64;

  if (n.find("C_INDEXING")>=0)
    data_type.array_index_ = LataDBDataType::C_INDEXING;
  if (n.find("F_INDEXING")>=0)
    data_type.array_index_ = LataDBDataType::F_INDEXING;
  if (n.find("NO_INDEXING")>=0)
    data_type.array_index_ = LataDBDataType::NOT_AN_INDEX;
  
  if (n.find("C_ORDERING")>=0)
    data_type.data_ordering_ = LataDBDataType::C_ORDERING;
  if (n.find("F_ORDERING")>=0)
    data_type.data_ordering_ = LataDBDataType::F_ORDERING;

  if (n.find("F_MARKERS_NO")>=0)
    data_type.fortran_bloc_markers_ = LataDBDataType::NO_BLOC_MARKER;
  if (n.find("F_MARKERS_SINGLE")>=0)
    data_type.fortran_bloc_markers_ = LataDBDataType::BLOC_MARKERS_SINGLE_WRITE;
  if (n.find("F_MARKERS_MULTIPLE")>=0)
    data_type.fortran_bloc_markers_ = LataDBDataType::BLOC_MARKERS_MULTIPLE_WRITES;

  // Fortran bloc markers are tested after INT32 and INT64 because they
  //  override the default value:
  if (n.find("MARKERS32")>=0)
    data_type.bloc_marker_type_ = LataDBDataType::INT32;
  if (n.find("MARKERS64")>=0)
    data_type.bloc_marker_type_ = LataDBDataType::INT64;
}

// This must work together with read_format_string:
void build_format_string(const LataDBDataType & default_type, 
                         const LataDBDataType & type,
                         Motcle & n)
{
  n = "";
  if (type.msb_ != default_type.msb_) {
    switch(type.msb_) {
    case LataDBDataType::ASCII: n += "ASCII,"; break;
    case LataDBDataType::MSB_BIG_ENDIAN: n += "BIG_ENDIAN,"; break;
    case LataDBDataType::MSB_LITTLE_ENDIAN: n += "LITTLE_ENDIAN,"; break;
    default: 
      Journal() << "write master lata: invalid MSB" << endl;
      throw(LataDBError(LataDBError::INVALID_OPERATION));
    }
  }

  // Is an integer type specified in the format string: then the default
  //  fortran bloc marker_type will be changed (look for MARKER32 in read_format_string)
  LataDBDataType::Type default_fortran_bloc_type = default_type.bloc_marker_type_;

  if (type.type_ != default_type.type_) {
    switch(type.type_) {
    case LataDBDataType::INT32: n += "INT32,"; default_fortran_bloc_type = LataDBDataType::INT32; break;
    case LataDBDataType::INT64: n += "INT64,"; default_fortran_bloc_type = LataDBDataType::INT64; break;
    case LataDBDataType::REAL32: n += "REAL32,"; break;
    case LataDBDataType::REAL64: n += "REAL64,"; break;
    default:
      Journal() << "write master lata: invalid type" << endl;
      throw(LataDBError(LataDBError::INVALID_OPERATION));
    }
  }

  // Specify indexing only if integer type:
  if ((type.type_ == LataDBDataType::INT32 || type.type_ == LataDBDataType::INT64)
      && type.array_index_ != default_type.array_index_)
    switch(type.array_index_) {
    case LataDBDataType::C_INDEXING: n += "C_INDEXING,"; break;
    case LataDBDataType::F_INDEXING: n += "F_INDEXING,"; break;
    case LataDBDataType::NOT_AN_INDEX: n += "NO_INDEXING,"; break;
    default:
      Journal() << "write master lata: invalid array_index_" << endl;
      throw(LataDBError(LataDBError::INVALID_OPERATION));
    }

  if (type.data_ordering_ != default_type.data_ordering_) {
    switch(type.data_ordering_) {
    case LataDBDataType::C_ORDERING: n += "C_ORDERING,"; break;
    case LataDBDataType::F_ORDERING: n += "F_ORDERING,"; break;
    default:
      Journal() << "write master lata: invalid data_ordering_" << endl;
      throw(LataDBError(LataDBError::INVALID_OPERATION));
    }
  }

  if (type.fortran_bloc_markers_ != default_type.fortran_bloc_markers_) {
    switch(type.fortran_bloc_markers_) {
    case LataDBDataType::NO_BLOC_MARKER: n += "F_MARKERS_NO,"; break;
    case LataDBDataType::BLOC_MARKERS_SINGLE_WRITE: n += "F_MARKERS_SINGLE,"; break;
    case LataDBDataType::BLOC_MARKERS_MULTIPLE_WRITES: n += "F_MARKERS_MULTIPLE,"; break;
    default:
      Journal() << "write master lata: invalid fortran_bloc_markers_" << endl;
      throw(LataDBError(LataDBError::INVALID_OPERATION));
    }
  }
  
  // Warning : tricky code to determine if we have to specify fortran bloc marker size:
  // If we specify a type_ and this type_ is an integer type, then the fortran bloc
  //  marker has implicitely the same type. We want to override this type if
  //  this assumption is wrong:
  if (type.fortran_bloc_markers_ != LataDBDataType::NO_BLOC_MARKER
      && default_fortran_bloc_type != type.bloc_marker_type_) {
    switch(type.bloc_marker_type_) {
    case LataDBDataType::INT32: n += "MARKER32,"; break;
    case LataDBDataType::INT64: n += "MARKER64,"; break;
    default:
      Journal() << "write master lata: invalid fortran bloc marker type" << endl;
      throw(LataDBError(LataDBError::INVALID_OPERATION));
    }
  }

  // Remove trailing "," if any.
  n.prefix(",");
}

// Description: returns the content of the third line of the file
// Exceptions: FILE_NOT_FOUND, BAD_HEADER (means that this is not a lata file)
Nom LataDB::read_master_file_options(const char *filename)
{
  LataDB db;
  EFichier is;
  db.read_master_file_header(filename, is);
  return db.software_id_; // Returns the content of the third line
}

// Description:
//  Opens the file and reads the three firt lines.
//  Fills the following attributes of the class:
//   header_
//   case_
//   software_id_
//   old_style_lata_
void LataDB::read_master_file_header(const char *filename, EFichier & is)
{
  if (!filename)
    filename = ""; // Will trigger an error for sure !
  is.ouvrir(filename);
  if (!is.good()) { // isnogood ?
    Journal() << "LataDB::read_master_file_options failed opening file " << filename << endl;
    throw(LataDBError(LataDBError::FILE_NOT_FOUND));
  }
  Journal(verb_level-1) << "Trying to read master lata file format LATA " 
                        << filename << endl;

  const entier bufsize=1024;
  char s[bufsize+1];
  // Lecture de l'entete:
  is.get_istream().getline(s, bufsize);
  check(is, "failed reading line 1");
  const char * lata_header = "LATA_V2.";
  old_style_lata_ = 0;
  if (strncmp(s, lata_header, strlen(lata_header)) == 0) {
    Journal(2) << "LataDB::read_master_file found lata format " << lata_header << endl;
    old_style_lata_ = 0;
  } else if (Motcle(s).debute_par("Trio_U")) {
    Journal(2) << "LataDB::read_master_file found old style lata format" << endl;
    old_style_lata_ = 1;
  } else {
    Journal(2) << "LataDB::read_master_file error reading header: expected LATA_V2.0 or Trio_U" 
               << " instead of " << s << endl;
    throw(LataDBError(LataDBError::BAD_HEADER));
  }
  header_ = s;
  is.get_istream().getline(s, bufsize);
  check(is, "failed reading line 2");
  case_ = s;
  is.get_istream().getline(s, bufsize);
  check(is, "failed reading line 3");
  software_id_ = s; 
}


// Description: Reads the .lata database in the given file indicating than the 
//  associated data files will be found in directory "prefix".
//  If not empty, "prefix" must finish with a '/'.
//  For "prefix" and "filename", if they do not begin with '/', are relative to pwd.
// Exceptions: 
//  BAD_HEADER  means that the header found in this stream is not LATA_V2
//  READ_ERROR  means that an error has been found in the file (premature eof,
//              io error, bad keyword, ...)
//  FILE_NOT_FOUND means that, well, the lata file could not be opened
void LataDB::read_master_file(const char *prefix, const char *filename)
{
  reset();

  if (!prefix)
    prefix = "";
  path_prefix_ = prefix;

  //Journal() << "RECOMPILED PLUGIN !" << endl;

  EFichier is;
  read_master_file_header(filename, is);

  // Defaults for lataV1
  default_type_int_.msb_ = LataDBDataType::ASCII;
  default_type_int_.type_ = LataDBDataType::INT32;
  default_type_int_.array_index_ = LataDBDataType::F_INDEXING;
  default_type_int_.data_ordering_ = LataDBDataType::C_ORDERING;
  default_type_int_.fortran_bloc_markers_ = LataDBDataType::BLOC_MARKERS_SINGLE_WRITE;
  default_type_int_.bloc_marker_type_ = LataDBDataType::INT32;
  default_float_type_ = LataDBDataType::REAL32;
  
  // Create timestep 0 (global domain and fields)
  timesteps_.add(LataDBTimestep());
  entier interface_file_not_found = 0;
  Nom nomlu;
  Motcle motlu;
  read_keyword(is, nomlu,motlu);

  while (1) {
    if (motlu == "Fin") 
      {
        Journal(verb_level) << "End of file by FIN" << endl;
        break;
      } 
    else if (motlu == "Format") 
      {
        Journal(verb_level) << "Reading Format " << endl;
        read_keyword(is, nomlu, motlu);
        LataDBDataType::Type tmp_int_type;
        read_format_string(motlu, default_type_int_, tmp_int_type, default_float_type_);
        default_type_int_.type_ = tmp_int_type;
        read_keyword(is, nomlu, motlu);
      } 
    else if (motlu == "Temps") 
      {
        LataDBTimestep & t = timesteps_.add(LataDBTimestep());
        const entier i = timesteps_.size() - 1;
        is >> t.time_;
        check(is, "failed reading time parameter");
        Journal(verb_level) << "Reading timestep " << i << " t=" << t.time_ << endl;
        read_keyword(is, nomlu, motlu);
      }
    else if (motlu == "Geom")
      {
        // This is the new syntax to declare a geometry.
        // nodes, elements faces, files are declared in separate "champ" entries
        LataDBGeometry dom;
        dom.timestep_ = timesteps_.size()-1;
        is >> dom.name_;
        check(is, "failed reading domain name");
        Journal(verb_level) << "New domain " << dom.name_ << endl;
        while (1) {
          read_keyword(is, nomlu, motlu);
          if (motlu.debute_par("type_elem=")) {
            read_string_param(is, motlu, dom.elem_type_, "error reading type_elem parameter");
            Journal(verb_level+1) << " type_elem=" << dom.elem_type_ << endl;
          } else
            break;
        }
        if (dom.elem_type_ == "") {
          Journal() << "Error reading Geometry: missing type_elem parameter" << endl;
          throw(LataDBError(LataDBError::READ_ERROR));
        }
        add(timesteps_.size() - 1, dom);
      }
    else if (motlu == "Geometrie") 
      {
        // Declare a geometry: nodes and elements are embedded in a single file described here
        // (legacy syntax)
        LataDBGeometry dom;
        LataDBField som;
        // Name
        is >> dom.name_;
        dom.timestep_ = timesteps_.size()-1;
        check(is, "failed reading domain name");
        Journal(verb_level) << "Reading domain " << dom.name_ << endl;
        som.name_ = "SOMMETS";
        som.geometry_ = dom.name_;
        // Filenames
        Nom n;
        is >> n;
        check(is, "failed reading domain filename");
        som.filename_ = n;
        entier nb_elem = -1;
        entier nb_faces = -1;
        entier nproc = -1;
        Nom file_decal_som;
        Nom file_decal_elem;
        Nom file_decal_faces;
        while (1) {
          read_keyword(is, nomlu, motlu);
          if (motlu.debute_par("nb_som_tot=")) {
            read_int_param(is, motlu, som.size_, "bad nb_som_tot parameter");
          } else if (motlu.debute_par("nb_elem_tot=")) {
            read_int_param(is, motlu, nb_elem, "bad nb_elem_tot parameter");
          } else if (motlu.debute_par("type_elem=")) {
            read_string_param(is, motlu, dom.elem_type_, "error reading type_elem parameter");
          } else if (motlu.debute_par("nb_faces_tot=")) {
            read_int_param(is, motlu, nb_faces, "bad nb_elem_tot parameter");
          } else if (motlu.debute_par("format=")) {
            Motcle fmt;
            read_string_param(is, motlu, fmt, "bad format parameter");
            if (fmt=="BINARY") {
              default_type_int_.msb_ = LataDBDataType::machine_msb_;
            }
          } else if (motlu.debute_par("joints_sommets")) {
            read_keyword(is, nomlu, motlu);
            read_int_param(is, motlu, nproc, "bad nproc parameter");
            read_keyword(is, nomlu, motlu);
            read_string_param(is, nomlu, file_decal_som, "bad decalage file parameter");
            Journal(verb_level+1) << " decal_som " << nproc;
          } else if (motlu.debute_par("joints_elements")) {
            read_keyword(is, nomlu, motlu);
            read_int_param(is, motlu, nproc, "bad nproc parameter");
            read_keyword(is, nomlu, motlu);
            read_string_param(is, nomlu, file_decal_elem, "bad decalage file parameter");            
            Journal(verb_level+1) << " decal_elem " << nproc;
          } else if (motlu.debute_par("joints_faces")) {
            read_keyword(is, nomlu, motlu);
            read_int_param(is, motlu, nproc, "bad nproc parameter");
            read_keyword(is, nomlu, motlu);
            read_string_param(is, nomlu, file_decal_faces, "bad decalage file parameter");            
            Journal(verb_level+1) << " decal_faces " << nproc;
          } else
            break;
          Journal(verb_level+1) << " " << motlu << endl;
        }
        som.datatype_ = default_type_float();
        LataDBField elem(som);
        elem.size_ = nb_elem;
        elem.datatype_ = default_type_int_;
        LataDBField faces(elem); // copy filename_
        faces.size_ = nb_faces;
        LataDBField elem_faces(elem);
        elem_faces.size_ = nb_elem;
        elem.name_ = "ELEMENTS";
        faces.name_ = "FACES";
        elem_faces.name_ = "ELEM_FACES";
        elem.geometry_ = dom.name_;
        faces.geometry_ = dom.name_;
        elem_faces.geometry_ = dom.name_;

        if (som.size_ < 0 || elem.size_ < 0) {
          Journal() << "Error reading Geometry: missing or bad nb_som_tot or nb_elem_tot parameter" << endl;
          throw(LataDBError(LataDBError::READ_ERROR));
        }
        if (dom.elem_type_ == "") {
          Journal() << "Error reading Geometry: missing type_elem parameter" << endl;
          throw(LataDBError(LataDBError::READ_ERROR));
        }
        get_element_data(dom.elem_type_, som.nb_comp_, elem.nb_comp_, faces.nb_comp_, elem_faces.nb_comp_);

        // Add domain and som which are complete. We need the "som" to be in the database
        //  for the "old lata 2D hack" in read_data2_()
        add(timesteps_.size() - 1, dom);
        add(timesteps_.size() - 1, som);
        // Parse the geometry file to find file_offsets 
        {
          Journal(verb_level) << " Parsing geometry file to find file offset of data blocs" << endl;
          LataDataFile f(internal_data_buffer_, path_prefix_, som.filename_,som.datatype_.msb_);
          IntTab * null = 0; // Null pointer => don't actually read the data
          read_data2_(f, som, null);
          elem.datatype_.file_offset_ = f.position();
          Journal(verb_level+1) << "  elements at file offset " << elem.datatype_.file_offset_ << endl;
          if (faces.size_ >= 0) {
            read_data2_(f, elem, null);
            faces.datatype_.file_offset_ = f.position();
            Journal(verb_level+1) << "  faces at file offset " << faces.datatype_.file_offset_ << endl;
            read_data2_(f, faces, null);
            elem_faces.datatype_.file_offset_ = f.position();
            Journal(verb_level+1) << "  elem_faces at file offset " << elem_faces.datatype_.file_offset_ << endl;
          }
        }

        add(timesteps_.size() - 1, elem);
        if (faces.size_ >= 0) {
          Journal(verb_level+1) << " Adding FACES and ELEM_FACES " << faces.size_ << endl;
          add(timesteps_.size() - 1, faces);
          add(timesteps_.size() - 1, elem_faces);
        }
        if (nproc > -1) {
          LataDBField joint(elem);
          joint.datatype_.file_offset_ = 0;
          joint.size_ = nproc;
          joint.nb_comp_ = 2;
          joint.reference_ = "";
          joint.name_ = "JOINTS_SOMMETS";
          joint.filename_ = file_decal_som;
          add(timesteps_.size() - 1, joint);
          joint.reference_ = "";
          joint.name_ = "JOINTS_ELEMENTS";
          joint.filename_ = file_decal_elem;
          add(timesteps_.size() - 1, joint);
          if (file_decal_faces != "??") {
            joint.reference_ = "";
            joint.name_ = "JOINTS_FACES";
            joint.filename_ = file_decal_faces;
            add(timesteps_.size() - 1, joint);
          }
        }
      }
    else if (motlu == "Champ")
      {
        LataDBField field;
        field.datatype_ = default_type_float();
        is >> field.name_;
        check(is, "failed reading field name");
        Journal(verb_level) << "Reading field " << field.name_ << endl; 
        Nom n;
        is >> n;
        check(is, "failed reading field filename");
        field.filename_ = n;
        Journal(verb_level+1) << " filename=" << n << endl;

        if ((field.name_ == "INTERFACES" || field.name_ == "PARTICULES") && old_style_lata_) {
          // This is the old dirty syntax for moving meshes

          Journal(verb_level+1) << " Parsing an oldstyle interface file" << endl;
          // Open the file and read the content
          try {
            LataDBDataType::MSB msb = default_type_int_.msb_;
            LataDataFile f(internal_data_buffer_, path_prefix_, field.filename_,msb);
           
            LataDBDataType::Type int_type = default_type_int_.type_;
            LataDBDataType::Type float_type = default_float_type_;
            LataDBGeometry dom;
            dom.timestep_ = timesteps_.size()-1;
            dom.name_ = field.name_;
            LataDBField som;
            som.name_ = "SOMMETS";
            som.filename_ = field.filename_;
            som.geometry_ = field.name_;
            som.datatype_ = default_type_float();
            som.datatype_.fortran_bloc_markers_ = LataDBDataType::NO_BLOC_MARKER;
            ArrOfInt tmptab(2);
            bloc_read(f, msb, int_type, tmptab);
            som.nb_comp_ = tmptab[0]; // dimension
            som.size_ = tmptab[1]; // nb nodes
            Journal(verb_level+1) << " Nb nodes=" << som.size_ << " dimension=" << som.nb_comp_ << endl;
            som.datatype_.file_offset_ = f.position();
            bloc_read_skip(f, msb, float_type, som.size_ * som.nb_comp_);
            LataDBField elem;
            elem.name_ = "ELEMENTS";
            elem.filename_ = field.filename_;
            elem.geometry_ = field.name_;
            elem.datatype_ = default_type_int_;
            elem.datatype_.fortran_bloc_markers_ = LataDBDataType::NO_BLOC_MARKER;
            elem.datatype_.array_index_ = LataDBDataType::C_INDEXING;
            bloc_read(f, msb, int_type, tmptab);
            elem.nb_comp_ = tmptab[0];
            elem.size_ = tmptab[1];
            Journal(verb_level+1) << " Nb elements=" << elem.size_ << " shape=" << elem.nb_comp_ << endl;
            if (field.name_ == "PARTICULES") {
              // Special case for front-tracking markers
              Journal(verb_level+1) << " PARTICULES: element type = point" << endl;
              dom.elem_type_ = "POINT";
            } else {
              if (elem.nb_comp_ == 2)
                dom.elem_type_ = "SEGMENT";
              else if (elem.nb_comp_ == 3)
                dom.elem_type_ = "TRIANGLE_3D";
              else {
                Journal() << "Error reading an interface: invalid element shape " << elem.nb_comp_ << endl;
                throw(LataDBError(LataDBError::READ_ERROR));
              }
            }
            elem.datatype_.file_offset_ = f.position();
            bloc_read_skip(f, msb, int_type, elem.size_ * elem.nb_comp_);
            add(timesteps_.size() - 1, dom);
            add(timesteps_.size() - 1, som);
            add(timesteps_.size() - 1, elem);
            // Read components:
            while(1) {
              LataDBField field;
              f.set_encoding(msb, int_type);
              f.set_exception(0);
              f >> field.localisation_;
              f.set_exception(1);
              if (field.localisation_ == "")
                break;
              field.filename_ = som.filename_;
              field.geometry_ = som.geometry_;
              field.datatype_ = som.datatype_;
              tmptab.resize_array(1);
              bloc_read(f, msb, int_type, tmptab);
              field.nb_comp_ = tmptab[0];
              if (field.nb_comp_ == som.nb_comp_)
                field.nature_ = LataDBField::VECTOR;
              else
                field.nature_ = LataDBField::SCALAR;
              f >> field.name_;
              if (field.localisation_ == "SOM") {
                field.size_ = som.size_;
              } else {
                field.size_ = elem.size_;
              }
              Journal(verb_level+1) << " Interface field " << field.localisation_ << " "
                                    << field.name_ << endl;
              field.datatype_.file_offset_ = f.position();
              bloc_read_skip(f, msb, float_type, field.size_ * field.nb_comp_);
              add(timesteps_.size() - 1, field);
            }
          }
          catch (LataDBError err) {
            // If file is missing, issue the "missing file" message and continue
            if (err.err_ != LataDBError::FILE_NOT_FOUND)
              throw;
            else
              interface_file_not_found++;
          }
          // Read next keyword:
          read_keyword(is, nomlu, motlu);
        } else {
          if (old_style_lata_) {
            // Old (legacy) syntax for champs: we must guess the properties from the filename!

            field.datatype_ = default_type_float();
            // Extract other data from filename (nb_comp_, localisation_, etc)
            // find geometry name
            Noms dom_names = geometry_names(0 /* timestep */);
            const entier nb_geom = dom_names.size();
            entier i;
            for (i = 0; i < nb_geom; i++) {
              Nom testname(".");
              testname += dom_names[i];
              testname += ".";
              if (Motcle(n).find(testname)>=0)
                break;
            }
            if (i == nb_geom) {
              Journal() << "Error in LataDB_V1::read_master_file: could not find domain for Champ " << n << endl;
              throw(LataDBError(LataDBError::READ_ERROR));
            }
            Journal(verb_level+1) << " geometry=" << dom_names[i] << endl;
            const LataDBGeometry & dom = get_geometry(0, dom_names[i]);
            field.geometry_ = dom_names[i];
            lata_v1_get_localisation(n, field.localisation_);
            Journal(verb_level+1) << " localisation=" << field.localisation_ << endl;
            const LataDBField & sommets = get_field(0 /* timestep */, dom_names[i], "SOMMETS", "*");
            const entier dim = sommets.nb_comp_;
            field.nb_comp_ = lata_v1_get_nb_comp(field.name_, field.localisation_, dom, dim, field.nature_, field.datatype_.data_ordering_);
            Journal(verb_level+1) << " composantes=" << field.nb_comp_ << endl;
            if (field.localisation_.debute_par("SOM"))
              field.size_ = sommets.size_;
            else if (field.localisation_.debute_par("ELEM"))
              field.size_ = get_field(0 /* timestep */, dom_names[i], "ELEMENTS", "*").size_;
            else if (field.localisation_.debute_par("FACE"))
              field.size_ = get_field(0 /* timestep */, dom_names[i], "FACES", "*").size_;
            else {
              Journal() << "Error in LataDB_V1::read_master_file: invalid localisation "
                        << field.localisation_ << endl;
              throw(LataDBError(LataDBError::READ_ERROR));
            }
            // Read next keyword:
            read_keyword(is, nomlu, motlu);
          } else {
            // NEW LATAV2 SYNTAX for fields
            // The default data type is "float_"
            field.datatype_ = default_type_float();
            field.size_ = -1;
            while(1) {
              read_keyword(is, nomlu, motlu);
              if (motlu.debute_par("geometrie=")) {
                read_string_param(is, nomlu, field.geometry_, "error reading geometrie parameter");
                // Check that the geometry exists
                get_geometry(timesteps_.size() - 1, field.geometry_, FIRST_AND_CURRENT);
              } else if (motlu.debute_par("composantes=")) {
                read_int_param(is, motlu, field.nb_comp_, "bad composantes parameter");
              } else if (motlu.debute_par("localisation=")) {
                read_string_param(is, motlu, field.localisation_, "error reading localisation parameter");
              } else if (motlu.debute_par("format=")) {
                LataDBDataType::Type tmp_int_type; // Unused
                LataDBDataType::Type tmp_float_type; // unused
                read_format_string(motlu, field.datatype_, tmp_int_type, tmp_float_type);
              } else if (motlu.debute_par("size=")) {
                read_int_param(is, motlu, field.size_, "error reading size parameter");
              } else if (motlu.debute_par("file_offset=")) {
                read_int_param(is, motlu, field.datatype_.file_offset_, "error reading file offset parameter");
              } else if (motlu.debute_par("nature=")) {
                Motcle nat;
                read_string_param(is, motlu, nat, "error reading nature parameter");
                if (nat.find("SCALAR")>=0)
                  field.nature_ = LataDBField::SCALAR;
                else if (nat.find("VECTOR")>=0)
                  field.nature_ = LataDBField::VECTOR;
                else {
                  Journal() << "Error in LataDB_V1::read_master_file: invalid nature "
                            << nat << endl;
                  throw(LataDBError(LataDBError::READ_ERROR));
                }
              } else if (motlu.debute_par("reference=")) {
                Nom ref;
                read_string_param(is, motlu, ref, "error reading reference parameter");
                field.reference_ = ref;
              } else if (motlu.debute_par("noms_compo=")) {
                Noms ref;
                read_noms_param(is, motlu, ref, "error reading noms_compo");
                Journal(verb_level+1)<<"noms_compos pas interprete "<<motlu<<endl;
              }
              else
                break;
              Journal(verb_level+1) << " " << motlu << endl;
            }
            if (field.size_ < 0) {
              // This is untested. Deactivate for the moment.
              //              Journal(verb_level) << " No size parameter given. Take size of the localisation field: ";
              // if (field_exists(timesteps_.size() - 1, field.geometry_, field.localisation_, FIRST_AND_CURRENT)) {
              // field.size_ = get_field(timesteps_.size() - 1, field.geometry_, field.localisation_, FIRST_AND_CURRENT).size_;
              // } else {
              Journal() << " Error, no size parameter for field " << field.name_ << " and localisation " << field.localisation_ 
                        << " does not match any existing field" << endl;
              throw(LataDBError(LataDBError::READ_ERROR));
              // }
            }
          }
          add(timesteps_.size() - 1, field);
        }
      }
    else if (motlu == "import_file")
      {
        // Load another lata master file recursively and merge timesteps
        Nom filename;
        is >> filename; // Read filename (without prefix)
        LataDB newdb;
        Nom filename2(prefix);
        filename2 += filename;
        Journal(verb_level) << "Importing another lata database from file: " << filename << endl;
        newdb.read_master_file(prefix, filename2);
      }
    else
      {
        Journal() << "Error: unknown keyword: " << motlu << endl;
        throw(LataDBError(LataDBError::READ_ERROR));
      }
  }
  if (interface_file_not_found)
    throw LataDBError(LataDBError::FILE_NOT_FOUND);
}

// Read field data from file f into data array "data".
//  If data is a null pointer, just skip the data bloc and leave the file pointer
//  at the beginning of the next data bloc (used to parse the geometry file if file_offset
//  are not specified in the lata master file)
template <class C_Tab>
void LataDB::read_data2_(LataDataFile & f,
                         const LataDBField & fld,
                         C_Tab * const data, // const pointer to non const data !
                         entier debut, entier n, const ArrOfInt *lines_to_read) const
{
  // Si file_offset_ vaut 0 on y va car on peut avoir lu a un autre endroit avant.
  if (fld.datatype_.file_offset_ >= 0) {
    Journal(verb_level_data_bloc+1) << " Seeking at position " << fld.datatype_.file_offset_ << endl; 
    f.seek(fld.datatype_.file_offset_, LataDataFile::ABSOLUTE);
  }
  if (n < 0) {
    if (lines_to_read)
      n = lines_to_read->size_array();
    else
      n = fld.size_;
  }

  // in old lata format, 2d data is written as 3d:
  // Yeah: dirty specs make dirty code...
  entier size_in_file = fld.size_;
  entier nb_comp_in_file = fld.nb_comp_;
  entier old_lata_hack = 0; (void) old_lata_hack;
  if (old_style_lata_ && (Motcle(fld.geometry_) != "INTERFACES") && (Motcle(fld.geometry_) != "PARTICULES")) {
    const LataDBField & som = get_field(0, fld.geometry_, "SOMMETS", "*");
    if (som.nb_comp_ == 2) {
      old_lata_hack = 1;
      if (fld.name_ == "ELEMENTS") {
        nb_comp_in_file *= 2;
      } else if (fld.name_ == "SOMMETS") {
        nb_comp_in_file = 3; // all coordinates in 3D
        size_in_file *= 2;
      } // else if (fld.localisation_.debute_par("SOM")) {
        // size_in_file *= 2;
      // }
      Journal(verb_level_data_bloc+1) << "Old lata hack for 2D" << endl;
    }
  }
  
  if (fld.nb_comp_ < 0 || fld.size_ < 0) {
    Journal() << "Error in LataDB::read_data_: nb_comp_ or size_ not initialized for component " << fld.name_ << endl;
    throw;
  }

  if ((!lines_to_read) && (debut < 0 || debut + n > fld.size_)) {
    Journal() << "Error in LataDB::read_data_: [debut,debut+n] invalid range (size=" << fld.size_ << ")" << endl;
    throw;
  }

  if (data)
    data->resize(n, nb_comp_in_file);

  switch (fld.datatype_.data_ordering_) {
  case LataDBDataType::C_ORDERING:
    // data written like this: tab(0,0) tab(0,1) tab(0,2) ... tab(1,0) tab(1,1) tab(1,2) ...
    if (fld.datatype_.fortran_bloc_markers_ == LataDBDataType::BLOC_MARKERS_MULTIPLE_WRITES) {
      Journal() << "Error in LataDB::read_data_: fortran_bloc_markers_=MULTIPLE_WRITES is incompatible with data_ordering=C" << endl;
      throw LataDBError(LataDBError::DATA_ERROR);
    }
    skip_blocksize(f, fld.datatype_);
    if (data) {
      if (!lines_to_read) {
        bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, (FileOffset)debut * nb_comp_in_file);
        bloc_read(f, fld.datatype_.msb_, fld.datatype_.type_, *data);
        bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, (FileOffset)(size_in_file - debut - n) * nb_comp_in_file);
      } else {
        C_Tab tmp;
        // Read 1024 lines chunks at a time even if only some values are needed inside
        entier chunk_size = 0;
        entier current_chunk_pos = 0;
        entier current_file_pos = 0;
        const entier n = lines_to_read->size_array();
        for (entier i = 0; i < n; i++) {
          const entier next_line = (*lines_to_read)[i];
          // Is this line in the current chunk ?
          if (next_line >= current_chunk_pos + chunk_size) {
            // No => read the chunk containing this line
            chunk_size = size_in_file - next_line;
            if (chunk_size > 1024)
              chunk_size = 1024;
            tmp.resize(chunk_size, nb_comp_in_file);
            bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, (next_line - current_file_pos) * nb_comp_in_file);
            bloc_read(f, fld.datatype_.msb_, fld.datatype_.type_, tmp);
            current_chunk_pos = next_line;
            current_file_pos = next_line + chunk_size;
          }
          // Extract data from tmp array
          const entier tmp_index = next_line - current_chunk_pos;
          for (entier j = 0; j < nb_comp_in_file; j++)
            (*data)(i, j) = tmp(tmp_index, j);
        }
        if (current_file_pos != size_in_file)
          bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, (size_in_file - current_file_pos) * nb_comp_in_file);
      }
    } else {
      // just skip the data
      bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, size_in_file *  nb_comp_in_file);
    }
    skip_blocksize(f, fld.datatype_);
    break;
  case LataDBDataType::F_ORDERING:
    {
      // data written like this: tab(0,0) tab(1,0) tab(2,0) ... tab(0,1) tab(1,1) tab(2,1) ... tab(0,2) tab(1,2) tab(2,2) ...
      entier multiple_bloc_markers = (fld.datatype_.fortran_bloc_markers_ == LataDBDataType::BLOC_MARKERS_MULTIPLE_WRITES);
      // reverse rows and columns of the array
      C_Tab tmp;
      if (!multiple_bloc_markers)
        skip_blocksize(f, fld.datatype_);
      for (entier i = 0; i < nb_comp_in_file; i++) {
        if (multiple_bloc_markers)
          skip_blocksize(f, fld.datatype_);
        if (data) {
          if (!lines_to_read) {
            tmp.resize(n, 1);
            bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, debut);
            bloc_read(f, fld.datatype_.msb_, fld.datatype_.type_, tmp);
            bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, size_in_file - debut - n);
            for (entier j = 0; j < n; j++)
              (*data)(j, i) = tmp(j, 0);
          } else {

            // Read 1024 lines chunks at a time even if only some values are needed inside
            entier chunk_size = 0;
            entier current_chunk_pos = 0;
            entier current_file_pos = 0;
            const entier n = lines_to_read->size_array();
            for (entier j = 0; j < n; j++) {
              const entier next_line = (*lines_to_read)[j];
              // Is this line in the current chunk ?
              if (next_line >= current_chunk_pos + chunk_size) {
                // No => read the chunk containing this line
                chunk_size = size_in_file - next_line;
                if (chunk_size > 1024)
                  chunk_size = 1024;
                tmp.resize(chunk_size, 1);
                bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, (next_line - current_file_pos));
                bloc_read(f, fld.datatype_.msb_, fld.datatype_.type_, tmp);
                current_chunk_pos = next_line;
                current_file_pos = next_line + chunk_size;
              }
              // Extract data from tmp array
              const entier tmp_index = next_line - current_chunk_pos;
              (*data)(j, i) = tmp(tmp_index, 0);
            }
            if (current_file_pos != size_in_file)
              bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, (size_in_file - current_file_pos));
          }
        } else {
          bloc_read_skip(f, fld.datatype_.msb_, fld.datatype_.type_, size_in_file);
        }
        if (multiple_bloc_markers)
          skip_blocksize(f, fld.datatype_);
      }
      if (!multiple_bloc_markers)
        skip_blocksize(f, fld.datatype_);
      break;
    }
  default:
    Journal() << "Error in LataDB::read_data_: data_ordering not implemented" << endl;
    throw;
  }
  
  // old lata 2d hack :
  if (data && nb_comp_in_file != fld.nb_comp_) {
    // drop column in data array
    C_Tab tmp(*data);
    data->resize(n, fld.nb_comp_);
    for (entier i = 0; i < n; i++) 
      for (entier j = 0; j < fld.nb_comp_; j++)
        (*data)(i,j) = tmp(i,j);
  }
}

// Description: 
//  Read n * fld.nb_comp_ values in the file filename_, starting from debut * fld.nb_comp_
template <class C_Tab>
void LataDB::read_data_(const LataDBField & fld, 
                        C_Tab & data, entier debut, entier n) const
{
  Journal(verb_level_data_bloc) << "LataDB::read_data(" << fld.timestep_ << "," << fld.uname_
                                << ") Reading " << path_prefix_ << fld.filename_ << " start at " << debut << " size "
                                << n << endl;

  LataDataFile f(internal_data_buffer_, path_prefix_, fld.filename_,fld.datatype_.msb_);
  read_data2_(f, fld, &data, debut, n);

}

// Description: 
//  Read n * fld.nb_comp_ values in the file filename_, starting from debut * fld.nb_comp_
template <class C_Tab>
void LataDB::read_data_(const LataDBField & fld,
                        C_Tab & data, const ArrOfInt & lines_to_read) const
{
  Journal(verb_level_data_bloc) << "LataDB::read_data(" << fld.timestep_ << "," << fld.uname_
                                << ") Reading " << path_prefix_ << fld.filename_ << ",  " << lines_to_read.size_array() << " non contiguous lines" 
                                << endl;

  LataDataFile f(internal_data_buffer_, path_prefix_, fld.filename_,fld.datatype_.msb_);
  read_data2_(f, fld, &data, -1, -1, &lines_to_read);
}

// Description: reads n * nb_comp values in the file filename_ starting from debut*nb_comp_
//  If array_index is F_STYLE, substract 1 to all values.
void LataDB::read_data(const LataDBField & fld, IntTab & data, entier debut, entier n) const
{
  read_data_(fld, data, debut, n);
  if (fld.datatype_.array_index_ == LataDBDataType::F_INDEXING) {
    ArrOfInt & data2 = data;
    const entier n = data2.size_array();
    for (entier i = 0; i < n; i++)
      data2[i]--;
  }
}

// Description: reads n * nb_comp values in the file filename_ starting from debut*nb_comp_
void LataDB::read_data(const LataDBField & fld, DoubleTab & data, entier debut, entier n) const
{
  Journal() << "LataDB::read_data not coded for double" << endl;
//  exit(-1);
}

void LataDB::read_data(const LataDBField & fld, FloatTab & data, entier debut, entier n) const
{
  read_data_(fld, data, debut, n);
}

// Description: reads lines_to_read.size_array() * nb_comp values.
//  If array_index is F_STYLE, substract 1 to all values.
void LataDB::read_data(const LataDBField & fld, IntTab & data, const ArrOfInt & lines_to_read) const
{
  read_data_(fld, data, lines_to_read);
  if (fld.datatype_.array_index_ == LataDBDataType::F_INDEXING) {
    ArrOfInt & data2 = data;
    const entier n = data2.size_array();
    for (entier i = 0; i < n; i++)
      data2[i]--;
  }
}

// Description: reads lines_to_read.size_array() * nb_comp values.
void LataDB::read_data(const LataDBField & fld, DoubleTab & data, const ArrOfInt & lines_to_read) const
{
  Journal() << "LataDB::read_data not coded for double" << endl;
//  exit(-1);
}

// Description: reads lines_to_read.size_array() * nb_comp values.
void LataDB::read_data(const LataDBField & fld, FloatTab & data, const ArrOfInt & lines_to_read) const
{
  read_data_(fld, data, lines_to_read);
}


// Description: copy the source LataDB object, keeping only timesteps, geometries and fields
//  that are specified (timestep 0 is always included, do not put it in the list).
//  field_nms can contain field.name_ (like VITESSE), or extended name with localisation
//  (like VITESSE_ELEM)
void LataDB::filter_db(const LataDB & source,
                       const Motcles & geometry_nms,
                       const Motcles & field_nms,
                       const ArrOfInt & timesteps)
{
  path_prefix_ = source.path_prefix_;
  header_ = source.header_;
  case_ = source.case_;
  software_id_ = source.software_id_;
  old_style_lata_ = source.old_style_lata_;
  default_type_int_ = source.default_type_int_;
  default_float_type_ = source.default_float_type_;
  
  const entier nb_tsteps = timesteps.size_array();
  for (entier it = 0; it < nb_tsteps + 1; it++) {
    entier src_tstep = 0;
    if (it > 0)
      src_tstep = timesteps[it-1];
    LataDBTimestep & tstep = timesteps_.add(LataDBTimestep());
    tstep.time_ = source.get_time(src_tstep);
    // Copy geometries
    Motcles geoms = noms_to_motcles(source.geometry_names(src_tstep));
    entier ig;
    for (ig = 0; ig < geoms.size(); ig++)
      if (geometry_nms.rang(geoms[ig]) >= 0)
        tstep.geoms_.add(source.get_geometry(src_tstep, geoms[ig]));
    // Copy fields
    geoms = noms_to_motcles(geometry_names(nb_timesteps()-1, FIRST_AND_CURRENT));
    for (ig = 0; ig < geoms.size(); ig++) {
      LataVector<Field_UName> unames = source.field_unames(src_tstep, geoms[ig], "*");
      for (entier i_f = 0; i_f < unames.size(); i_f++) {
        const LataDBField & src = source.get_field(src_tstep, unames[i_f]);
        Nom name_loc = src.name_;
        name_loc += "_";
        name_loc += src.localisation_;
        if (field_nms.rang(src.name_) >= 0 || field_nms.rang(name_loc) >= 0) 
          tstep.fields_.add(src);
      }
    }
  }
}

// Description: set the default value of the path prefix where write_data() will write the data
// Warning: there is no check that the master lata file is actually written at the same place 
//  and that all the files and data blocks mentionned in the database actually exist.
// For the file_offset_ field, -2 is considered "unknown".
void LataDB::set_path_prefix(const char * s)
{
  path_prefix_ = s;
}

#define UPDATE_MACRO(x,unknown) if (((old_type.x==unknown)||(type.x==old_type.x))&&(new_type.x!=unknown)) type.x=new_type.x

// Description: changes the data type of all fields in the database.
//  The property "x" is changed to "new_type.x" if "new_type.x" is not "unknown"
//  and if "old_type.x" is "unknown" or "equal to the previous property" 
// Example: convert all data to ASCII:
//   LataDBDataType old_type; // All defaults to "unknown" => we update all fields
//   LataDBDataType new_type;
//   new_type.msb_ = LataDBDataType::ASCII; // Change msb_ property to ASCII:
// Example 2: change all REAL32 data to REAL64
//   LataDBDataType old_type;
//   old_type.type_ = LataDBDataType::REAL32;
//   LataDBDataType new_type;
//   new_type.msb_ = LataDBDataType::REAL64;
void LataDB::change_all_data_types(const LataDBDataType & old_type, const LataDBDataType & new_type)
{
  const entier nb_tsteps = timesteps_.size();
  for (entier src_tstep = 0; src_tstep < nb_tsteps; src_tstep++) {
    LataVector<LataDBField> & fields = timesteps_[src_tstep].fields_;
    const entier nb_fields = fields.size();
    for (entier i_field = 0; i_field < nb_fields; i_field++) {
      LataDBDataType & type = fields[i_field].datatype_;
      // For each field, if "old_type" is "unknown" or equal to the previous value,
      //  and if "new_type" is not "unknown, then update the field
      UPDATE_MACRO(msb_, LataDBDataType::UNKNOWN_MSB);
      UPDATE_MACRO(type_, LataDBDataType::UNKNOWN_TYPE);
      UPDATE_MACRO(array_index_, LataDBDataType::UNKNOWN_ARRAYINDEX);
      UPDATE_MACRO(data_ordering_, LataDBDataType::UNKNOWN_ORDERING);
      UPDATE_MACRO(fortran_bloc_markers_, LataDBDataType::UNKNOWN_MARKERS);
      UPDATE_MACRO(bloc_marker_type_, LataDBDataType::UNKNOWN_TYPE);
    }
  }
}
#undef UPDATE_MACRO

void LataDB::change_all_data_filenames(const Nom & old_prefix, const Nom & new_prefix)
{
  const entier nb_tsteps = timesteps_.size();
  for (entier i = 0; i < nb_tsteps; i++) {
    LataVector<LataDBField> & fields = timesteps_[i].fields_;
    // Browse all fields:
    const entier nb_fields = fields.size();
    for (entier j = 0; j < nb_fields; j++) {
      Nom & filename = fields[j].filename_;
      Nom old_filename = filename;
      filename = new_prefix;
      if (old_filename.debute_par(old_prefix)) {
        const entier n = old_filename.longueur()-1;
        const char * s = old_filename;
        for (entier i = old_prefix.longueur()-1; i < n; i++)
          filename += Nom(s[i]);
      } else if (old_filename == LataDBField::memory_buffer_file()) {
        filename += Nom(".data");
      } else {
        filename += Nom('_');
        filename += old_filename;
      }
      Journal(verb_level+1) << " Changing filename " << old_filename << " -> " << filename << endl;
    }
  } 
}

// This method takes all filenames mentionned in the database and sets the file_offset_ entry:
//  - set to 0 for the first field where a given filename appears,
//  - then for all subsequent files referring to the same name:
//      If split_files != 0, rename the files by appending a "_number" and set file_offset to 0
//      otherwise set file_offset_ to 1
void LataDB::check_all_data_fileoffsets(entier split_files)
{
  Noms existing_filenames;
  ArrOfInt counts; // For each filenames, number of fields referring to it
  counts.set_smart_resize(1);

  const entier nb_tsteps = timesteps_.size();
  for (entier i = 0; i < nb_tsteps; i++) {
    LataVector<LataDBField> & fields = timesteps_[i].fields_;
    // Browse all fields:
    const entier nb_fields = fields.size();
    for (entier j = 0; j < nb_fields; j++) {
      LataDBField & field = fields[j];
      const entier rank = existing_filenames.rang(field.filename_);
      if (rank < 0) {
        // New filename
        existing_filenames.add(field.filename_);
        counts.append_array(1);
        field.datatype_.file_offset_ = 0;
        Journal(verb_level+1) << " Changing fileoffset to 0 for file " << field.filename_ 
                              << " " << field.name_ << endl;
      } else {
        // Existing filename
        if (split_files) {
          entier n = counts[rank]++;
          field.filename_ += "_";
          field.filename_ += Nom(n);
          field.datatype_.file_offset_ = 0;
          Journal(verb_level+1) << " Changing fileoffset to 0 and renaming file " << field.filename_  
                                << " " << field.name_ << endl;
        } else {
          field.datatype_.file_offset_ = 1;
          Journal(verb_level+1) << " Changing fileoffset to 1 for file " << field.filename_  
                                << " " << field.name_ << endl; 
        }
      }
    }
  }  
}

// Returns the rank of the created timestep (always at the end)
entier LataDB::add_timestep(double time)
{
  const entier n = nb_timesteps();
  // Timestep 0 can have any time: test only versus other timesteps:
  if (n > 1 && time <= get_time(n-1)) {
    Journal() << "Error in LataDB::add_timestep(" << time 
              << "): time is below or equal to last timestep " << get_time(n-1) << endl;
    throw(LataDBError(LataDBError::INVALID_OPERATION));
  }
  LataDBTimestep & t = timesteps_.add(LataDBTimestep());
  t.time_ = time;
  Journal(verb_level+1) << "LataDB::add_timestep " << n << " " << time << endl;
  return n;
}

static void add_geom_check(const LataDBGeometry & geom, entier test_flag, const char *message)
{
  if (!test_flag) {
    Journal() << "Error in LataDB::add_geometry, name_=" << geom.name_ << endl
              << " geometry data is invalid because of: " << message << endl;
    throw(LataDBError(LataDBError::INVALID_OPERATION));
  }
}

void LataDB::add_geometry(const LataDBGeometry & geom)
{  
  add_geom_check(geom, geom.timestep_ >= 0 && geom.timestep_ < nb_timesteps(), "timestep");
  Noms geoms= geometry_names(geom.timestep_, CURRENT);
  add_geom_check(geom, geom.name_ != "" && geom.name_ != "??" && geoms.rang(geom.name_) < 0, "empty or already existing name"); 

  add(geom.timestep_, geom);
  Journal(verb_level+1) << "LataDB::add_geometry " << geom.name_ << endl;
}

void LataDB::set_elemtype(entier tstep, const char *geom_name, const char *elem_type)
{
  LataDBGeometry & geom = (LataDBGeometry&) get_geometry(tstep, geom_name);
  geom.elem_type_ = elem_type;
}


static void add_field_check(const LataDBField & field, entier test_flag, const char *message)
{
  if (!test_flag) {
    Journal() << "Error in LataDB::add_field, name_=" << field.name_ << " geometry=" << field.geometry_ << endl
              << " field data is invalid because of: " << message << endl;
    throw(LataDBError(LataDBError::INVALID_OPERATION));
  }
}

// Adds a new field to the database.
// The field.datatype_.file_offset_ will be interpreted in a particular way if the data is
//  written with write_data(), see write_data() documentation.
// Take special care if the same file is referenced more than once in the database:
//  only one file should have file_offset_ <= 0 and this one will have to be written first 
//  with write_data()  (or you know what you are doing...)
void LataDB::add_field(const LataDBField & field)
{
  add_field_check(field, field.timestep_ >= 0 && field.timestep_ < nb_timesteps(), "timestep");
  add_field_check(field, field.filename_ != "" && field.filename_ != "??", "filename");
  add_field_check(field, field.nb_comp_ > 0, "nb_comp");
  Noms geoms = geometry_names(field.timestep_, FIRST_AND_CURRENT);
  add_field_check(field, field.geometry_ == "" || geoms.rang(field.geometry_) >= 0, "unknown geometry name");
  add_field_check(field, field.name_ != "" && field.name_ != "??", "empty name"); 
  add_field_check(field, field.component_names_.size() == 0 || field.component_names_.size() == field.nb_comp_, "number of component_names");
  add_field_check(field, field.size_ >= 0, "size");
  add_field_check(field, field.datatype_.msb_ != LataDBDataType::UNKNOWN_MSB, "datatype msb unspecified");
  add_field_check(field, field.datatype_.type_ == LataDBDataType::INT32
                  || field.datatype_.type_ == LataDBDataType::INT64
                  || field.datatype_.type_ == LataDBDataType::REAL32
                  || field.datatype_.type_ == LataDBDataType::REAL64, "datatype type unspecified");
  // If integer type, we must say the indexing type:
  add_field_check(field, 
                  field.datatype_.type_ == LataDBDataType::REAL32
                  || field.datatype_.type_ == LataDBDataType::REAL64
                  || field.datatype_.array_index_ != LataDBDataType::UNKNOWN_ARRAYINDEX, 
                  "datatype array indexing unspecified");
  add_field_check(field, field.datatype_.data_ordering_ != LataDBDataType::UNKNOWN_ORDERING, "datatype data ordering unspecified");
  add_field_check(field, field.datatype_.fortran_bloc_markers_ != LataDBDataType::UNKNOWN_MARKERS, "datatype fortran bloc markers unspecified");
  add_field_check(field, field.datatype_.file_offset_ >= 0, "datatype file_offset_");
  // ouf...
  add(field.timestep_, field);
  Journal(verb_level+1) << "LataDB::add_field : " << field.name_ << " " << field.geometry_ << " " << field.filename_ << " " << field.uname_ << endl;
}

LataDBDataType LataDB::default_type_float() const
{
  LataDBDataType type = default_type_int_;
  type.type_ = default_float_type_;
  return type;
}

// Description: Writes the lata master file to filename (filename must contain the path
//  if you don't want to write in the current working directory). All data contained
//  in the database is dumped to the file. 
void LataDB::write_master_file(const char *filename) const
{
  if (!filename) {
    Journal() << "LataDB::write_master_file got a null filename !!!" << endl;
    throw(LataDBError(LataDBError::INVALID_OPERATION));    
  }
  std::ofstream os(filename);
  if (!os.good()) { // isnogood ?
    Journal() << "LataDB::write_master_file failed opening file " << filename << endl;
    throw(LataDBError(LataDBError::FILE_NOT_FOUND));
  }
  // Try to write, if error, catch and close the file:
  Journal(verb_level-1) << "Writing lata master file:" << filename << endl;
  os << "LATA_V2.1" << endl;
  os << case_ << endl;
  os << software_id_ << endl;
  
  // ****************************************************************
  // Writing data format information:
  {
    Motcle fmt, fmt2;
    build_format_string(LataDBDataType(), default_type_int_, fmt);
    build_format_string(default_type_int_, default_type_float(), fmt2);
    os << "Format " << fmt << "," << fmt2 << endl;
  }

  // ***************************************************************
  // Writing timesteps:
  const entier nb_tsteps = nb_timesteps();
  for (entier tstep = 0; tstep < nb_tsteps; tstep++) {
    if (tstep > 0)
      os << "TEMPS " << get_time(tstep) << endl;

    Noms geoms = geometry_names(tstep);
    const entier nb_geoms = geoms.size();
    for (entier i_geom = 0; i_geom < nb_geoms; i_geom++) {
      const LataDBGeometry & geom = get_geometry(tstep, geoms[i_geom], FIRST_AND_CURRENT);
      // Do not write geometries of the first timestep
      if (geom.timestep_ == tstep)
        os << "GEOM  " << geom.name_ << " type_elem=" << geom.elem_type_ << endl;
    }
    Field_UNames unames = field_unames(tstep, "*", "*");
    for (entier i_field = 0; i_field < unames.size(); i_field++) {
      const LataDBField & field = get_field(tstep, unames[i_field]);
      os << "CHAMP " << field.name_ 
         << " " << field.filename_;
      if (field.geometry_ != "")
        os << " geometrie=" << field.geometry_;
      os << " size=" << field.size_;
      os << " composantes=" << field.nb_comp_;
      if (field.localisation_ != "??" && field.localisation_ != "")
        os << " localisation=" << field.localisation_;
      if (field.component_names_.size() > 0) {
        os << " noms_compo=";
        const entier n = field.component_names_.size();
        for (entier i = 0; i < n; i++) {
          os << field.component_names_[i];
          if (i < n-1)
            os << ",";
        }
      }
      switch(field.nature_) {
      case LataDBField::UNKNOWN: break;
      case LataDBField::SCALAR: os << " nature=scalar"; break;
      case LataDBField::VECTOR: os << " nature=vector"; break;
      default:
        Journal() << "LataDB::write_master_file error: unknown NATURE" << endl;
        throw(LataDBError(LataDBError::INVALID_OPERATION));
      }
      if (field.reference_ != "" && field.reference_ != "??")
        os << " reference=" << field.reference_;
      Motcle format_string;
      build_format_string(default_type_float(), field.datatype_, format_string);
      if (format_string != "") 
        os << " format=" << format_string;
      if (field.datatype_.file_offset_ > 0)
        os << " file_offset=" << field.datatype_.file_offset_;
      os << endl;
    }
  }
  os << "FIN" << endl;
  write_master_file_to_call_ = 0;
}

// Description: internal template to write a data block. We provide explicit methods write_data()
//  to the user instead of a template.
template <class C_Tab>
FileOffset LataDB::write_data_(entier tstep, const Field_UName & uname, const C_Tab & data)
{
  LataDBField & fld = getset_field(tstep, uname);

  LataDataFile f(internal_data_buffer_, path_prefix_, fld.filename_,
                 fld.datatype_.msb_,
                 (fld.datatype_.file_offset_ <= 0) ? LataDataFile::WRITE : LataDataFile::APPEND);
  fld.datatype_.file_offset_ = f.position();
  Journal(verb_level_data_bloc) << "Writing block data at offset " << fld.datatype_.file_offset_ << endl;
  if (fld.nb_comp_ != data.dimension(1) || fld.size_ != data.dimension(0)) {
    Journal() << "Error in LataDB::write_data_: nb_comp_ or size_ declared in the field doesnt match array dimensions." << fld.name_ << endl;
    throw;
  }

  const entier n = fld.size_;
  
  switch (fld.datatype_.data_ordering_) {
  case LataDBDataType::C_ORDERING: 
    {
      if (fld.datatype_.fortran_bloc_markers_ == LataDBDataType::BLOC_MARKERS_MULTIPLE_WRITES) {
        Journal() << "Error in LataDB::write_data_: fortran_bloc_markers_=MULTIPLE_WRITES is incompatible with data_ordering=C" << endl;
        throw LataDBError(LataDBError::DATA_ERROR);
      }
      const entier sz = data.size_array();
      write_blocksize(f, fld.datatype_, sz);
      bloc_write(f, fld.datatype_.msb_, fld.datatype_.type_, data, fld.nb_comp_);
      write_blocksize(f, fld.datatype_, sz);
      break;
    }
  case LataDBDataType::F_ORDERING:
    {
      entier multiple_bloc_markers = (fld.datatype_.fortran_bloc_markers_ == LataDBDataType::BLOC_MARKERS_MULTIPLE_WRITES);
      // reverse rows and columns of the array
      C_Tab tmp;
      tmp.resize(n, 1);
      if (!multiple_bloc_markers)
        write_blocksize(f, fld.datatype_, data.size_array());
      for (entier i = 0; i < fld.nb_comp_; i++) {
        if (multiple_bloc_markers)
          write_blocksize(f, fld.datatype_, n);
        for (entier j = 0; j < n; j++)
          tmp(j, 0) = data(j, i);
        bloc_write(f, fld.datatype_.msb_, fld.datatype_.type_, tmp, 1);
        if (multiple_bloc_markers)
          write_blocksize(f, fld.datatype_, n);
      }
      if (!multiple_bloc_markers)
        write_blocksize(f, fld.datatype_, data.size_array());
      break;
    }
  default:
    Journal() << "Error in LataDB::write_data_: data_ordering not implemented" << endl;
    throw;
  }
  write_master_file_to_call_ = 1;
  return f.position();
}

// Writes the data to disk according to datatype_ of the field.
// The filename will be "path_prefix_ + field.filename_".
// The path_prefix_ can be changed with set_path_prefix()
// If field.datatype_.file_offset_<=0, any existing file is deleted and the data is written at offset 0
// otherwise the data is written at the end of the file and file_offset_ for this field is updated.
// Returns the FileOffset of the file pointer after writing the data (points to the end of the file)
// The call to write_master_file() must be done after all write_data (otherwise the file_offset_ might be wrong)
FileOffset LataDB::write_data(entier tstep, const Field_UName & uname, const DoubleTab &tab)
{
  Journal() << " LataDB::write_data not coded for double" << endl;
//  exit(-1);
  return 0;
}

// See write_data(..., const DoubleTab &)
FileOffset LataDB::write_data(entier tstep, const Field_UName & uname, const FloatTab &tab)
{
  return write_data_(tstep, uname, tab);
}

// See write_data(..., const DoubleTab &)
FileOffset LataDB::write_data(entier tstep, const Field_UName & uname, const IntTab &tab)
{
  if (get_field(tstep, uname).datatype_.array_index_ == LataDBDataType::F_INDEXING) {
    IntTab tmp;
    tmp.set_smart_resize(1);
    tmp.resize(tab.dimension(0), tab.dimension(1));
    ArrOfInt & array = tmp;
    const ArrOfInt & src = tab;
    for (entier i = 0; i < array.size_array(); i++)
      array[i] = src[i] + 1;
    return write_data_(tstep, uname, tmp);
  }

  return write_data_(tstep, uname, tab);
}

LataDB::~LataDB()
{
#if 0
  if (write_master_file_to_call_) {
    Journal() << "Internal Error !!! write_data() has been called without calling write_master_file() after." << endl;
//    exit(-1); // In c++ it is forbidden to throw exceptions in a destructor.
  }
#endif
}

const char *LataDBError::describe() const
{
  switch(err_) {
  case READ_ERROR: return "READ_ERROR"; break;
  case BAD_HEADER: return "BAD_HEADER"; break;
  case BAD_TIMESTEP: return "BAD_TIMESTEP"; break;
  case NAME_NOT_FOUND: return "NAME_NOT_FOUND"; break;
  case DATA_ERROR: return "DATA_ERROR"; break;
  case FILE_NOT_FOUND: return "FILE_NOT_FOUND"; break;
  case BAD_ELEM_TYPE: return "BAD_ELEM_TYPE"; break;
  case INVALID_OPERATION: return "INVALID_OPERATION"; break;
  case INTEGER_OVERFLOW: return "INTEGER_OVERFLOW"; break;
  default: ;
  }
  return "LataDB_unknown_error";
}
#undef verb_level
#undef verb_level_data_bloc
