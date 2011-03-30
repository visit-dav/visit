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

#include <Octree_Double.h>
#include <DoubleTab.h>

Octree_Double::Octree_Double()
{
  dim_ = 0;
}

void Octree_Double::reset()
{
  dim_ = 0;
  octree_int_.reset();
  origin_.reset();
  factor_.reset();
}

// Description: Convertit une coordonnees reele en coordonnee entiere pour l'octree_int
// Valeur de retour: 1 si ok, 0 si coordonnee hors de l'octree
inline entier Octree_Double::integer_position(double x, entier direction, entier &ix) const
{
  const double coord_max = (double) Octree_Int::coord_max_;
  double rnd_x = (x - origin_[direction]) * factor_[direction];
  // 0.49 permet d'accepter une coordonnee x egale a xmin ou xmax de l'octree,
  //  sinon pour un octree cree a partir de sommets, il y a un risque
  //  de ne pas trouver les coordonnees des points qu'on avait mis au bord de l'octree.
  if (rnd_x >= -0.49 && rnd_x <= coord_max + 0.49) {
    ix = (entier) floor(rnd_x + 0.5);
    return 1;
  }
  return 0;
}

// Valeur de retour: 1 s'il y a une intersection non vide avec l'octree, 0 sinon
inline entier Octree_Double::integer_position_clip(double xmin, double xmax, 
                                                   entier & x0, entier & x1, 
                                                   entier direction) const
{
  const double coord_max = (double) Octree_Int::coord_max_;
  xmin = (xmin - origin_[direction]) * factor_[direction];
  xmax = (xmax - origin_[direction]) * factor_[direction];
  // pas de marge ici comme on cherche avec une boite, l'epsilon est deja
  // dans la dimension de la boite.
  if (xmin > coord_max || xmax < 0.)
    return 0;
  if (xmin < .0)
    x0 = 0;
  else
    x0 = (entier) (floor(xmin+0.5));
  if (xmax > coord_max)
    x1 = Octree_Int::coord_max_;
  else
    x1 = (entier) (floor(xmax+0.5));
  return 1;
}

// Description: cherche les elements ou les points contenus dans l'octree_floor qui
//  contient le point (x,y,z). Renvoie le nombre n de ces elements.
//  Les indices des elements sont dans floor_elements()[index+i] pour 0 <= i < n
entier Octree_Double::search_elements(double x, double y, double z, entier & index) const
{
  if (dim_ == 0)
    return 0; // octree vide
  entier ix = 0, iy = 0, iz = 0;
  entier ok = integer_position(x, 0, ix)
    && integer_position(y, 1, iy)
    && integer_position(z, 2, iz);
  if (ok) {
    return octree_int_.search_elements(ix, iy, iz, index);
  } else {
    return 0;
  }
}

// Description: methode outil pour build_nodes et build_elements
//  (calcul des facteurs de conversion entre reels et entiers pour Octree_Int
void Octree_Double::compute_origin_factors(const DoubleTab & coords, 
                                           const double epsilon,
                                           const entier include_virtual)
{
  // Recherche des coordonnees min et max du domaine
  const entier nb_som = include_virtual ? coords.dimension_tot(0) : coords.dimension(0);
  if (nb_som == 0)
    return; // octree vide

  const entier dim = coords.dimension(1);
  dim_ = dim;
  origin_.resize_array(3);
  factor_.resize_array(3);
  ArrOfDouble xmin(dim, 1.e37);
  ArrOfDouble xmax(dim, -1.e-37);
  assert(dim >= 1 && dim <= 3);
  entier i, j;
  for (i = 0; i < nb_som; i++) {
    for (j = 0; j < dim; j++) {
      const double x = coords(i, j);
      if (x < xmin[j])
        xmin[j] = x;
      if (x > xmax[j])
        xmax[j] = x;
    }
  }
  const double coord_max = (double) Octree_Int::coord_max_;  
  for (j = 0; j < dim; j++) {
    xmin[j] -= epsilon;
    xmax[j] += epsilon;
    origin_[j] = xmin[j];
    if (xmax[j] - xmin[j] > 0.) {
      factor_[j] = coord_max / (xmax[j] - xmin[j]);
    } else
      factor_[j] = 0.;
  }
}

// Description: construit un octree contenant les points de coordonnees coords.
//  Si include_virtual=1, on stocke coords.dimension_tot(0) elements, sinon on en
//  stocke coords.dimension(0)
void Octree_Double::build_nodes(const DoubleTab & coords, const entier include_virtual)
{
  octree_int_.reset();
  compute_origin_factors(coords, 0. /* epsilon */, include_virtual);
  const entier nb_som = include_virtual ? coords.dimension_tot(0) : coords.dimension(0);
  if (nb_som == 0)
    return; // octree vide
  const entier dim = coords.dimension(1);
  IntTab elements_boxes(nb_som, dim);
  for (entier i = 0; i < nb_som; i++) {
    for (entier j = 0; j < dim; j++) {
      entier pos1 = 0;
      const double x = coords(i, j);
      if (!integer_position(x, j, pos1)) {
        Cerr << "Fatal error in octree : integer position outside octree" << finl;
        Process::exit();
      }
      elements_boxes(i, j) = pos1;
    }
  }
  octree_int_.build(dim, elements_boxes);
}

