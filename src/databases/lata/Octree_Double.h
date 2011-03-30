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


#ifndef Octree_Double_inclu
#define Octree_Double_inclu
#include <Octree_Int.h>
#include <ArrOfDouble.h>
class DoubleTab;

// .DESCRIPTION : Un octree permettant de chercher dans l'espace des elements ou des points
//  decrits par des coordonnees reeles. Cet objet est base sur Octree_Int.
class Octree_Double
{
public:
  Octree_Double();
  void   reset();
  void   build_elements(const DoubleTab & coords, const IntTab & elements, 
                        const double epsilon, const entier include_virtual);
  void   build_nodes(const DoubleTab & coords, const entier include_virtual);
  entier search_elements(double x, double y, double z, entier & index) const;
  entier search_elements_box(double xmin, double ymin, double zmin,
                             double xmax, double ymax, double zmax,
                             ArrOfInt & elements) const;
  static entier search_nodes_close_to(double x, double y, double z,
                                      const DoubleTab & coords, ArrOfInt & node_list,
                                      double epsilon);
  entier search_elements_box(const ArrOfDouble & center, const double radius,
                             ArrOfInt & elements) const;
  static entier search_nodes_close_to(const ArrOfDouble & point,
                                      const DoubleTab & coords, ArrOfInt & node_list,
                                      double epsilon);
  entier dimension() const { assert(dim_ > 0); return dim_; }
  inline const ArrOfInt & floor_elements() const { return octree_int_.floor_elements(); };
protected:
  inline entier integer_position(double x, entier direction, entier &ix) const;
  inline entier integer_position_clip(double xmin, double xmax, 
                                      entier & x0, entier & x1, 
                                      entier direction) const;
  void compute_origin_factors(const DoubleTab & coords, 
                              const double epsilon,
                              const entier include_virtual);

  Octree_Int octree_int_;
  // Ces deux tableaux sont toujours de taille 3 par commodite
  ArrOfDouble origin_;
  ArrOfDouble factor_;
  entier dim_;
};
#endif
