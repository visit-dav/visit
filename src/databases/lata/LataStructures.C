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
#include <LataStructures.h>
#include <LataDB.h>
#include <stdlib.h>
static const int info_level = 4;

// Description: returns the number of items of the given type
trustIdType Domain::nb_items(const LataField_base::Elem_som loc) const
{
  trustIdType n = -1;
  switch (loc)
    {
    case LataField_base::SOM:
      n = nb_nodes();
      break;
    case LataField_base::ELEM:
      n = nb_elements();
      break;
    case LataField_base::FACES:
      n = nb_faces();
      break;
    default:
      Journal() << "Invalid localisation " << (int) loc << " in Domain::nb_items" << endl;
      throw;
    }
  return n;
}

// Description: returns the offset in the lata block on disk of the first
//  item for this Domain.id_.block_ (parallel computation).
//  (this value must be set with set_lata_block_offset)
trustIdType Domain::lata_block_offset(const LataField_base::Elem_som loc) const
{
  trustIdType n = -1;
  switch (loc)
    {
    case LataField_base::SOM:
      n = decal_nodes_lata_;
      break;
    case LataField_base::ELEM:
      n = decal_elements_lata_;
      break;
    case LataField_base::FACES:
      n = decal_faces_lata_;
      break;
    default:
      Journal() << "Invalid localisation " << (int) loc << " in Domain::lata_block_offset" << endl;
      throw;
    }
  if (n < 0)
    {
      Journal() << "Error: lata_block_offset not set for localisation " << (int) loc << endl;
      throw;
    }
  return n;
}

// Description: set the lata_block_offset (see lata_block_offset)
void Domain::set_lata_block_offset(const LataField_base::Elem_som loc, trustIdType n)
{
  switch (loc)
    {
    case LataField_base::SOM:
      decal_nodes_lata_ = n;
      break;
    case LataField_base::ELEM:
      decal_elements_lata_ = n;
      break;
    case LataField_base::FACES:
      decal_faces_lata_ = n;
      break;
    default:
      Journal() << "Invalid localisation " << (int) loc << " in Domain::set_lata_block_offset" << endl;
      throw;
    }
}

template<class TabType>
void DomainUnstructured::compute_cell_center_coordinates(TabType& coord, trustIdType index_begin) const
{
  using TYPE = typename TabType::Value_type_;    // type stored in TabType
  const int dim = (int)nodes_.dimension(1);
  const trustIdType nb_elem = elements_.dimension(0);
  const int nb_som_elem = (int)elements_.dimension(1);
  const TYPE facteur = (TYPE)(1.0 / (double)nb_som_elem);
  TYPE tmp[3];
  for (trustIdType i = 0; i < nb_elem; i++)
    {
      int j, k;
      tmp[0] = tmp[1] = tmp[2] = 0.;
      for (j = 0; j < nb_som_elem; j++)
        {
          trustIdType som = elements_(i, j);
          for (k = 0; k < loop_max(dim, 3); k++)
            {
              tmp[k] += nodes_(som, k);
              break_loop(k, dim);
            }
        }
      for (k = 0; k < loop_max(dim, 3); k++)
        {
          coord(index_begin + i, k) = tmp[k] * facteur;
          break_loop(k, dim);
        }
    }
}

template void DomainUnstructured::compute_cell_center_coordinates(BigFloatTab& coord, trustIdType index_begin) const;
template void DomainUnstructured::compute_cell_center_coordinates(BigDoubleTab& coord, trustIdType index_begin) const;

LataField_base::Elem_som LataField_base::localisation_from_string(const Motcle& loc)
{
  if (loc.debute_par("SOM"))
    return SOM;
  else if (loc.debute_par("ELEM"))
    return ELEM;
  else if (loc.debute_par("FACE"))
    return FACES;
  else
    return UNKNOWN;
}

Nom LataField_base::localisation_to_string(const Elem_som loc)
{
  Nom n;
  switch(loc)
    {
    case SOM:
      n = "SOM";
      break;
    case ELEM:
      n = "ELEM";
      break;
    case FACES:
      n = "FACES";
      break;
    case UNKNOWN:
      n = "";
    }
  return n;
}

