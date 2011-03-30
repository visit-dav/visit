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

#ifndef Octree_Int_inclus
#define Octree_Int_inclus
#include <IntTab.h>
#include <VectArrOfInt.h>

struct IntBoxData;
class ArrOfBit;

// .DESCRIPTION : Un octree permettant de retrouver des objets ponctuels ou
//   parallelipipediques dans un espace 1D, 2D ou 3D et des coordonnees entieres
class Octree_Int
{
 public:
  void   build(const entier dimension, const IntTab & elements_boxes);
  entier search_elements(entier x, entier y, entier z, entier & floor_elements_index) const;
  entier search_elements_box(entier xmin, entier ymin, entier zmin,
                             entier xmax, entier ymax, entier zmax,
                             ArrOfInt & elements) const;
  void   reset();

  inline const ArrOfInt & floor_elements() const { return floor_elements_; };

  // Le plus grand entier autorise pour les coordonnees (du type 2^n - 1)
  static const entier coord_max_;
  // Premier entier de la moitie superieure de l'octree root (si coord_max_=2^n-1, half_width_=2^(n-1))
  static const entier root_octree_half_width_;
 protected:
  entier build_octree_recursively(const entier octree_center_x, const entier octree_center_y, const entier octree_center_z,
                                  const entier octree_half_width,
                                  const IntTab & elements_boxes,
                                  VECT(ArrOfInt) & vect_elements_list,
                                  const entier level,
                                  VECT(ArrOfInt) & tmp_elem_flags);
  entier build_octree_floor(const ArrOfInt & elements_list);

  entier search_octree_floor(entier x_pos, entier y_pos, entier z_pos) const;
  void   search_elements_box_floor(IntBoxData & boxdata,
                                   entier octree_floor_id) const;
  void   search_elements_box_recursively(IntBoxData & boxdata,
                                         entier octree_id,
                                         entier cx, entier cy, entier cz,
                                         entier half_width) const;

  // Un octree peut etre soit vide, soit subdivise en nb_octrees autres octrees,
  // soit un octree_floor contenant une liste d'elements.
  enum Octree_Type { EMPTY, OCTREE, FLOOR };

  static inline entier octree_id(entier index, Octree_Type type);
  static inline entier octree_index(entier octree_id, Octree_Type type);
  static inline Octree_Type octree_type(entier octree_id);

  // Octree_id du cube principal : peut etre EMPTY, OCTREE ou FLOOR
  entier root_octree_id_;
  // Nombre d'elements stockes (dimension(0) du tableau elements_boxes)
  entier nb_elements_;
  // Tableau contenant tous les cubes qui sont divises en sous-cubes
  //  octree_structure_(i, j) decrit le contenu du sous-cube j du cube d'index i.
  //  pour 0 <= j < nombre de sous-cubes par cube.
  //  On appelle "octree_id" une valeur X=octree_structure_(i,j) (identifiant octree)
  //  L'octree id encode a la fois le type de l'octree et l'index ou
  //  il se trouve dans les tableaux (voir octree_id(entier, Octree_Type))
  IntTab octree_structure_;
  
  // Tableau contenant la liste des elements de chaque sous-cube final non subdivise.
  // Si X < 0, on note i_debut = -X-1.
  // floor_elements_(i_debut) = n = nombre d'elements dans ce sous-cube
  // floor_elements_[i_debut+j] = numero d'un element qui coupe ce sous-cube pour 1 <= j <= n
  ArrOfInt floor_elements_;
};

#endif
