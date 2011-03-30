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

#include <LataWriter.h>
#include <LataStructures.h>

// Path, if not empty, must include a trailing '/'
// basename must not include the .lata extension
void LataWriter::init_file(const Nom & path, const Nom & basename,
                           const LataDBDataType & default_int_format, 
                           LataDBDataType::Type default_float_type)
{
  db_.reset();
  db_.set_path_prefix(path);
  basename_ = basename;
  db_.default_type_int_ = default_int_format;
  db_.default_float_type_ = default_float_type;
  db_.header_ = "Lata V2";
  db_.case_ = "lata2dx";
  db_.software_id_ = "Trio_U";
  //split_ = split;

  // Global geometries and fields:
  db_.add_timestep(0.);
}

// Add a new timestep to the lata database (new TEMPS entry)
// Geometries and fields are always written in the last added timestep
//  (the timestep stored within the domain or field is ignored)
// Those written before the first call to write_time() go into global
//  fields and geometry definitions.
void LataWriter::write_time(double t)
{
  db_.add_timestep(t);
}

void LataWriter::write_geometry(const Domain & dom)
{
  // Index of the last timestep:
  const entier tstep = db_.nb_timesteps() - 1;

  // Build a geometry database entry and add it to database
  LataDBGeometry geom;
  geom.name_ = dom.id_.name_;
  geom.elem_type_ = dom.element_type_to_string(dom.elt_type_);
  geom.timestep_ = tstep;
  db_.add_geometry(geom);

  // Write geometry data
  const DomainUnstructured * dom1_ptr = dynamic_cast<const DomainUnstructured*>(&dom);
  const DomainIJK * dom2_ptr = dynamic_cast<const DomainIJK*>(&dom);
  
  if (dom1_ptr)
    {
      // For unstructured meshes, we write the following fields:
      //  SOMMETS
      //  ELEMENTS
      //  [ FACES ]
      //  [ ELEM_FACES ]
      const DomainUnstructured & domain = *dom1_ptr;
      LataDBField field;
      // Write nodes
      Nom fieldname = "SOMMETS";
      field.uname_ = Field_UName(geom.name_, fieldname, "" /* localisation */);
      field.name_ = fieldname;
      field.timestep_ = tstep;

      field.filename_ = basename_;
      field.filename_ += ".lata.";
      field.filename_ += fieldname;
      field.filename_ += ".";
      field.filename_ += geom.name_;
      if (tstep > 0) { 
        field.filename_ += ".";
        field.filename_ += Nom(tstep);
      }
      field.nb_comp_ = domain.dimension();
      field.geometry_ = geom.name_;
      field.datatype_ = db_.default_type_float();
      field.localisation_ = "";
      field.reference_ = "";
      field.size_ = domain.nb_nodes();

      db_.add_field(field);
      db_.write_data(tstep, field.uname_, domain.nodes_);

      // Write elements
      fieldname = "ELEMENTS";
      field.uname_ = Field_UName(geom.name_, fieldname, "" /* localisation */);
      field.name_ = fieldname;
      field.timestep_ = tstep;

      field.filename_ = basename_;
      field.filename_ += ".lata.";
      field.filename_ += fieldname;
      field.filename_ += ".";
      field.filename_ += geom.name_;
      if (tstep > 0) { 
        field.filename_ += ".";
        field.filename_ += Nom(tstep);
      }
      field.nb_comp_ = domain.elements_.dimension(1);
      field.geometry_ = geom.name_;
      field.datatype_ = db_.default_type_int_;
      field.localisation_ = "";
      field.reference_ = "SOMMETS";
      field.size_ = domain.nb_elements();
      
      db_.add_field(field);
      db_.write_data(tstep, field.uname_, domain.elements_);

      // Write faces
      if (domain.faces_ok()) {
       fieldname = "FACES";
       field.uname_ = Field_UName(geom.name_, fieldname, "" /* localisation */);
       field.name_ = fieldname;
       field.timestep_ = tstep; 

       field.filename_ = basename_;
       field.filename_ += ".lata.";
       field.filename_ += fieldname;
       field.filename_ += ".";
       field.filename_ += geom.name_;
       if (tstep > 0) { 
        field.filename_ += ".";
        field.filename_ += Nom(tstep);
       }
       field.nb_comp_ = domain.faces_.dimension(1);
       field.geometry_ = geom.name_;
       field.datatype_ = db_.default_type_int_;
       field.localisation_ = "";
       field.reference_ = "SOMMETS";
       field.size_ = domain.nb_faces();
      
       db_.add_field(field);
       db_.write_data(tstep, field.uname_, domain.faces_);

       fieldname = "ELEM_FACES";
       field.uname_ = Field_UName(geom.name_, fieldname, "" /* localisation */);
       field.name_ = fieldname;
       field.timestep_ = tstep; 

       field.filename_ = basename_;
       field.filename_ += ".lata.";
       field.filename_ += fieldname;
       field.filename_ += ".";
       field.filename_ += geom.name_;
       if (tstep > 0) { 
        field.filename_ += ".";
        field.filename_ += Nom(tstep);
       }
       field.nb_comp_ = domain.elem_faces_.dimension(1);
       field.geometry_ = geom.name_;
       field.datatype_ = db_.default_type_int_;
       field.localisation_ = "";
       field.reference_ = "FACES";
       field.size_ = domain.nb_elements();
      
       db_.add_field(field);
       db_.write_data(tstep, field.uname_, domain.elem_faces_);
     }
    }
  else if (dom2_ptr)
    {
      // For IJK we write 2 or 3 fields containing 1-dimensionnal arrays with
      //  the nodes coordinates in each direction:
      //  SOMMETS_IJK_I, SOMMETS_IJK_J, SOMMETS_IJK_K.

      const DomainIJK & domain = *dom2_ptr;
      // Write coordinates
      const entier dim = domain.coord_.size();
      if (dim > 3) {
        Journal() << "Error in LataWriter::write_geometry: dimension > 3" << endl;
        throw InternalError;
      }
      Noms dir_names(3);
      dir_names[0] = "I";
      dir_names[1] = "J";
      dir_names[2] = "K";
      for (entier i_dim = 0; i_dim < dim; i_dim++) {
        FloatTab coord;
        {
          const ArrOfFloat & x = domain.coord_[i_dim];
          const entier n = x.size_array();
          coord.resize(n, 1);
          for (entier i = 0; i < n; i++)
            coord(i, 0) = x[i];
        }
        
        Nom fieldname = "SOMMETS_IJK_";
        fieldname += dir_names[i_dim];
        LataDBField field;
        field.uname_ = Field_UName(geom.name_, fieldname, "" /* localisation */);
        field.name_ = fieldname;
        field.timestep_ = tstep;

        field.filename_ = basename_;
        field.filename_ += ".lata.";
        field.filename_ += fieldname;
        field.filename_ += ".";
        field.filename_ += geom.name_;
        if (tstep > 0) { 
          field.filename_ += ".";
          field.filename_ += Nom(tstep);
        }
        field.nb_comp_ = 1;
        field.geometry_ = geom.name_;
        field.datatype_ = db_.default_type_float();
        field.localisation_ = "";
        field.reference_ = "";
        field.size_ = coord.dimension(0);

        db_.add_field(field);
        db_.write_data(tstep, field.uname_, coord);
      }
      
      if (domain.invalid_connections_.size_array() > 0) {
        const entier n = domain.invalid_connections_.size_array();
        IntTab tmp(n, 1);
        for (entier i = 0; i < n; i++) 
          tmp(i, 0) = domain.invalid_connections_[i];

        Nom fieldname = "INVALID_CONNECTIONS";
        LataDBField field;
        field.uname_ = Field_UName(geom.name_, fieldname, "ELEM" /* localisation */);
        field.name_ = fieldname;
        field.timestep_ = tstep;

        field.filename_ = basename_;
        field.filename_ += ".lata.";
        field.filename_ += fieldname;
        field.filename_ += ".";
        field.filename_ += geom.name_;
        if (tstep > 0) { 
          field.filename_ += ".";
          field.filename_ += Nom(tstep);
        }
        field.nb_comp_ = 1;
        field.geometry_ = geom.name_;
        field.datatype_ = db_.default_type_int_;
        field.datatype_.array_index_ = LataDBDataType::NOT_AN_INDEX;
        field.localisation_ = "ELEM";
        field.reference_ = "";
        field.size_ = n;

        db_.add_field(field);
        db_.write_data(tstep, field.uname_, tmp);
      }
    }
  else
    {
      Journal() << "Error LataWriter::write_geometry domain type not supported" << endl;
      throw InternalError;
    }
}

