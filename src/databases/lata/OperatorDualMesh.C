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

#include <LataFilter.h>
#include <Operator.h>

// Journal level
#define verb_level 4

void build_geometry_(OperatorDualMesh& op, const DomainUnstructured& src, LataDeriv<Domain>& dest_domain)
{
  Journal(verb_level) << "OperatorDualMesh geometry(unstructured) " << src.id_.name_ << endl;
  if (!src.faces_ok())
    {
      Journal() << "Error in OperatorDualMesh::build_geometry: source domain has no faces data" << endl;
      throw;
    }
  const int max_nb_som_face = 3; // for tetrahedra
  if (src.elt_type_ != Domain::triangle && src.elt_type_ != Domain::tetra)
    {
      Journal() << "Error in OperatorDualMesh::build_geometry: cannot operate on unstructured mesh with this element type" << endl;
      throw;
    }
  const trustIdType nb_som = src.nodes_.dimension(0);
  const trustIdType nb_elem = src.elem_faces_.dimension(0); // Not elements_, in case elem_faces_ has no virtual data.
  const int dim = src.dimension();

  DomainUnstructured& dest = dest_domain.instancie(DomainUnstructured);
  dest.id_ = src.id_;
  dest.id_.name_ += "_dual";
  dest.elt_type_ = src.elt_type_;

  dest.nodes_ = src.nodes_;
  dest.nodes_.resize(nb_som + nb_elem, dim);
  src.compute_cell_center_coordinates(dest.nodes_, nb_som);

  const int nb_faces_elem = (int)src.elem_faces_.dimension(1);
  const int nb_som_face = (int)src.faces_.dimension(1);
  const int nb_som_elem = (int)src.elements_.dimension(1);
  dest.elements_.resize(nb_elem * nb_faces_elem, nb_som_elem);
  trustIdType index = 0;
  for (trustIdType i = 0; i < nb_elem; i++)
    {
      const trustIdType central_node = nb_som + i;
      for (int j = 0; j < nb_faces_elem; j++)
        {
          const trustIdType face = src.elem_faces_(i, j);
          dest.elements_(index, 0) = central_node;
          for (int k = 0; k < loop_max(nb_som_face, max_nb_som_face); k++)
            {
              dest.elements_(index, k + 1) = src.faces_(face, k);
              break_loop(k, nb_som_face);
            }
          index++;
        }
    }
  const trustIdType nb_elem_virt = src.nb_virt_items(LataField_base::ELEM);
  dest.set_nb_virt_items(LataField_base::ELEM, nb_elem_virt * nb_faces_elem);
}

// Builds a field on the dual domain from the field on the source domain.
// Source field must be located at faces.
// (destination field is located at the elements. the value for an element
//  is the value associated to the adjacent face of the source domain).
template<class TabType>
void build_field_(OperatorDualMesh& op, const DomainUnstructured& src_domain, const DomainUnstructured& dest_domain, const Field<TabType>& src, Field<TabType>& dest)
{
  Journal(verb_level) << "OperatorDualMesh field(unstructured) " << src.id_.uname_ << endl;
  dest.component_names_ = src.component_names_;
  dest.localisation_ = LataField_base::ELEM;
  dest.nature_ = src.nature_;
  const trustIdType nb_elem = src_domain.elements_.dimension(0);
  const int nb_face_elem = (int)src_domain.elem_faces_.dimension(1);
  const int nb_comp = (int)src.data_.dimension(1);
  dest.data_.resize(nb_elem * nb_face_elem, nb_comp);
  trustIdType index = 0;
  for (trustIdType i = 0; i < nb_elem; i++)
    {
      for (int j = 0; j < nb_face_elem; j++)
        {
          const trustIdType face = src_domain.elem_faces_(i, j);
          for (int k = 0; k < nb_comp; k++)
            dest.data_(index, k) = src.data_(face, k);
          index++;
        }
    }
}

