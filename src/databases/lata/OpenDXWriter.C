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

#include <OpenDXWriter.h>
#include <LataFilter.h>
#include <iostream>
#include <fstream>
#include <iosfwd>

class DX_stream;

class DX_stream
{
public:
  DX_stream() : os_to_cout_(0), os_(0) {};
  void init_cout(int ascii) 
  { 
    reset();
    os_to_cout_ = 1;
    ascii_ = ascii;
    os_ = &std::cout;
  }
  void init_file(const char *fname, int ascii)
  {
    reset();
    os_to_cout_ = 0;
    ascii_ = ascii;
    os_ = new std::ofstream(fname);
  }
  ~DX_stream() { reset(); }
  void reset()
  {
    if (!os_to_cout_)
      delete os_;
    os_ = 0;
    os_to_cout_ = 0;
  }
  DX_stream & operator<<(const float f) { (*os_) << f; return *this; }
  DX_stream & operator<<(const int i) { (*os_) << i; return *this; }
  DX_stream & operator<<(const char * s) { (*os_) << s; return *this; }
  DX_stream & operator<<(DX_stream & f(DX_stream &)) { return f(*this); }

  void write(char * ptr, int sz) { os_->write(ptr, sz); }
  entier ok() { return os_ != 0; }
  entier ascii() { return ascii_; }
  std::ostream & stream() { return *os_; }
protected:
  int os_to_cout_;
  int ascii_;
  std::ostream *os_;
};

DX_stream & endl(DX_stream & os) 
{ 
  os.stream() << std::endl; 
  return os; 
}

void DX_write_vect(DX_stream & os, int dxobject, const ArrOfFloat & v)
{
  const int places = v.size_array();
  os << "object " << dxobject << " class array" << endl;
  os << "type float rank 1 shape 1 items " << places << " ";

  if (!os.ascii()) {
    os << (mymachine_msb ? "msb ieee" : "lsb ieee") << " data follows" << endl;
    os.write((char*)v.addr(), sizeof(float) * places);
  } else {
    os << "ascii data follows" << endl;
    for (int i=0;i<places;i++) {
      os << v[i] << " ";
      os << endl;
    }
  }
}
void DX_write_vect(DX_stream & os, int dxobject, const FloatTab & v)
{
  const int places = v.dimension(0);
  const int shape  = v.dimension(1);
  os << "object " << dxobject << " class array" << endl;
  os << "type float rank 1 shape " << shape << " items " << places << " ";

  if (!os.ascii()) {
    os << (mymachine_msb ? "msb ieee" : "lsb ieee") << " data follows" << endl;
    os.write((char*)v.addr(), sizeof(float) * places * shape);
  } else {
    os << "ascii data follows" << endl;
    for (int i=0;i<places;i++) {
      for (int j=0;j<shape;j++)
        os << v(i, j) << " ";
      os << endl;
    }
  }
}

void DX_write_vect(DX_stream & os, int dxobject, const IntTab & v)
{
  if (sizeof(int) != 4) {
    Journal() << "Error DX_write_vect : int size != 32 bits" << endl;
    throw OpenDXWriter::DXInternalError;
  }
  const int places = v.dimension(0);
  const int shape  = v.dimension(1);
  os << "object " << dxobject << " class array" << endl;
  os << "type int rank 1 shape " << shape << " items " << places << " ";

  if (!os.ascii()) {
    os << (mymachine_msb ? "msb ieee" : "lsb ieee") << " data follows" << endl;
    os.write((char*)v.addr(), sizeof(int) * places * shape);
  } else {
    os << "ascii data follows" << endl;
    for (int i=0;i<places;i++) {
      for (int j=0;j<shape;j++)
        os << v(i, j) << " ";
      os << endl;
    }
  }
}

OpenDXWriter::OpenDXWriter()
{
  os_ = new DX_stream;
}

OpenDXWriter::~OpenDXWriter()
{
  delete os_;
}

void OpenDXWriter::reset()
{
  index_counter_ = 0;
  fields_indexes_.resize_array(0);
  fields_indexes_.set_smart_resize(1);
  fields_names_ = Noms();
  nodes_index_ = -1;
  finish_geometry(); // reset geometry data
}

void OpenDXWriter::init_cout(double time, int ascii)
{
  reset();
  os_->init_cout(ascii);
  dx_time_index_ = ++index_counter_;
  FloatTab t;
  t.resize(1,1);
  t(0,0) = time;
  DX_write_vect(*os_, dx_time_index_, t);
}

void OpenDXWriter::init_file(double time, Nom & filename_, int ascii)
{
  reset();
  os_->init_file(filename_, ascii);
  dx_time_index_ = ++index_counter_;
  FloatTab t;
  t.resize(1,1);
  t(0,0) = time;
  DX_write_vect(*os_, dx_time_index_, t);
}

const char * DX_element_name(Domain::Element elem)
{
  switch(elem) {
  case Domain::line: return "lines";
  case Domain::triangle: return "triangles";
  case Domain::quadri: return "quads";
  case Domain::tetra: return "tetrahedra";
  case Domain::hexa: return "cubes";
  default:
    Journal() << "DX_element_name unknown element" << endl;
    throw OpenDXWriter::DXInternalError;
  }
}