void LataWriter::write_component(const LataField_base & field)
{
  // Index of the last timestep:
  const entier tstep = db_.nb_timesteps() - 1;
  
  LataDBField lata_field;
  
  lata_field.uname_ = field.id_.uname_;
  lata_field.name_ = field.id_.uname_.get_field_name();
  lata_field.timestep_ = tstep;
  lata_field.filename_ = basename_;
  lata_field.filename_ += ".lata.";
  lata_field.filename_ += lata_field.uname_.build_string();
  if (tstep > 0) { 
    lata_field.filename_ += ".";
    lata_field.filename_ += Nom(tstep);
  }
  lata_field.geometry_ = field.id_.uname_.get_geometry();
  lata_field.component_names_ = field.component_names_;
  // Unites a remplir
  // Size = -1 => valeur par defaut cherchee dans la geometrie
  lata_field.localisation_ = LataField_base::localisation_to_string(field.localisation_);
  lata_field.nature_ = field.nature_;

  const Field<FloatTab> * float_f = dynamic_cast<const Field<FloatTab>*>(&field);
  const Field<IntTab>* int_f = dynamic_cast<const Field<IntTab>*>(&field);
  if (int_f) {
    lata_field.nb_comp_ = int_f->data_.dimension(1);
    lata_field.size_ = int_f->data_.dimension(0);
    lata_field.datatype_ = db_.default_type_int_;
    lata_field.datatype_.array_index_ = LataDBDataType::NOT_AN_INDEX;
    db_.add_field(lata_field);
    db_.write_data(tstep, lata_field.uname_, int_f->data_);
  } else if (float_f) {
    lata_field.nb_comp_ = float_f->data_.dimension(1);
    lata_field.size_ = float_f->data_.dimension(0);
    lata_field.datatype_ = db_.default_type_float();
    db_.add_field(lata_field);
    db_.write_data(tstep, lata_field.uname_, float_f->data_);
  }
}

void LataWriter::finish()
{
  Nom n(db_.path_prefix());
  n += basename_;
  n += ".lata";
  db_.write_master_file(n);
}
