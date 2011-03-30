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

#include <LataFilter.h>
#include <Operator.h>

#define verb_level 4

template<class T, class Tab> int search_in_ordered_vect(T x, const Tab & v) {
  if (!v.size_array())
    return -1;
  int i1=0;
  int i;
  int i2 = (int)v.size_array()-1;
  while (i1 != i2) {
    i = (i1+i2)/2;
    if (v[i] < x)
      i1=i+1;
    else
      i2=i;
  }
  if (v[i1] == x)
    return i1;
  return -1;
}

template<class T, class Tab>
static void retirer_doublons(Tab & tab, const T epsilon)
{
  int i = 0;
  int j;
  const int n = tab.size_array();
  T last_tab_i = -1e40;
  for (j = 0; j < n; j++) {
    const T x = tab[j];
    assert(x >= last_tab_i); // Array must be sorted
    if (x - last_tab_i > epsilon) {
      tab[i] = x;
      last_tab_i = x;
      i++;
    }
  }
  tab.resize_array(i);
}

void build_geometry_(OperatorRegularize & op,
                     const DomainUnstructured & src, LataDeriv<Domain> & dest_domain)
{
  Journal(verb_level) << "OperatorRegularize domain " << src.id_.name_ << endl;
  if (src.elt_type_ != Domain::quadri && src.elt_type_ != Domain::hexa) {
    Journal() << "Error in OperatorRegularize::build_geometry: cannot operate on unstructured mesh with this element type" << endl;
    throw;
  }

  DomainIJK & dest = dest_domain.instancie(DomainIJK);
  dest.elt_type_ = src.elt_type_;
  const entier nsom = src.nodes_.dimension(0);
  const entier dim = src.nodes_.dimension(1);
  ArrOfInt nb_som_dir(dim);
  {
    double product_n = 1.;
    for (entier i_dim = 0; i_dim < dim; i_dim++) {
      ArrOfFloat & coord = dest.coord_.add(ArrOfFloat());
      coord.resize_array(nsom);
      entier i;
      for (i = 0; i < nsom; i++)
        coord[i] = src.nodes_(i, i_dim);
      coord.ordonne_array();
      retirer_doublons(coord, op.tolerance_);
      product_n *= coord.size_array();
      // Add extended domain layer:
      if (coord.size_array() > 1) {
        const entier n = coord.size_array();
        const entier l = op.extend_layer_;
        coord.resize_array(n + l * 2);
        double x0 = coord[n-1];
        double delta = coord[n-2] - x0;
        for (i = 1; i <= l; i++)
          coord[n + l + i] = x0 + delta * i;
        for (i = l-1; i >= 0; i--)
          coord[i + l] = coord[i];
        x0 = coord[l];
        delta = coord[l+1] - x0;
        for (i = 1; i <= l; i++)
          coord[l - i] = x0 - delta * i;
      }
      nb_som_dir[i_dim] = coord.size_array();
    }
    // Verifying that unique has deleted many points...
    // If well organised, nsom=nx*ny*nz
    // If chaos, nsom=(nx+ny+nz)/3
    // We want to verify that we are nearer to organisation than to chaos !
    if (product_n > (double) nsom * (double) nsom - 1.) {
      Journal() << "Positions do not seam regular !" << endl;
      throw;
    }
  }
  int i;
  op.renum_nodes_.resize_array(nsom);
  int nb_som_ijk = 1;
  for (i = 0; i < dim; i++) 
    nb_som_ijk *= nb_som_dir[i];
  IntTab ijk_indexes;
  ijk_indexes.resize(nsom, dim);
  for (i = 0; i < nsom; i++) {
    entier ijk_index = 0;
    for (int j = dim-1; j >= 0; j--) {
      const double x = src.nodes_(i,j);
      int index = search_in_ordered_vect(x, dest.coord_[j]);
      if (index < 0) {
        Journal() << "Error: coordinate (" << i << "," << j << ") = " << x << " not found in regularize" << endl
                  << "Try reducing regularize tolerance value (option regularize=epsilon)" << endl;
        throw;
      }
      ijk_indexes(i, j) = index;
      ijk_index += index;
      if (j)
        ijk_index *= nb_som_dir[j-1];
    }
    op.renum_nodes_[i] = ijk_index;
  }
  const int max_index = max_array(nb_som_dir);
  int nb_elems_ijk = 1;
  for (i = 0; i < dim; i++)
    nb_elems_ijk *= nb_som_dir[i] - 1;
  dest.invalid_connections_.resize_array(nb_elems_ijk);
  dest.invalid_connections_ = 1; // Everything invalid by default
  const int nelem = src.elements_.dimension(0);
  const int nb_som_elem = src.elements_.dimension(1);
  op.renum_elements_.resize_array(nelem);
  // Pour chaque element, indice dans le maillage ijk du plus sommet le plus proche de l'origine
  // (pour les faces...)
  ArrOfInt idx_elem_som;
  idx_elem_som.resize_array(nelem);
  int min_index[3];
  for (i = 0; i < nelem; i++) {
    min_index[0] = min_index[1] = min_index[2] = max_index;
    for (int j = 0; j < nb_som_elem; j++) {
      int node = src.elements_(i,j);
      for (int k = 0; k < loop_max(dim, 3); k++) {
        int idx = ijk_indexes(node, k);
        min_index[k] = (idx < min_index[k]) ? idx : min_index[k];
        break_loop(k,dim);
      }
    }
    entier idx = 0;
    entier idx_som = 0;
    if (dim == 1) {
      idx = min_index[0];
      idx_som = idx;
    } else if (dim == 2) {
      idx = min_index[1] * (nb_som_dir[0]-1) + min_index[0];
      idx_som = min_index[1] * nb_som_dir[0] + min_index[0];
    } else if (dim == 3) {
      idx = (min_index[2] * (nb_som_dir[1]-1) + min_index[1]) * (nb_som_dir[0]-1) + min_index[0];
      idx_som = (min_index[2] * nb_som_dir[1] + min_index[1]) * nb_som_dir[0] + min_index[0];
    } else
      throw;
    op.renum_elements_[i] = idx;
    dest.invalid_connections_.clearbit(idx);
    idx_elem_som[i] = idx_som;
  }
  
  if (src.faces_ok()) {
    const int nfaces = src.faces_.dimension(0);
    op.renum_faces_.resize_array(nfaces);
    op.renum_faces_ = -1;
    const int nb_elem_face = src.elem_faces_.dimension(1);
    ArrOfInt delta_dir(dim);
    delta_dir[0] = 1;
    for (i = 1; i < dim; i++)
      delta_dir[i] = delta_dir[i-1] * nb_som_dir[i-1];
    for (i = 0; i < nelem; i++) {
      // Les faces haut, gauche et arriere du cube a l'origine portent le numero 0
      // Voir DomaineIJK pour la convention sur la numerotation des faces
      for (entier j = 0; j < nb_elem_face; j++) {
        const entier i_face = src.elem_faces_(i, j);
        entier dir = j % dim;
        entier index = idx_elem_som[i];
        if (j>=dim) 
          index += delta_dir[dir];
        // Encodage du numero de la face et de la direction
        index = (index << 2) + dir;
        if (op.renum_faces_[i_face] < 0) {
          op.renum_faces_[i_face] = index;
        } else if (op.renum_faces_[i_face] != index) {
          Journal() << "Error in OperatorRegularize: faces renumbering failed" << endl;
          throw;
        }
      }
    }
  }
  op.geom_init_ = 1;
}