void OpenDXWriter::write_geometry(const Domain & dom)
{
  // Write last geometry and begin a new one
  finish_geometry();
  const DomainUnstructured * dom1 = dynamic_cast<const DomainUnstructured*>(&dom);
  const DomainIJK * dom2 = dynamic_cast<const DomainIJK*>(&dom);
  DX_stream & os = *os_;
  if (dom1) {
    nodes_index_ = ++index_counter_;
    DX_write_vect(os, nodes_index_, dom1->nodes_);
    elements_index_ = ++index_counter_;
    DX_write_vect(os, elements_index_, dom1->elements_);
    os << "attribute \"element type\" string \"" << DX_element_name(dom.elt_type_) << "\"" << endl;
    os << "attribute \"ref\" string \"positions\"" << endl;
  } else if (dom2) {
    const entier dim = dom.dimension();
    ArrOfInt dx_coord_index(dim);
    for (entier i = 0; i < dim; i++) {
      dx_coord_index[i] = ++index_counter_;
      const entier n = dom2->coord_[i].size_array();
      FloatTab tmp;
      tmp.resize(n, 3);
      for (entier j = 0; j < n; j++)
        tmp(j, i) = dom2->coord_[i][j];
      DX_write_vect(os, dx_coord_index[i], tmp);
    }
    nodes_index_ = ++index_counter_;
    os << "object " << nodes_index_ << " class productarray" << endl;
    entier i;
    for (i = dim-1; i >= 0; i--)
      os << " term " << dx_coord_index[i] << endl;
    elements_index_ = ++index_counter_;
    os << "object " << elements_index_ << " class gridconnections counts";
    for (i = dim-1; i >= 0; i--)
      os << " " << dom2->coord_[i].size_array();
    os << endl;
    os << "attribute \"element type\" string \"" << ((dim==2)?"quads":"cubes") << "\"" << endl;
    os << "attribute \"ref\" string \"positions\"" << endl;
    const entier n1 = dom2->invalid_positions_.size_array();
    if (n1 > 0) {
      invalid_positions_ = ++index_counter_;
      IntTab tmp;
      tmp.resize(n1, 1);
      ArrOfInt & array = tmp;
      for (entier i = 0; i < n1; i++) array[i] = dom2->invalid_positions_[i];
      DX_write_vect(os, invalid_positions_, tmp);
      os << "attribute \"ref\" string \"positions\"" << endl;
    }
#if 0
    if (n2 > 0) {
      IntTab tmp;
      tmp.resize(n2, 1);
      ArrOfInt & array = tmp;
      for (entier i = 0; i < n2; i++) array[i] = dom2->invalid_connections_[i];
      DX_write_vect(os, invalid_connections_, tmp);
      os << "attribute \"ref\" string \"connections\"" << endl;
    }
#endif
  } else {
    Journal() << "Error OpenDXWriter::write_geometry domain type not supported" << endl;
    throw DXInternalError;
  }
  fields_names_.add(dom.id_.name_);
}

void OpenDXWriter::finish_geometry()
{
  if (nodes_index_ >= 0) {
    index_counter_++;
    fields_indexes_.append_array(index_counter_);
    DX_stream & os = *os_;
    os << "object " << index_counter_ << " class field" << endl;
    os << " component \"positions\" " << nodes_index_ << endl;
    os << " component \"connections\" " << elements_index_ << endl;
    if (invalid_positions_ >= 0)
      os << " component \"invalid positions\" " << invalid_positions_ << endl;
    if (invalid_connections_ >= 0)
      os << " component \"invalid connections\" " << invalid_connections_ << endl;
    os << " component \"TIME\" " << dx_time_index_ << endl;

    for (entier i=0; i < components_indexes_.size_array(); i++)
      os << " component \"" << components_names_[i] << "\" " << components_indexes_[i] << endl;
  }
  nodes_index_ = -1;
  elements_index_ = -1;
  components_indexes_.resize_array(0);
  components_indexes_.set_smart_resize(1);
  components_names_ = Noms();
  invalid_positions_ = -1;
  invalid_connections_ = -1;
}

void OpenDXWriter::write_component(const LataField_base & field)
{
  index_counter_++;
  const Field<IntTab> * int_field = dynamic_cast<const Field<IntTab>*>(&field);
  const Field<FloatTab> * float_field = dynamic_cast<const Field<FloatTab>*>(&field);
  if (int_field)
    DX_write_vect(*os_, index_counter_, int_field->data_);
  else if (float_field)
    DX_write_vect(*os_, index_counter_, float_field->data_);
  else {
    Journal() << "Error OpenDXWriter::write_component: unknown field type" << endl;
    throw;
  }
  if (field.localisation_ == LataField_base::ELEM)
    (*os_) << "attribute \"dep\" string \"connections\"" << endl;
  else if (field.localisation_ == LataField_base::SOM)
    (*os_) << "attribute \"dep\" string \"positions\"" << endl;
  else
    ; // no attribute
  components_indexes_.append_array(index_counter_);
  Nom n = field.id_.uname_.get_field_name();
  n += "_";
  n += field.id_.uname_.get_localisation();
  components_names_.add(n);
}

entier OpenDXWriter::finish(int force_group)
{
  DX_stream & os = *os_;
  finish_geometry();
  if (force_group || fields_indexes_.size_array() > 1) {
    DX_stream & os = *os_;
    os << "object " << ++index_counter_ << " class group" << endl;
    for (entier i = 0; i < fields_indexes_.size_array(); i++) {
      os << " member \"" << fields_names_[i] << "\" value " << fields_indexes_[i] << endl;
    }
  }
  os << "END" << endl;
  return index_counter_;
}
