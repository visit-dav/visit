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

void build_geometry_(OperatorFacesMesh& op, const DomainUnstructured& src, LataDeriv<Domain>& dest_domain)
{
  Journal(verb_level) << "OperatorFacesMesh geometry(unstructured) " << src.id_.name_ << endl;
  if (!src.faces_ok())
    {
      Journal() << "Error in OperatorFacesMesh::build_geometry: source domain has no faces data" << endl;
      throw;
    }
  // const int max_nb_som_face = 3; // for tetrahedra
  if (src.elt_type_ != Domain::triangle && src.elt_type_ != Domain::quadri && src.elt_type_ != Domain::polygone && src.elt_type_ != Domain::tetra && src.elt_type_ != Domain::polyedre)
    {
      Journal() << "Error in OperatorFacesMesh::build_geometry: cannot operate on unstructured mesh with this element type" << endl;
      throw;
    }
  //  const int nb_som = src.nodes_.dimension(0);
  // const int nb_elem = src.elem_faces_.dimension(0); // Not elements_, in case elem_faces_ has no virtual data.
  //const int dim = src.dimension();

  DomainUnstructured& dest = dest_domain.instancie(DomainUnstructured);
  dest.id_ = src.id_;
  dest.id_.name_ += "_centerfaces";
  if (src.elt_type_ == Domain::triangle || src.elt_type_ == Domain::quadri || src.elt_type_ == Domain::polygone)
    dest.elt_type_ = Domain::line;
  else if (src.elt_type_ == Domain::tetra)
    dest.elt_type_ = Domain::triangle;
  else if (src.elt_type_ == Domain::polyedre)
    dest.elt_type_ = Domain::polygone;

  dest.nodes_ = src.nodes_;
  dest.elements_ = src.faces_;

  const trustIdType nb_elem_virt = src.nb_virt_items(LataField_base::FACES);
  dest.set_nb_virt_items(LataField_base::ELEM, nb_elem_virt);
}

// Builds a field on the dual domain from the field on the source domain.
// Source field must be located at faces.
// (destination field is located at the elements. the value for an element
//  is the value associated to the adjacent face of the source domain).
template<class TabType>
void build_field_(OperatorFacesMesh& op, const DomainUnstructured& src_domain, const DomainUnstructured& dest_domain, const Field<TabType>& src, Field<TabType>& dest)
{
  Journal(verb_level) << "OperatorFacesMesh field(unstructured) " << src.id_.uname_ << endl;
  dest.component_names_ = src.component_names_;
  dest.localisation_ = LataField_base::ELEM;
  dest.nature_ = src.nature_;

  dest.data_ = src.data_;

}

void build_geometry_(OperatorFacesMesh& op, const DomainIJK& src, LataDeriv<Domain>& dest_domain)
{
  Journal(verb_level) << "OperatorFacesMesh geometry(ijk) " << src.id_.name_ << endl;
  Journal() << "Error in OperatorFacesMesh::build_geometry: cannot operate on domainIJK" << endl;
  throw;

}
template<class TabType>
void build_field_(OperatorFacesMesh& op, const DomainIJK& src_domain, const DomainIJK& dest_domain, const Field<TabType>& src, Field<TabType>& dest)
{
  Journal(verb_level) << "OperatorFacesMesh field(ijk) " << src.id_.uname_ << endl;
  Journal() << "Error in OperatorFacesMesh::build_geometry: cannot operate on domainIJK" << endl;
  throw;
}

void OperatorFacesMesh::build_geometry(const Domain& src_domain, LataDeriv<Domain>& dest)
{
  apply_geometry(*this, src_domain, dest);
}

void OperatorFacesMesh::build_field(const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest)
{
  if (src_field.localisation_ != LataField_base::FACES)
    {
      Journal() << "Error in OperatorFacesMesh::build_field: source field is not located at faces" << endl;
      throw;
    }
  apply_field(*this, src_domain, src_field, dest_domain, dest);
}
#undef level
