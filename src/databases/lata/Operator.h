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

#ifndef OPERATORS_H
#define OPERATORS_H
#include <LataFilter.h>

// A tool to "reconnect" several subdomains of a parallel computation:
//  reconnect_geometry() searches for duplicate node coordinates and
//  changes the elements_ and faces_ arrays to use the smallest node
//  index that has the same coordinate. Hence, we recover the connectivity
//  between blocks.
class Reconnect
{
public:
  static void reconnect_geometry(DomainUnstructured& geom, double tolerance, trustIdType nb_nodes_untouched = 0);
  static void apply_renumbering(const BigArrOfTID& nodes_renumber, BigArrOfTID& data);
  static void search_duplicate_nodes(const BigFloatTab& src_coord, BigArrOfTID& nodes_renumber, double eps, trustIdType nb_nodes_untouched = 0);
};

class OperatorClipbox: public Operator
{
public:
  void build_geometry(const Domain& src_domain, LataDeriv<Domain>& dest) override;
  void build_field(const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest) override;
  // Renumerotation des sommets, elements et faces par rapport aux donnees brutes lues
  // renum_truc_[new_index] = index in lata file;
  // La renumerotation vient de clip_box et de regularize
  BigArrOfInt renum_nodes_;
  BigArrOfInt renum_elements_;
  BigArrOfInt renum_faces_;
};

class OperatorBoundary: public Operator
{
public:
  OperatorBoundary() : geom_init_(false) { }
  void build_geometry(const Domain& src_domain, LataDeriv<Domain>& dest) override;
  void build_field(const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest) override;
  Size_t compute_memory_size() const override
  {
    return memory_size(src_nodes_) + memory_size(src_element_) + memory_size(src_face_);
  }
  // Renumerotation des sommets, elements et faces par rapport aux donnees brutes lues
  // renum_truc_[new_index] = index in lata file;
  // La renumerotation vient de clip_box et de regularize
  BigArrOfTID src_nodes_; // for each boundary node, which node is it in source domain ?
  BigArrOfTID src_element_; // same for boundary face vs source domain element
  BigArrOfInt src_face_; // local (hence 'int', not 'TID') face number on src_element_
  bool geom_init_;
};

class OperatorRegularize: public Operator
{
public:
  OperatorRegularize()
  {
    tolerance_ = -1.;
    geom_init_ = false;
    extend_layer_ = 0;
  }
  void set_tolerance(double epsilon)
  {
    tolerance_ = epsilon;
  }
  void set_extend_layer(int n)
  {
    if (n >= 0)
      extend_layer_ = n;
    else
      extend_layer_ = 0;
  }
  void build_geometry(const Domain& src_domain, LataDeriv<Domain>& dest) override;
  void build_field(const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest) override;

  Size_t compute_memory_size() const override
  {
    return memory_size(renum_nodes_) + memory_size(renum_elements_) + memory_size(renum_faces_);
  }
  // Renumerotation des sommets, elements et faces par rapport aux donnees brutes lues
  // renum_truc_[old_index] = new_index;
  BigArrOfTID renum_nodes_;
  BigArrOfTID renum_elements_;
  // Pour les faces: les faces de chaque direction du domaine ijk sont numerotees
  //  separement: faces de normales X entre 0 et N, faces de normales Y entre 0 et N, etc...
  // Le numero d'une face est egal au plus petit des numeros de ses sommets du le maillage ijk.
  // Renum faces contient le codage suivant:
  //  numero de la face = renum_faces_[i] >> 2;
  //  direction de la face  = (renum_faces_ & 3)
  BigArrOfTID renum_faces_;
  double tolerance_;
  int extend_layer_;
  bool geom_init_;
};

class OperatorDualMesh: public Operator
{
public:
  void build_geometry(const Domain& src_domain, LataDeriv<Domain>& dest) override;
  void build_field(const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest) override;
  Size_t compute_memory_size() const override
  {
    return 0;
  }
};
class OperatorFacesMesh: public Operator
{
public:
  void build_geometry(const Domain& src_domain, LataDeriv<Domain>& dest) override;
  void build_field(const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest) override;
  Size_t compute_memory_size() const override
  {
    return 0;
  }
};