Motcle Domain::lata_element_name(Domain::Element type)
{
  switch(type)
    {
    case point:
      return "POINT";
    case line:
      return "SEGMENT";
    case triangle:
      return "TRIANGLE";
    case quadri:
      return "QUADRANGLE";
    case tetra:
      return "TETRAEDRE";
    case hexa:
      return "HEXAEDRE";
    case prism6:
      return "PRISM6";
    case polyedre:
      return "POLYEDRE";
    case polygone:
      return "POLYGONE";
    default:
      return "UNSPECIFIED";
    }
}

Domain::Element Domain::element_type_from_string(const Motcle& type_elem)
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
  else if (type_elem == "SEGMENT_AXI")
    type=line;
  else if (type_elem == "TETRAEDRE")
    type=tetra;
  else if (type_elem == "TRIANGLE")
    type=triangle;
  else if (type_elem == "TRIANGLE_3D")
    type=triangle;
  else if (type_elem == "POINT")
    type=point;
  else if ((type_elem == "PRISM6")||(type_elem == "PRISME"))
    type=prism6;
  else if (type_elem.debute_par("POLYEDRE"))
    type=polyedre;
  else if (type_elem.debute_par("POLYGONE"))
    type=polygone;
  else
    {
      Journal() << "Error in elem_type_from_string: unknown element type " << type_elem << endl;
      throw;
    }
  return type;
}

Nom Domain::element_type_to_string(Element type)
{
  Nom n;
  switch(type)
    {
    case point:
      n = "POINT";
      break;
    case line:
      n = "SEGMENT";
      break;
    case triangle:
      n = "TRIANGLE";
      break;
    case quadri:
      n = "QUADRANGLE";
      break;
    case tetra:
      n = "TETRAEDRE";
      break;
    case hexa:
      n = "HEXAEDRE";
      break;
    case prism6:
      n = "PRISM6";
      break;
    case polyedre:
      n = "POLYEDRE_0";
      break;
    case polygone:
      n = "POLYGONE";
      break;
    case unspecified:
      n = "UNKNOWN";
      break;
    }
  return n;
}