// Description: Construit un octree a partir d'elements volumiques decrits par des
//  ensembles de sommets. On stocke dans l'octree les parallelipipdes englobant chaque
//  element (contenant tous les sommets de l'element) plus une marge de epsilon.
//  Si include_virtual=1, on stocke elements.dimension_tot(0) elements, sinon on en
//  stocke elements.dimension(0) 
void Octree_Double::build_elements(const DoubleTab & coords, const IntTab & elements, 
                                   const double epsilon, const entier include_virtual)
{
  octree_int_.reset();
  compute_origin_factors(coords, epsilon, include_virtual);

  const entier nb_elems = include_virtual ? elements.dimension_tot(0) : elements.dimension(0);
  const entier nb_som_elem = elements.dimension(1);
  const entier dim = coords.dimension(1);
  IntTab elements_boxes(nb_elems, dim * 2);
  for (entier i = 0; i < nb_elems; i++) {
    for (entier j = 0; j < dim; j++) {
      double xmin = 1.e37;
      double xmax = -1.e37;
      for (entier k = 0; k < nb_som_elem; k++) {
        const entier som = elements(i, k);
        const double x = coords(som, j);
        xmin = (x<xmin) ? x : xmin;
        xmax = (x>xmax) ? x : xmax;
      }   
      entier pos1 = 0, pos2 = 0;
      if (!integer_position(xmin, j, pos1) || !integer_position(xmax, j, pos2)) {
        Cerr << "Fatal error in octree : integer position outside octree" << finl;
        Process::exit();
      }
      elements_boxes(i, j) = pos1;
      elements_boxes(i, j+dim) = pos2;
    }
  }
  octree_int_.build(dim, elements_boxes);
}

// Description: cherche tous les elements ou points ayant potentiellement une intersection
//  non vide avec la boite donnee.
entier Octree_Double::search_elements_box(double xmin, double ymin, double zmin,
                                          double xmax, double ymax, double zmax,
                                          ArrOfInt & elements) const
{
  const entier dim = dim_;
  if (dim == 0) {
    elements.resize_array(0);
    return 0;
  }
  entier x0 = 0, x1 = 0, y0 = 0, y1 = 0, z0 = 0, z1 = 0;
  entier ok = integer_position_clip(xmin, xmax, x0, x1, 0);
  if (ok && dim >= 1) {
    ok = integer_position_clip(ymin, ymax, y0, y1, 1);
    if (ok && dim >= 2)
      ok = integer_position_clip(zmin, zmax, z0, z1, 2);
  }
  if (ok)
    octree_int_.search_elements_box(x0, y0, z0, x1, y1, z1, elements);
  else
    elements.resize_array(0);
  return elements.size_array();
}

// Description: cherche tous les elements ou points ayant potentiellement une intersection
//  non vide avec la boite donnee (centre + ou - radius dans chaque direction)
entier Octree_Double::search_elements_box(const ArrOfDouble & center, const double radius,
                                          ArrOfInt & elements) const
{
  entier dim = center.size_array();
  double x = center[0];
  double y = (dim>=2) ? center[1] : 0.;
  double z = (dim>2) ? center[2] : 0.;
  entier i = search_elements_box(x-radius, y-radius, z-radius,
                                 x+radius, y+radius, z+radius,
                                 elements);
  return i;
}

// Description: Methode hors classe
//  Cherche parmi les sommets de la liste node_list ceux qui sont a une
//  distance inferieure a epsilon du point (x,y,z). node_list contient des indices de
//  sommets dans le tableau coords. La liste des noeuds verifiant le critere est mise
//  dans node_list. On renvoie l'indice dans le tableau coords du sommet le plus proche.
entier Octree_Double::search_nodes_close_to(double x, double y, double z,
                                            const DoubleTab & coords, ArrOfInt & node_list,
                                            double epsilon)
{
  const entier n = node_list.size_array();
  double eps2 = epsilon * epsilon;
  entier count = 0;
  const entier dim = coords.dimension(1);
  double dmin = eps2;
  entier nearest = -1;
  for (entier i = 0; i < n; i++) {
    const entier som = node_list[i];
    double dx = x - coords(som, 0);
    double dy = (dim >= 2) ? y - coords(som, 1) : 0.;
    double dz = (dim >= 3) ? z - coords(som, 2) : 0.;
    double d2 = dx * dx + dy * dy + dz * dz;
    if (d2 < eps2) {
      node_list[count] = som;
      if (d2 < dmin) {
        dmin = d2;
        nearest = som;
      }
      count++;
    }
  }
  node_list.resize_array(count);
  return nearest;
}

// Description: Idem que search_nodes_close_to(double x, double y, double z, ...)
entier Octree_Double::search_nodes_close_to(const ArrOfDouble & point,
                                            const DoubleTab & coords, ArrOfInt & node_list,
                                            double epsilon)
{
  entier dim = point.size_array();
  double x = point[0];
  double y = (dim>=2) ? point[1] : 0.;
  double z = (dim>2) ? point[2] : 0.;
  entier i = search_nodes_close_to(x, y, z, coords, node_list, epsilon);
  return i;
}