template <class TabType>
void build_field_(OperatorRegularize & op,
                  const DomainUnstructured & src_domain,
                  const DomainIJK          & dest_domain,
                  const Field<TabType> & src,
                  Field<TabType> & dest)
{
  Journal(verb_level) << "OperatorRegularize field " << src.id_.uname_ << endl;
  if (!op.geom_init_) {
    // Must fill the renum_.... arrays first !
    LataDeriv<Domain> dest;
    op.build_geometry(src_domain, dest);
  }
  dest.component_names_ = src.component_names_;
  dest.localisation_ = src.localisation_;
  dest.nature_ = src.nature_;
  const entier sz = src.data_.dimension(0);
  const entier nb_compo = src.data_.dimension(1);
  entier i;
  switch(src.localisation_) {
  case LataField_base::ELEM:
    dest.data_.resize(dest_domain.nb_elements(), nb_compo);
    for (i = 0; i < sz; i++) {
      const entier new_i = op.renum_elements_[i];
      for (entier j = 0; j < nb_compo; j++)
        dest.data_(new_i, j) = src.data_(i, j);
    }
    break;
  case LataField_base::SOM:
    dest.data_.resize(dest_domain.nb_nodes(), nb_compo);
    for (i = 0; i < sz; i++) {
      const entier new_i = op.renum_nodes_[i];
      for (entier j = 0; j < nb_compo; j++)
        dest.data_(new_i, j) = src.data_(i, j);
    }
    break;
  case LataField_base::FACES:
    {
      if (nb_compo != 1) {
        Journal() << "Error in OperatorRegularize: field at faces has nb_compo != 1" << endl;
        throw;
      }
      dest.nature_ = LataDBField::VECTOR;
      const entier nb_dim = dest_domain.dimension();
      dest.data_.resize(dest_domain.nb_faces(), nb_dim);
      // Field is interpreted as normal component to the face
      for (i = 0; i < sz; i++) {
        const entier code = op.renum_faces_[i];
        // decodage numero et direction de la face:
        const entier new_i = code >> 2;
        const entier direction = (code & 3);
        dest.data_(new_i, direction) = src.data_(i, 0);
      }
    }
    break;
  default:
    Journal() << "Error in OperatorRegularize::build_field_: unknown localisation" << endl;
    throw;
  }
}

void OperatorRegularize::build_geometry(const Domain & src_domain, LataDeriv<Domain> & dest)
{
  apply_geometry(*this, src_domain, dest);
}

void OperatorRegularize::build_field(const Domain & src_domain, const LataField_base & src_field,
                                     const Domain & dest_domain, LataDeriv<LataField_base> & dest)
{
  apply_field(*this, src_domain, src_field, dest_domain, dest);
}
#undef verb_level