// Description: read the specified geometry from the lataDB_ structure and put it in "dom".
//  load_faces: flag, tells if we should read faces definitions in the lata file
//  merge_virtual_elements: flag, if a "VIRTUAL_ELEMENTS" array is present in the lata file,
//   merges these elements to the requested block.
void DomainUnstructured::fill_domain_from_lataDB(const LataDB& lataDB,
                                                 const Domain_Id& id,
                                                 bool load_faces,
                                                 bool merge_virtual_elements)
{
  operator=(DomainUnstructured()); // Reset all data.

  id_ = id;
  const LataDBGeometry& geom = lataDB.get_geometry(id.timestep_, id.name_);

  // ********************************
  // 1) Look for the sub-block items to read (parallel computation)
  trustIdType decal_nodes = 0;
  trustIdType decal_elements = 0;
  trustIdType decal_faces = 0;
  trustIdType nb_sommets = -1;
  trustIdType nbelements = -1;
  trustIdType nbfaces = -1;

  bool domain_has_faces = load_faces && lataDB.field_exists(id.timestep_, id.name_, "FACES");

  // Tableau de 3 joints (SOM, ELEM et FACES)
  LataVector<BigTIDTab> joints;
  int nproc = 1;
  for (int i_item = 0; i_item < 3; i_item++)
    {
      //    LataField_base::Elem_som loc = LataField_base::SOM;
      Nom nom("JOINTS_SOMMETS");
      Nom nom2("SOMMETS");
      if (i_item == 1)
        {
          //loc = LataField_base::ELEM;
          nom = "JOINTS_ELEMENTS";
          nom2 = "ELEMENTS";
        }
      else if (i_item == 2)
        {
          // loc = LataField_base::FACES;
          nom = "JOINTS_FACES";
          nom2 = "FACES";
        }

      BigTIDTab& joint = joints.add();
      if (lataDB.field_exists(id.timestep_, id.name_, nom))
        {
          trustIdType nbitems = lataDB.get_field(id.timestep_, id.name_, nom2, "*").size_;
          BigTIDTab tmp;
          lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, nom, "*"), tmp);
          nproc = (int)tmp.dimension(0);
          // Recalcule la deuxieme colonne en fonction de la premiere
          joint.resize(nproc, 2);
          for (int i = 0; i < nproc; i++)
            {
              joint(i, 0) = tmp(i, 0);
              if (i < nproc-1)
                joint(i, 1) = tmp(i+1, 0) - tmp(i, 0);
              else
                joint(i, 1) = nbitems - tmp(i, 0);
            }
        }
    }

  if (id_.block_ < 0 || nproc == 1)
    {
      // read all blocks at once default values are ok
      set_joints(LataField_base::SOM) = joints[0];
      set_joints(LataField_base::ELEM) = joints[1];
      set_joints(LataField_base::FACES) = joints[2];
    }
  else
    {
      if (id_.block_ >= nproc)
        {
          Journal() << "LataFilter::get_geometry : request non existant block " << id.block_
                    << " in geometry " << id.name_ << endl;
          throw;
        }
      const int n = id_.block_;
      decal_nodes = joints[0](n, 0);
      nb_sommets = joints[0](n, 1);
      decal_elements = joints[1](n, 0);
      nbelements = joints[1](n, 1);
      if (domain_has_faces)
        {
          decal_faces = joints[2](n, 0);
          nbfaces = joints[2](n, 1);
        }
    }

  // ******************************
  // 2) Read nodes, elements and faces data
  elt_type_ = Domain::element_type_from_string(geom.elem_type_);

  if (geom.elem_type_.debute_par("RECTANGLE")) is_rectangle_ = true;

  lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "SOMMETS", "*"), nodes_, decal_nodes, nb_sommets);
  lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "ELEMENTS", "*"), elements_, decal_elements, nbelements);
  set_lata_block_offset(LataField_base::SOM, decal_nodes);
  set_lata_block_offset(LataField_base::ELEM, decal_elements);
  if (decal_nodes > 0)
    {
      // Nodes are stored with global numbering in the lata file: transform to sub_block numbering :
      // elements_ -= decal_nodes; // Do not use TRUSTArray generic (Kokkos needed)
      for (int i=0; i<elements_.size();i++)
          elements_(i) -= decal_nodes;
    }
  if (domain_has_faces)
    {
      //cerr << "Domain has faces..." << endl;
      set_lata_block_offset(LataField_base::FACES, decal_faces);
      lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "FACES", "*"), faces_, decal_faces, nbfaces);
      if (decal_nodes > 0) {
          //faces_ -= decal_nodes; // Do not use TRUSTArray generic (Kokkos needed)
          for (int i=0; i<faces_.size();i++)
              faces_(i) -= decal_nodes;
      }
      lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "ELEM_FACES", "*"), elem_faces_, decal_elements, nbelements);
      if (decal_faces > 0) {
          //elem_faces_ -= decal_faces; // Do not use TRUSTArray generic (Kokkos needed)
          for (int i=0; i<elem_faces_.size();i++)
              elem_faces_(i) -= decal_faces;
      }
    }

  // *************************
  // 3) Merge virtual elements if requested
  if (merge_virtual_elements && lataDB.field_exists(id.timestep_, id.name_, "VIRTUAL_ELEMENTS") && id.block_ >= 0)
    {
      Journal(info_level) << " Merging virtual elements" << endl;
      // joints_virt_elems(sub_block, 0) = index of first virtual element in the VIRTUAL_ELEMENTS array
      BigTIDTab joints_virt_elems;
      // Load the virtual elements (nodes are in global numbering)
      //  First: find the index and number of virtual elements for block number id.block_:
      lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "JOINTS_VIRTUAL_ELEMENTS", "*"), joints_virt_elems);
      trustIdType nb_virt_elems;
      if (id.block_ < nproc-1)
        nb_virt_elems = joints_virt_elems(id.block_+1, 0) - joints_virt_elems(id.block_, 0);
      else
        nb_virt_elems = (int)lataDB.get_field(id.timestep_, id.name_, "VIRTUAL_ELEMENTS", "*").size_ - joints_virt_elems(id.block_, 0);
      Journal(info_level+1) << " Number of virtual elements for block " << id.block_ << "=" << nb_virt_elems << endl;
      //  Second: load the indexes of the virtual elements to load:
      BigTIDTab virt_elems;
      lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "VIRTUAL_ELEMENTS", "*"), virt_elems, joints_virt_elems(id.block_,0), nb_virt_elems);
      set_virt_items(LataField_base::ELEM, virt_elems);

      {
        //  Third: load the virtual elements (virt_elems contains the global indexes of the elements to
        //  load and virt_elem_som will contain global nodes indexes of the virtual elements)
        BigTIDTab virt_elem_som;
        lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "ELEMENTS", "*"), virt_elem_som, virt_elems);
        // Find which virtual nodes are required and load them: virtual nodes to load are
        // all nodes of the virtual elements (they have duplicates).
        BigArrOfTID index;
        BigTIDTab& virt_elem_som_array = virt_elem_som;  // Array seen as monodimensionnal
        array_sort_indirect(virt_elem_som_array, index);
        // Global nodes indexes of needed virtual nodes
        BigArrOfTID nodes_to_read;
        {
          const trustIdType n = index.size_array();
          // Global index of the last loaded node:
          trustIdType last_node = -1;
          // Local index of the new loaded node:
          trustIdType new_node_index = nodes_.dimension(0)-1;
          for (trustIdType i = 0; i < n; i++)
            {
              // Take nodes to load in ascending order of their global numbers:
              const trustIdType idx = index[i];
              const trustIdType node = virt_elem_som_array[idx];
              if (node != last_node)
                {
                  // Node not yet encountered
                  nodes_to_read.append_array(node);
                  new_node_index++;
                  last_node = node;
                }
              virt_elem_som_array[idx] = new_node_index;
            }
        }
        set_virt_items(LataField_base::SOM, nodes_to_read);
        // Copy virtual elements to elements_
        trustIdType debut = elements_.size_array();
        elements_.resize(elements_.dimension(0) + virt_elem_som.dimension(0),
                         (int)elements_.dimension(1));
        elements_.inject_array(virt_elem_som, virt_elem_som.size_array(), debut);
        // Load virtual nodes
        BigFloatTab tmp_nodes;
        lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "SOMMETS", "*"), tmp_nodes, nodes_to_read);
        // Copy to nodes_
        debut = nodes_.size_array();
        nodes_.resize(nodes_.dimension(0) + tmp_nodes.dimension(0),
                      (int)nodes_.dimension(1));
        nodes_.inject_array(tmp_nodes, tmp_nodes.size_array(), debut);
      }

      if (domain_has_faces)
        {
          // Find which virtual faces are required and load them
          // For each virtual element, index of its faces (like virt_elem_som)
          BigTIDTab virt_elem_faces;
          lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "ELEM_FACES", "*"), virt_elem_faces, virt_elems);
          // Build the list of missing faces:
          BigArrOfTID index;
          BigArrOfTID& virt_elem_faces_array = virt_elem_faces;  // Array seen as monodimensionnal
          array_sort_indirect(virt_elem_faces_array, index);
          BigArrOfTID faces_to_read;
          {
            const trustIdType n = index.size_array();
            // Global index of the last loaded face:
            trustIdType last_face = -1;
            // Local index of the new loaded node:
            trustIdType new_face_index = faces_.dimension(0)-1;
            for (trustIdType i = 0; i < n; i++)
              {
                // Take nodes to load in ascending order of their global numbers:
                const trustIdType idx = index[i];
                const trustIdType face = virt_elem_faces_array[idx];
                if (face != last_face)
                  {
                    // Node not yet encountered
                    faces_to_read.append_array(face);
                    new_face_index++;
                    last_face = face;
                  }
                virt_elem_faces_array[idx] = new_face_index;
              }
          }
          set_virt_items(LataField_base::FACES, faces_to_read);
          // Copy virtual elem_faces to elem_faces
          trustIdType debut = elem_faces_.size_array();
          elem_faces_.resize(elem_faces_.dimension(0) + virt_elem_faces.dimension(0),
                             (int)elem_faces_.dimension(1));
          elem_faces_.inject_array(virt_elem_faces, virt_elem_faces.size_array(), debut);

          // Load virtual faces
          BigTIDTab tmp_faces_nodes;
          lataDB.read_data(lataDB.get_field(id.timestep_, id.name_, "FACES", "*"), tmp_faces_nodes, faces_to_read);
          // Convert global nodes indexes to local loaded nodes indexes in tmp_faces_nodes
          {
            // sort tmp_faces in ascending order so that the search requires linear time
            BigArrOfTID& array_tmp_faces_nodes = tmp_faces_nodes;
            index.reset();
            array_sort_indirect(array_tmp_faces_nodes, index);
            const trustIdType n = array_tmp_faces_nodes.size_array();
            // Take nodes in tmp_faces_nodes in ascending order and find the corresponding node in nodes_to_read
            // (which is also in sorted)
            trustIdType i1; // index in array_tmp_faces_nodes (the current node to convert)
            trustIdType i2 = 0; // index in nodes_to_read
            const trustIdType index_of_first_virtual_node = nodes_.dimension(0) - nb_virt_items(LataField_base::SOM);
            const BigArrOfTID& nodes_to_read = get_virt_items(LataField_base::SOM);
            const trustIdType max_i2 = nodes_to_read.size_array();
            for (i1 = 0; i1 < n; i1++)
              {
                const trustIdType j = index[i1];
                const trustIdType global_node_index_to_find = array_tmp_faces_nodes[j];
                // find the matching node in nodes_to_read (nodes_to_read is in ascending order)
                while (nodes_to_read[i2] != global_node_index_to_find)
                  {
                    i2++;
                    if (i2 >= max_i2)
                      {
                        cerr << "Internal error in DomainUnstructured::fill_domain_from_lataDB:\n"
                             << " node " << global_node_index_to_find << " of a virtual face does not belong to a virtual element" << endl;
                        throw;
                      }
                  }
                array_tmp_faces_nodes[j] = index_of_first_virtual_node + i2; // index of this node in the local nodes_ array
              }
          }
          // Copy to faces_ array
          debut = faces_.size_array();
          faces_.resize(faces_.dimension(0) + tmp_faces_nodes.dimension(0),
                        (int)faces_.dimension(1));
          faces_.inject_array(tmp_faces_nodes, tmp_faces_nodes.size_array(), debut);
        }
    }
}