void build_geometry_(OperatorDualMesh& op, const DomainIJK& src, LataDeriv<Domain>& dest_domain)
{
  Journal(verb_level) << "OperatorDualMesh geometry(ijk) " << src.id_.name_ << endl;
  if (src.elt_type_ != Domain::quadri && src.elt_type_ != Domain::hexa)
    {
      Journal() << "Error in OperatorDualMesh::build_geometry: cannot operate on unstructured mesh with this element type" << endl;
      throw;
    }

  DomainIJK& dest = dest_domain.instancie(DomainIJK);
  dest.elt_type_ = src.elt_type_;
  const int dim = src.dimension();
  for (int i_dim = 0; i_dim < dim; i_dim++)
    {
      const ArrOfFloat& c1 = src.coord_[i_dim];
      ArrOfFloat& c2 = dest.coord_.add(ArrOfFloat());
      const int n = c1.size_array() - 1;
      c2.resize_array(n * 2 + 1);
      for (int i = 0; i < n; i++)
        {
          c2[i * 2] = c1[i];
          c2[i * 2 + 1] = (c1[i] + c1[i + 1]) * 0.5f;
        }
      c2[n * 2] = c1[n];
    }

  if (src.invalid_connections_.size_array() > 0)
    {
      dest.invalid_connections_.resize_array(dest.nb_elements());
      dest.invalid_connections_ = 0;
      int index = 0;

      const int ni = dest.coord_[0].size_array() - 1;
      const int nj = dest.coord_[1].size_array() - 1;
      const int nk = (dim == 3) ? (dest.coord_[2].size_array() - 1) : 1;
      const int ni_src = src.coord_[0].size_array() - 1;
      const int nj_src = src.coord_[1].size_array() - 1;
      for (int k = 0; k < nk; k++)
        {
          const int k_src = k / 2;
          for (int j = 0; j < nj; j++)
            {
              const int j_src = j / 2;
              const trustIdType idx_source = ((trustIdType)k_src * (trustIdType)nj_src + (trustIdType)j_src) * (trustIdType)ni_src;
              for (int i = 0; i < ni; i++)
                {
                  const trustIdType idx = idx_source + i / 2;
                  if (src.invalid_connections_[idx])
                    dest.invalid_connections_.setbit(index);
                  index++;
                }
            }
        }
    }
  dest.virtual_layer_begin_ = 2 * src.virtual_layer_begin_;
  dest.virtual_layer_end_ = 2 * src.virtual_layer_end_;
}

#define IJK(i,j,k) ((trustIdType)k*nj_ni_src + (trustIdType)j*(trustIdType)ni_src + (trustIdType)i)

template<class TabType>
void build_field_(OperatorDualMesh& op, const DomainIJK& src_domain, const DomainIJK& dest_domain, const Field<TabType>& src, Field<TabType>& dest)
{
  Journal(verb_level) << "OperatorDualMesh field(ijk) " << src.id_.uname_ << endl;
  dest.component_names_ = src.component_names_;
  dest.localisation_ = LataField_base::ELEM;
  dest.nature_ = LataDBField::VECTOR;
  const int dim = src_domain.dimension();
  trustIdType index = 0;

  // Loop on destination elements
  const int ni = dest_domain.coord_[0].size_array() - 1;
  const int nj = dest_domain.coord_[1].size_array() - 1;
  const int nk = (dim == 3) ? (dest_domain.coord_[2].size_array() - 1) : 1;
  dest.data_.resize(ni * nj * nk, dim);
  const int ni_src = src_domain.coord_[0].size_array();
  const trustIdType nj_ni_src = (trustIdType)src_domain.coord_[1].size_array() * (trustIdType)ni_src;
  for (int k = 0; k < nk; k++)
    {
      const int k2 = k / 2;
      const int k3 = (k + 1) / 2;
      for (int j = 0; j < nj; j++)
        {
          const int j2 = j / 2;
          const int j3 = (j + 1) / 2;
          for (int i = 0; i < ni; i++)
            {
              const int i2 = i / 2;
              const int i3 = (i + 1) / 2;
              dest.data_(index, 0) = src.data_(IJK(i3, j2, k2), 0);
              dest.data_(index, 1) = src.data_(IJK(i2, j3, k2), 1);
              if (dim == 3)
                dest.data_(index, 2) = src.data_(IJK(i2, j2, k3), 2);
              index++;
            }
        }
    }
}

#undef IJK

void OperatorDualMesh::build_geometry(const Domain& src_domain, LataDeriv<Domain>& dest)
{
  apply_geometry(*this, src_domain, dest);
}

void OperatorDualMesh::build_field(const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest)
{
  if (src_field.localisation_ != LataField_base::FACES)
    {
      Journal() << "Error in OperatorDualMesh::build_field: source field is not located at faces" << endl;
      throw;
    }
  apply_field(*this, src_domain, src_field, dest_domain, dest);
}
#undef level