class OperatorNCMesh: public Operator
{
public:
  void build_geometry(const Domain& src_domain, LataDeriv<Domain>& dest) override;
  void build_field(const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest) override;
  Size_t compute_memory_size() const override
  {
    return 0;
  }
};

// These generic methods just say that the particular function does not exist:
void build_geometry_(Operator& op, const Domain& src, LataDeriv<Domain>& dest);
void build_field_(Operator& op, const Domain& src, const Domain& dest, const LataField_base& srcf, LataField_base& destf);

template<class Op>
void apply_geometry(Op& op, const Domain& src_domain, LataDeriv<Domain>& dest)
{
  const DomainUnstructured *src1 = dynamic_cast<const DomainUnstructured*>(&src_domain);
  const DomainIJK *src2 = dynamic_cast<const DomainIJK*>(&src_domain);

  if (src1)
    build_geometry_(op, *src1, dest);
  else if (src2)
    build_geometry_(op, *src2, dest);
  else
    {
      Journal() << "Error in OperatorDualMesh::build_geometry: unsupported domain type" << endl;
      throw;
    }
}

// See apply_field
template<class Op, class DomSrc, class DomDest>
void apply_field3(Op& op, const DomSrc& src_domain, const LataField_base& src_field, const DomDest& dest_domain, LataDeriv<LataField_base>& dest)
{
  const Field<BigDoubleTab> *src1 = dynamic_cast<const Field<BigDoubleTab>*>(&src_field);
  const Field<BigFloatTab> *src2 = dynamic_cast<const Field<BigFloatTab>*>(&src_field);
  const Field<BigIntTab> *src3 = dynamic_cast<const Field<BigIntTab>*>(&src_field);
  const Field<BigTIDTab> *src4 = dynamic_cast<const Field<BigTIDTab>*>(&src_field);


  if (src1)
    build_field_(op, src_domain, dest_domain, *src1, dest.instancie(Field<BigDoubleTab> ));
  else if (src2)
    build_field_(op, src_domain, dest_domain, *src2, dest.instancie(Field<BigFloatTab> ));
  else if (src3)
    build_field_(op, src_domain, dest_domain, *src3, dest.instancie(Field<BigIntTab> ));
  else if (src4)
    build_field_(op, src_domain, dest_domain, *src3, dest.instancie(Field<BigTIDTab> ));
  else
    {
      Journal() << "Error in apply_field3: unsupported field type" << endl;
      throw;
    }
}

// See apply_field
template<class Op, class DomSrc>
void apply_field2(Op& op, const DomSrc& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest)
{
  const DomainUnstructured *d1 = dynamic_cast<const DomainUnstructured*>(&dest_domain);
  const DomainIJK *d2 = dynamic_cast<const DomainIJK*>(&dest_domain);
  if (d1)
    apply_field3(op, src_domain, src_field, *d1, dest);
  else if (d2)
    apply_field3(op, src_domain, src_field, *d2, dest);
  else
    {
      Journal() << "Error in apply_field2: unsupported destination domain type" << endl;
      throw;
    }
}

// This template calls the appropriate "build_field_()" method in the given operator.
//  The operator should implement non virtual methods for any usefull combination
//  of source domain type, destination domain type and source field type. This template
//  will call the correct method depending on the effective type of the parameters
//  (determined with dynamic_cast).
template<class Op>
void apply_field(Op& op, const Domain& src_domain, const LataField_base& src_field, const Domain& dest_domain, LataDeriv<LataField_base>& dest)
{
  const DomainUnstructured *d1 = dynamic_cast<const DomainUnstructured*>(&src_domain);
  const DomainIJK *d2 = dynamic_cast<const DomainIJK*>(&src_domain);
  if (d1)
    apply_field2(op, *d1, src_field, dest_domain, dest);
  else if (d2)
    apply_field2(op, *d2, src_field, dest_domain, dest);
  else
    {
      Journal() << "Error in apply_field: unsupported source domain type" << endl;
      throw;
    }
}

#endif