void Domain::fill_field_from_lataDB(const LataDB& lataDB,
                                    const Field_Id& id,
                                    LataDeriv<LataField_base>& field) const
{
  Journal() << "Error : fill_field_from_lataDB not coded for this domain type" << endl;
  throw;
}

// Reads the requested field to "field" structure.
// id.block_ is not used, the data block read is the same as the domain.
void DomainUnstructured::fill_field_from_lataDB(const LataDB& lataDB,
                                                const Field_Id& id,
                                                LataDeriv<LataField_base>& field) const
{
  const LataDBField& lata_field = lataDB.get_field(id.timestep_, id.uname_);
  LataField_base::Elem_som loc = LataField_base::localisation_from_string(lata_field.localisation_);
  const trustIdType decal = lata_block_offset(loc);

  const BigArrOfTID& virt_items = get_virt_items(loc);
  const trustIdType virt_size = virt_items.size_array();
  const trustIdType size = nb_items(loc) - virt_size;

  const LataDBDataType& type = lata_field.datatype_;
  switch(type.type_)
    {
    case LataDBDataType::REAL32:
      {
        BigFloatTab& data = field.instancie(Field<BigFloatTab> ).data_;
        lataDB.read_data(lata_field, data, decal, size);
        if (virt_size > 0)
          {
            BigFloatTab tmp;
            lataDB.read_data(lata_field, tmp, virt_items);
            const trustIdType debut = data.size_array();
            data.resize(data.dimension(0)+virt_size, (int)data.dimension(1));
            data.inject_array(tmp, virt_size, debut);
          }
        break;
      }
    case LataDBDataType::REAL64:
      {
        BigDoubleTab& data = field.instancie(Field<BigDoubleTab> ).data_;
        lataDB.read_data(lata_field, data, decal, size);
        if (virt_size > 0)
          {
            BigDoubleTab tmp;
            lataDB.read_data(lata_field, tmp, virt_items);
            const trustIdType debut = data.size_array();
            data.resize(data.dimension(0)+virt_size, (int)data.dimension(1));
            data.inject_array(tmp, virt_size, debut);
          }
        break;
      }
    case LataDBDataType::INT32:
    case LataDBDataType::INT64:
      {
        BigTIDTab& data = field.instancie(Field<BigTIDTab> ).data_;
        lataDB.read_data(lata_field, data, decal, size);
        if (virt_size > 0)
          {
            BigTIDTab tmp;
            lataDB.read_data(lata_field, tmp, virt_items);
            const trustIdType debut = data.size_array();
            data.resize(data.dimension(0)+virt_size, (int)data.dimension(1));
            data.inject_array(tmp, virt_size, debut);
          }
        break;
      }
    default:
      Journal() << "LataFilter::get_field_from_lataDB " << id.uname_ << ": data type not implemented" << endl;
      throw;
    }
  field.valeur().id_ = id;
  field.valeur().component_names_ = lata_field.component_names_;
  field.valeur().localisation_ = loc;
  field.valeur().nature_ = lata_field.nature_;
}

