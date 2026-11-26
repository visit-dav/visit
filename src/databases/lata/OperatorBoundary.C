/* Copyright (c) 2022, CEA
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

#include <LataFilter.h>
#include <Operator.h>
#include <Static_Int_Lists.h>
#include <Connectivite_som_elem.h>

// Journal level for messages
#define verb_level 4

void build_ref_elem_face(const Domain::Element elt_type, IntTab& ref_elem_face)
{
  static int faces_sommets_tetra[4][3] = { { 1, 2, 3 }, { 0, 3, 2 }, { 3, 0, 1 }, { 0, 2, 1 } };
  static int faces_sommets_hexa[6][4] = { { 0, 2, 4, 6 }, { 0, 1, 4, 5 }, { 0, 1, 2, 3 }, { 1, 3, 5, 7 }, { 2, 3, 6, 7 }, { 4, 5, 6, 7 } };

  int i, j;
  switch(elt_type)
    {
    case Domain::tetra:
      ref_elem_face.resize(4, 3);
      for (i = 0; i < 4; i++)
        for (j = 0; j < 3; j++)
          ref_elem_face(i, j) = faces_sommets_tetra[i][j];
      break;
    case Domain::hexa:
      ref_elem_face.resize(6, 4);
      for (i = 0; i < 6; i++)
        for (j = 0; j < 4; j++)
          ref_elem_face(i, j) = faces_sommets_hexa[i][j];
      break;
    default:
      Journal() << "build_ref_elem_face : non code pour element " << endl;
    }
}

void build_geometry_(OperatorBoundary& op, const DomainUnstructured& src, LataDeriv<Domain>& dest_domain)
{
  Journal(verb_level) << "OperatorBoundary domain " << src.id_.name_ << endl;
  DomainUnstructured& dest = dest_domain.instancie(DomainUnstructured);
  switch(src.elt_type_)
    {
    case Domain::tetra:
      dest.elt_type_ = Domain::triangle;
      break;
    case Domain::hexa:
      dest.elt_type_ = Domain::quadri;
      break;
    default:
      Journal() << "Error in OperatorBoundary: element type not supported" << endl;
      throw;
    }

  Static_Int_Lists_64 som_elem;
  construire_connectivite_som_elem<trustIdType>(src.nb_nodes(), src.elements_, som_elem, 0);
  // For each element:
  //  for each face of this element
  //    how many neighbouring elements ?
  //      if only one neighbour, it's a boundary face !

  IntTab element_faces;
  build_ref_elem_face(src.elt_type_, element_faces);
  const int nb_nodes_per_face = element_faces.dimension(1);
  const int nb_faces_per_element = element_faces.dimension(0);

  ArrOfTID one_face(nb_nodes_per_face);
  ArrOfTID adjacent_elements;

  // For each node in the source domain, node number on the boundary:
  BigArrOfTID nodes_renumber;
  nodes_renumber.resize_array(src.nb_nodes());
  nodes_renumber = -1;

  int element_index, local_face_index;
  // Browse only real elements (so we don't see boundaries between processors)
  const trustIdType nelem = src.nb_elements() - src.nb_virt_items(LataField_base::ELEM);
  int i;
  trustIdType count = 0;
  for (element_index = 0; element_index < nelem; element_index++)
    {
      for (local_face_index = 0; local_face_index < nb_faces_per_element; local_face_index++)
        {
          for (i = 0; i < nb_nodes_per_face; i++)
            {
              int local_node = element_faces(local_face_index, i);
              trustIdType node = src.elements_(element_index, local_node);
              one_face[i] = node;
            }
          find_adjacent_elements(som_elem, one_face, adjacent_elements);
          if (adjacent_elements.size_array() == 1)
            {
              op.src_element_.append_array(element_index);
              op.src_face_.append_array(local_face_index);
              for (i = 0; i < nb_nodes_per_face; i++)
                {
                  trustIdType node = one_face[i];
                  trustIdType dest_node = nodes_renumber[node];
                  if (dest_node < 0)
                    {
                      dest_node = count++;
                      op.src_nodes_.append_array(node);
                      nodes_renumber[node] = dest_node;
                    }
                }
            }
        }
    }

  // Build nodes
  const trustIdType nb_nodes = op.src_nodes_.size_array();
  const int dim = (int)src.nodes_.dimension(1);
  dest.nodes_.resize(nb_nodes, dim);
  for (i = 0; i < nb_nodes; i++)
    {
      const trustIdType n = op.src_nodes_[i];
      for (int j = 0; j < dim; j++)
        dest.nodes_(i, j) = src.nodes_(n, j);
    }

  // Build elements
  const trustIdType nb_elems = op.src_element_.size_array();
  dest.elements_.resize(nb_elems, nb_nodes_per_face);
  for (i = 0; i < nb_elems; i++)
    {
      const trustIdType elem = op.src_element_[i];
      const int face = op.src_face_[i];
      for (int j = 0; j < nb_nodes_per_face; j++)
        {
          const trustIdType src_node = src.elements_(elem, element_faces(face, j));
          dest.elements_(i, j) = nodes_renumber[src_node];
        }
    }
  op.geom_init_ = 1;
}

template<class TabType>
void build_field_(OperatorBoundary& op, const DomainUnstructured& src_domain, const DomainUnstructured& dest_domain, const Field<TabType>& src, Field<TabType>& dest)
{
  if (!op.geom_init_)
    {
      // Must fill the renum_.... arrays first !
      LataDeriv<Domain> destb;
      op.build_geometry(src_domain, destb);
    }
  dest.component_names_ = src.component_names_;
  dest.localisation_ = src.localisation_;
  dest.nature_ = src.nature_;
  if (dest.localisation_ == LataField_base::FACES)
    dest.localisation_ = LataField_base::ELEM;

  const int nb_compo = (int)src.data_.dimension(1);
  trustIdType i, sz = 0;
  switch(src.localisation_)
    {
    case LataField_base::ELEM:
      sz = dest_domain.nb_elements();
      dest.data_.resize(sz, nb_compo);
      for (i = 0; i < sz; i++)
        {
          const trustIdType old_i = op.src_element_[i];
          for (int j = 0; j < nb_compo; j++)
            dest.data_(i, j) = src.data_(old_i, j);
        }
      break;
    case LataField_base::SOM:
      sz = dest_domain.nb_nodes();
      dest.data_.resize(sz, nb_compo);
      for (i = 0; i < sz; i++)
        {
          const trustIdType old_i = op.src_nodes_[i];
          for (int j = 0; j < nb_compo; j++)
            dest.data_(i, j) = src.data_(old_i, j);
        }
      break;
    case LataField_base::FACES:
      sz = dest_domain.nb_elements();
      dest.data_.resize(sz, nb_compo);
      for (i = 0; i < sz; i++)
        {
          const trustIdType old_i = src_domain.elem_faces_(op.src_element_[i], op.src_face_[i]);
          for (int j = 0; j < nb_compo; j++)
            dest.data_(i, j) = src.data_(old_i, j);
        }
      break;
    default:
      Journal() << "Error in OperatorRegularize::build_field_: unknown localisation" << endl;
      throw;
    }
}

void OperatorBoundary::build_geometry(const Domain& src_domain, LataDeriv<Domain>& dest)
{
  apply_geometry(*this, src_domain, dest);
}

void OperatorBoundary::build_field(const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest)
{
  apply_field(*this, src_domain, src_field, dest_domain, dest);
}

#undef verb_level
