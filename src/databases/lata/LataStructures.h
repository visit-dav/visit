/****************************************************************************
* Copyright (c) 2022, CEA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#ifndef LataStructures_H
#define LataStructures_H

#include <ArrOfBit.h>
#include <LataTools.h>
#include <LataDB.h>

// This file contains definitions of data structures containind meshes and fields
//  used by LataFilter.

// Description: Domain_Id is what you need to identify the content
//  of a Domain object (at this time, the domain name, the timestep and the
//  parallel sub_block number)
class Domain_Id
{
public:
  Domain_Id(const char *name = "??", int t = 0, int block = -1) : name_(name), timestep_(t), block_(block) { }
  // Domain name
  Nom name_;
  // At which timestep (needed for dynamic domains)
  int timestep_;
  // Which block of the parallel computation ? -1 => all blocks
  int block_;
};

// Description: Field_Id is what you need to identify the content of a
//  LataField_base structure (at this time, the field uname,
//  the timestep and the parallel sub_block number)
class Field_Id
{
public:
  Field_Id() : timestep_(0), block_(-1) { }
  Field_Id(const Field_UName& uname, int timestep, int block) : timestep_(timestep), block_(block), uname_(uname) { }

  int timestep_;
  int block_;
  Field_UName uname_;

  operator Domain_Id() const { return Domain_Id(uname_.get_geometry(), timestep_, block_); }
};

// Description: This structure contains a discrete data array for a specific
//  field, at one timestep, for one sub_block of the geometry, with
//  one localisation (but many components)
class LataField_base: public LataObject
{
public:
  LataField_base()
  {
    localisation_ = UNKNOWN;
    nature_ = LataDBField::UNKNOWN;
  }
  Field_Id id_;
  Noms component_names_;
  enum Elem_som { ELEM, SOM, FACES, UNKNOWN };
  Elem_som localisation_;
  LataDBField::Nature nature_;

  static Elem_som localisation_from_string(const Motcle&);
  static Nom localisation_to_string(const Elem_som);
};
class DomainUnstructured;
class DomainIJK;

// This class stores the geometry of a domain
class Domain: public LataObject
{
public:
  Domain_Id id_;
  enum Element { point, line, triangle, quadri, tetra, hexa, prism6, polyedre, polygone, unspecified };
  enum DomainType { IJK, UNSTRUCTURED };
  static Element element_type_from_string(const Motcle& type_elem);
  static Nom element_type_to_string(Element type);
  Element elt_type_;

  Domain() :
    elt_type_(unspecified), decal_nodes_lata_(-1), // -1 indicates: value not set. see lata_block_offset
    decal_elements_lata_(-1), decal_faces_lata_(-1) { } ;
  DomainType get_domain_type() const;
  const DomainUnstructured& cast_DomainUnstructured() const;
  const DomainIJK& cast_DomainIJK() const;
  virtual int dimension() const = 0;
  virtual trustIdType nb_nodes() const = 0;
  virtual trustIdType nb_elements() const = 0;
  virtual trustIdType nb_faces() const = 0;
  virtual trustIdType nb_items(const LataField_base::Elem_som) const;
  virtual trustIdType lata_block_offset(const LataField_base::Elem_som) const;
  virtual void set_lata_block_offset(const LataField_base::Elem_som, trustIdType n);

  virtual void fill_field_from_lataDB(const LataDB& lataDB, const Field_Id& id, LataDeriv<LataField_base>& field) const = 0;

  static Motcle lata_element_name(Domain::Element type);

  const bool& is_rectangle() const { return is_rectangle_;}

protected:
  // If the Domain has been loaded from a lata file and it's not the
  // first block this is the offset in the lata file:
  trustIdType decal_nodes_lata_;
  trustIdType decal_elements_lata_;
  trustIdType decal_faces_lata_;
  bool is_rectangle_ = false;
};

class DomainUnstructured: public Domain
{
public:
  DomainUnstructured()
  {
    nb_virt_nodes_ = 0;
    nb_virt_elements_ = 0;
    nb_virt_faces_ = 0;
  }

  BigFloatTab nodes_;
  // For each element, indexes of the nodes (first node is at index 0)
  // Nodes ordering in an element is the same as in TRUST
  BigTIDTab elements_;
  // For each face, indexes of the nodes (if present in lata file)
  BigTIDTab faces_;
  // For each elements, indexes of the faces (first face at index 0, if present in lata file)
  // Faces ordering in an element is the same as in TRUST
  BigTIDTab elem_faces_;

  int dimension() const override { return (int)nodes_.dimension(1); }
  trustIdType nb_nodes() const override { return nodes_.dimension(0); }
  trustIdType nb_elements() const override { return elements_.dimension(0); }
  trustIdType nb_faces() const override { return faces_.dimension(0); }
  // Tests if the geometry contains faces description
  bool faces_ok() const { return elem_faces_.dimension(0) == elements_.dimension(0); }

  template<typename TabType>
  void compute_cell_center_coordinates(TabType& coord, trustIdType index_begin) const;

  Size_t compute_memory_size() const override { return memory_size(nodes_) + memory_size(elements_) + memory_size(faces_) + memory_size(elem_faces_); }

  const BigTIDTab& get_joints(LataField_base::Elem_som loc) const
  {
    const BigTIDTab *ptr = 0;
    switch(loc)
      {
      case LataField_base::SOM:
        ptr = &joints_sommets_;
        break;
      case LataField_base::ELEM:
        ptr = &joints_elements_;
        break;
      case LataField_base::FACES:
        ptr = &joints_faces_;
        break;
      default:
        throw;
      }
    if (ptr->dimension(1) == 0)
      throw;
    return *ptr;
  }

  BigTIDTab& set_joints(LataField_base::Elem_som loc)
  {
    BigTIDTab *ptr = 0;
    switch(loc)
      {
      case LataField_base::SOM:
        ptr = &joints_sommets_;
        break;
      case LataField_base::ELEM:
        ptr = &joints_elements_;
        break;
      case LataField_base::FACES:
        ptr = &joints_faces_;
        break;
      default:
        throw;
      }
    return *ptr;
  }

  const BigArrOfTID& get_virt_items(LataField_base::Elem_som loc) const
  {
    switch(loc)
      {
      case LataField_base::SOM:
        return virt_nodes_;
      case LataField_base::ELEM:
        return virt_elements_;
      case LataField_base::FACES:
        return virt_faces_;
      default:
        throw;
      }
  }

  void set_virt_items(LataField_base::Elem_som loc, const BigArrOfTID& list)
  {
    switch(loc)
      {
      case LataField_base::SOM:
        virt_nodes_ = list;
        nb_virt_nodes_ = list.size_array();
        break;
      case LataField_base::ELEM:
        virt_elements_ = list;
        nb_virt_elements_ = list.size_array();
        break;
      case LataField_base::FACES:
        virt_faces_ = list;
        nb_virt_faces_ = list.size_array();
        break;
      default:
        throw;
      }
  }

  void set_nb_virt_items(LataField_base::Elem_som loc, trustIdType n)
  {
    switch(loc)
      {
      case LataField_base::SOM:
        nb_virt_nodes_ = n;
        break;
      case LataField_base::ELEM:
        nb_virt_elements_ = n;
        break;
      case LataField_base::FACES:
        nb_virt_faces_ = n;
        break;
      default:
        throw;
      }
  }

  trustIdType nb_virt_items(LataField_base::Elem_som loc) const
  {
    switch(loc)
      {
      case LataField_base::SOM:
        return nb_virt_nodes_;
      case LataField_base::ELEM:
        return nb_virt_elements_;
      case LataField_base::FACES:
        return nb_virt_faces_;
      default:
        throw;
      }
  }

  virtual void fill_domain_from_lataDB(const LataDB& lataDB, const Domain_Id& id, bool load_faces=true, bool merge_virtual_elements=false);
  virtual void fill_field_from_lataDB(const LataDB& lataDB, const Field_Id& id, LataDeriv<LataField_base>& field) const override;

protected:
  // data not always filled:
  BigTIDTab joints_sommets_;
  BigTIDTab joints_elements_;
  BigTIDTab joints_faces_;
  BigArrOfTID virt_nodes_; // Global indexes of virtual nodes to load
  BigArrOfTID virt_elements_; // Global indexes of virtual elements to load
  BigArrOfTID virt_faces_; // Global indexes of virtual faces to load
  trustIdType nb_virt_nodes_;
  trustIdType nb_virt_elements_;
  trustIdType nb_virt_faces_;
};

// This is a structured grid, grid axes aligned on X, Y and Z.
// The grid can have "invalid_positions_" and "invalid_connections_".
// Nodes are numbered like this:
//  node_index(i,j,k) = (k * nb_nodes_y + j) * nb_nodes_x + i
// Elements are numbered like this:
//  element_index(i,j,k) = (k * nb_elements_y + j) * nb_elements_x + i
// Faces are numbered like this: faces of each direction have a numbering starting at zero.
//  The number of a particular face is the smallest number of its nodes.
//  Hence some numbers are not used (le last face of each "row" depending on the
//  direction)
//
//  For IJK we make a strong assumption: the total number of elements, or faces, or nodes can be big (greater than 32bits)
//  **but** in each direction, the number of steps (or points) always fits within 32 bits.
class DomainIJK: public Domain
{
public:
  DomainIJK();
  // In each spatial direction, ordered list of coordinates of the IJK grid
  LataVector<ArrOfFloat> coord_;

  // For each node and each element, flag indicates if it is valid or not
  // (eg, has usable field values)
  // If array is empty, all data is valid.
  BigArrOfBit invalid_positions_;
  BigArrOfBit invalid_connections_;

  int dimension() const override { return coord_.size(); }
  trustIdType nb_nodes() const override
  {
    trustIdType n = 1;
    int d = coord_.size();
    for (int i = 0; i < d; i++)
      n *= coord_[i].size_array();
    return n;
  }

  trustIdType nb_elements() const override
  {
    trustIdType n = 1;
    int d = coord_.size();
    for (int i = 0; i < d; i++)
      n *= coord_[i].size_array() - 1;
    return n;
  }

  // Dimension(0) des tableaux de valeurs aux faces
  //  (voir convention sur la numerotation des faces)
  //  les champs associes aux faces des differentes directions sont
  //  stockes dans les composantes du champ.
  trustIdType nb_faces() const override { return nb_nodes(); }

  Size_t compute_memory_size() const override
  {
    Size_t x = 0;
    const int n = coord_.size();
    for (int i = 0; i < n; i++)
      x += memory_size(coord_[i]);
    return x + memory_size(invalid_positions_) + memory_size(invalid_connections_);
  }

  // renvoie le nombre de sommets dans la direction dir
  //  (renvoie 1 si dir >= dimension())
  int nb_som_dir(int dir) const
  {
    if (dir >= dimension())
      return 1;
    else
      return coord_[dir].size_array();
  }

  // renvoie le nombre d'elements dans la direction dir
  //  (renvoie 1 si dir >= dimension())
  int nb_elem_dir(int dir) const
  {
    if (dir >= dimension())
      return 1;
    else
      return coord_[dir].size_array() - 1;
  }

  virtual void fill_domain_from_lataDB(const LataDB& lataDB, const Domain_Id& id, const int split_in_nparts=1, const int virt_layer_size=1);
  void fill_field_from_lataDB(const LataDB& lataDB, const Field_Id& id, LataDeriv<LataField_base>& field) const override;

  // when loading fields, we will load elements (i,j,k) with
  //   part_begin_ <= k < part_end_
  // (or j in 2D), part_begin_ and part_end_ include the virtual layer
  int part_begin_;
  int part_end_;
  // number of layers of virtual elements at each side:
  int virtual_layer_begin_;
  int virtual_layer_end_;
};

template<class TabType>
class Field: public LataField_base
{
public:
  TabType data_;
  Size_t compute_memory_size() const override
  {
    return memory_size(data_);
  }
};
#endif