DomainIJK::DomainIJK()
{
  part_begin_ = 0;
  part_end_ = 0;
  virtual_layer_begin_ = 0;
  virtual_layer_end_ = 0;
}

void DomainIJK::fill_domain_from_lataDB(const LataDB& lataDB,
                                        const Domain_Id& id,
                                        int split_in_N_parts,
                                        const int virt_layer_size)
{
  if (virt_layer_size < 0)
    {
      Journal() << "Error in DomainIJK::fill_domain_from_lataDB: virt_layer_size < 0" << endl;
      throw;
    }
  id_ = id;

  Journal(info_level) << "Filling ijk domain " << id.name_ << " tstep " << id.timestep_ << " block " << id.block_ << endl;
  coord_.reset();
  int dim3 = lataDB.field_exists(id.timestep_, id.name_, "SOMMETS_IJK_K", LataDB::FIRST_AND_CURRENT /* timestep */);
  {
    const LataDBField& coord = lataDB.get_field(id.timestep_,
                                                Field_UName(id.name_, "SOMMETS_IJK_I", ""),
                                                LataDB::FIRST_AND_CURRENT /* timestep */);
    BigFloatTab tmp;
    lataDB.read_data(coord, tmp);
    coord_.add();
    FloatTab tmp2;
    tmp.ref_as_small(tmp2);
    coord_[0] = tmp2;
  }
  {
    const LataDBField& coord = lataDB.get_field(id.timestep_,
                                                Field_UName(id.name_, "SOMMETS_IJK_J", ""),
                                                LataDB::FIRST_AND_CURRENT /* timestep */);
    BigFloatTab tmp;
    lataDB.read_data(coord, tmp);
    coord_.add();
    FloatTab tmp2;
    tmp.ref_as_small(tmp2);
    coord_[1] = tmp2;
  }
  if (dim3)
    {
      const LataDBField& coord = lataDB.get_field(id.timestep_,
                                                  Field_UName(id.name_, "SOMMETS_IJK_K", ""),
                                                  LataDB::FIRST_AND_CURRENT /* timestep */);
      BigFloatTab tmp;
      lataDB.read_data(coord, tmp);
      coord_.add();
      FloatTab tmp2;
      tmp.ref_as_small(tmp2);
      coord_[2] = tmp2;
    }

  elt_type_ = dim3 ? hexa : quadri;

  int block = (id.block_) < 0 ? 0 : id.block_;

  if (id.block_ >= split_in_N_parts)
    {
      Journal() << "Error in DomainIJK::fill_domain_from_lataDB: invalid block " << id.block_ << endl;
      throw;
    }

  // Load the N-th part
  //  The ijk domain is virtually split in the Z direction (or Y en 2D)
  int maxdim = coord_.size() - 1;
  // Number of elements in the Z direction:
  const int nelem = coord_[maxdim].size_array() - 1;
  int part_size = nelem / split_in_N_parts;
  if (part_size * split_in_N_parts < nelem)
    part_size++;

  // Begin and end of the requested part:
  part_begin_ = part_size * block - virt_layer_size;
  if (part_begin_ < 0)
    part_begin_ = 0;
  part_end_ = part_size * block + part_size + virt_layer_size;
  if (part_end_ > nelem)
    part_end_ = nelem;
  if (part_begin_ > part_end_)
    // empty block
    part_begin_ = part_end_ = 0;

  if (block > 0 && part_end_ > part_begin_)
    // There is a virtual layer at the begin
    virtual_layer_begin_ = virt_layer_size;
  if (block < split_in_N_parts-1 && part_end_ > part_begin_)
    virtual_layer_end_ = virt_layer_size;

  // Extract coordinates:
  ArrOfFloat tmp(coord_[maxdim]);
  const int n = part_end_ - part_begin_ + 1;
  coord_[maxdim].resize_array(n);
  for (int i = 0; i < n; i++)
    coord_[maxdim][i] = tmp[i + part_begin_];

  Journal(info_level) << "Domain " << id.name_ << " has number of nodes: [ ";
  for (int dim = 0; dim < coord_.size(); dim++)
    Journal(info_level) << coord_[dim].size_array() << " ";
  Journal(info_level) << "]" << endl;

  if (part_end_ > part_begin_ // part might be empty if too many processors
      && lataDB.field_exists(id.timestep_, id.name_, "INVALID_CONNECTIONS", LataDB::FIRST_AND_CURRENT /* timestep */))
    {
      Journal(info_level) << " loading invalid_connections" << endl;
      BigIntTab Itmp;
      trustIdType ij = 0, offset = 0, sz = 0;
      // Product of number of elements in directions I and J
      ij = coord_[0].size_array() - 1;
      if (coord_.size() > 2)
        ij *= coord_[1].size_array() - 1;
      // Select a range of elements in direction K
      offset = ij * part_begin_;
      sz = nb_elements();
      const LataDBField& lata_field = lataDB.get_field(id.timestep_,  id.name_, "INVALID_CONNECTIONS", "ELEM",
                                                       LataDB::FIRST_AND_CURRENT);

      lataDB.read_data(lata_field, Itmp, offset, sz);

      invalid_connections_.resize_array(nb_elements());
      invalid_connections_ = 0; // everything valid by default

      for (int i = 0; i < sz; i++)
        {
          if (Itmp(i, 0) != 0)
            invalid_connections_.setbit(i);
        }
    }
}

void DomainIJK::fill_field_from_lataDB(const LataDB& lataDB,
                                       const Field_Id& id,
                                       LataDeriv<LataField_base>& field) const
{
  const LataDBField& lata_field = lataDB.get_field(id.timestep_, id.uname_);
  LataField_base::Elem_som loc = LataField_base::localisation_from_string(lata_field.localisation_);

  int ij = 0, offset = 0, sz = 0;
  switch(loc)
    {
    case LataField_base::ELEM:
      // Product of number of elements in directions I and J
      ij = coord_[0].size_array() - 1;
      if (coord_.size() > 2)
        ij *= coord_[1].size_array() - 1;
      // Select a range of elements in direction K
      offset = ij * part_begin_;
      sz = ij * (part_end_ - part_begin_);
      break;
    case LataField_base::SOM:
    case LataField_base::FACES:
      // Product of number of nodes in directions I and J
      ij = coord_[0].size_array();
      if (coord_.size() > 2)
        ij *= coord_[1].size_array();
      offset = ij * part_begin_;
      sz = ij * (part_end_ + 1 - part_begin_);
      break;
    default:
      Journal() << "Error in DomainIJK::fill_field_from_lataDB: unknown localisation" << endl;
      throw;
    }

  const LataDBDataType& type = lata_field.datatype_;
  switch(type.type_)
    {
    case LataDBDataType::REAL32:
      {
        BigFloatTab& data = field.instancie(Field<BigFloatTab> ).data_;
        lataDB.read_data(lata_field, data, offset, sz);
        break;
      }
    case LataDBDataType::REAL64:
      {
        BigDoubleTab& data = field.instancie(Field<BigDoubleTab> ).data_;
        lataDB.read_data(lata_field, data, offset, sz);
        break;
      }
    case LataDBDataType::INT32:
    case LataDBDataType::INT64:
      {
        BigTIDTab& data = field.instancie(Field<BigTIDTab> ).data_;
        lataDB.read_data(lata_field, data, offset, sz);
        break;
      }
    default:
      Journal() << "LataFilter::get_field_from_lataDB " << id.uname_ << ": data type not implemented" << endl;
      throw;
    }
  field.valeur().id_ = id;
  field.valeur().component_names_ = lata_field.component_names_;
  field.valeur().localisation_ = loc;
  field.valeur().nature_ = lata_field.nature_;
}


Domain::DomainType Domain::get_domain_type() const
{
  const DomainUnstructured* geom_ptr = dynamic_cast<const DomainUnstructured*>(this);
  if (geom_ptr!=0)
    return UNSTRUCTURED;
  const DomainIJK* ijk_ptr = dynamic_cast<const DomainIJK*>(this);
  if (ijk_ptr!=0)
    return IJK;
  throw ("Not implemeneted");
}

const DomainUnstructured& Domain::cast_DomainUnstructured() const
{
  return dynamic_cast<const DomainUnstructured&>(*this);
}

const DomainIJK&   Domain::cast_DomainIJK() const
{
  return dynamic_cast<const DomainIJK&>(*this);
}
