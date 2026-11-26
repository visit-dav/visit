/****************************************************************************
* Copyright (c) 2024, CEA
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

#include <Octree_Double.h>

template <typename _SIZE_>
void Octree_Double_32_64<_SIZE_>::reset()
{
  dim_ = 0;
  octree_int_.reset();
  origin_.reset();
  factor_.reset();
}

/*! @brief searches for the elements or points contained in the octree_floor that contains the point (x,y,z).
 *
 * Returns the number n of these elements.
 *   The indices of the elements are in floor_elements()[index+i] for 0 <= i < n
 *
 */
template <typename _SIZE_>
typename Octree_Double_32_64<_SIZE_>::int_t Octree_Double_32_64<_SIZE_>::search_elements(double x, double y, double z, int_t& index) const
{
  if (dim_ == 0)
    return 0; // empty octree
  int ix = 0, iy = 0, iz = 0;
  int ok = integer_position(x, 0, ix)
           && integer_position(y, 1, iy)
           && integer_position(z, 2, iz);
  return ok ? octree_int_.search_elements(ix, iy, iz, index) : 0;
}

/*! @brief builds an octree containing the points with coordinates coords.
 *
 * If include_virtual=1, stores coords.dimension_tot(0) elements, otherwise stores
 *   coords.dimension(0) elements.
 *   If epsilon = 0, builds an octree of zero-size points (each point
 *    is in a single octree_floor).
 *   Otherwise, builds an octree of cubic elements centered on the coords, with half-width epsilon.
 *    A point can then be found in several octree_floors.
 *
 */
template <typename _SIZE_>
void Octree_Double_32_64<_SIZE_>::build_nodes(const DoubleTab_t& coords, const bool include_virtual, const double epsilon)
{
  octree_int_.reset();
  compute_origin_factors(coords, epsilon, include_virtual);
  const int_t nb_som = include_virtual ? coords.dimension_tot(0) : coords.dimension(0);
  if (nb_som == 0)
    return; // empty octree
  const int dim = coords.dimension_int(1);
  if (epsilon < 0.)
    {
      Cerr << "Internal error in Octree_Double_32_64<_SIZE_>::build_nodes: negative epsilon" << finl;
      Process::exit();
    }
  bool have_epsilon = (epsilon != 0.);
  IntTab_t elements_boxes;
  elements_boxes.resize(nb_som, have_epsilon ? (dim*2) : dim, RESIZE_OPTIONS::NOCOPY_NOINIT);

  for (int_t i = 0; i < nb_som; i++)
    for (int j = 0; j < dim; j++)
      {
        int pos1 = 0;
        const double x0 = coords(i, j);
        double x = x0 - epsilon;
        if (!integer_position(x, j, pos1))
          {
            Cerr << "Fatal error in octree : integer position outside octree" << finl;
            Process::exit();
          }
        elements_boxes(i, j) = pos1;
        if (have_epsilon)
          {
            pos1 = 0;
            double xbis = x0 + epsilon;
            if (!integer_position(xbis, j, pos1))
              {
                Cerr << "Fatal error in octree : integer position outside octree" << finl;
                Process::exit();
              }
            elements_boxes(i, dim+j) = pos1;
          }
      }
  octree_int_.build(dim, elements_boxes);
}

/*! @brief searches for all elements or points potentially having a non-empty intersection with the given box.
 *
 */
template <typename _SIZE_>
typename Octree_Double_32_64<_SIZE_>::int_t Octree_Double_32_64<_SIZE_>::search_elements_box(double xmin, double ymin, double zmin,
                                                                                             double xmax, double ymax, double zmax,
                                                                                             ArrOfInt_t& elements) const
{
  const int dim = dim_;
  if (dim == 0)
    {
      elements.resize_array(0);
      return 0;
    }
  int x0 = 0, x1 = 0, y0 = 0, y1 = 0, z0 = 0, z1 = 0;
  int ok = integer_position_clip(xmin, xmax, x0, x1, 0);
  if ((ok) && (dim >= 1))
    {
      ok = integer_position_clip(ymin, ymax, y0, y1, 1);
      if ((ok) && (dim >= 2))
        ok = integer_position_clip(zmin, zmax, z0, z1, 2);
    }
  if (ok)
    octree_int_.search_elements_box(x0, y0, z0, x1, y1, z1, elements);
  else
    elements.resize_array(0);
  return elements.size_array();
}

/*! @brief Searches for all elements or points potentially having a non-empty intersection with the given box (center +/- radius in each direction).
 *
 * @param center Center of the box.
 * @param radius Half-width of the box in each direction.
 * @param elements Array filled with the indices of the matching elements.
 * @return Number of elements found.
 */
template <typename _SIZE_>
typename Octree_Double_32_64<_SIZE_>::int_t
Octree_Double_32_64<_SIZE_>::search_elements_box(const ArrOfDouble& center, const double radius,
                                                 ArrOfInt_t& elements) const
{
  int dim = center.size_array();
  double x = center[0];
  double y = (dim>=2) ? center[1] : 0.;
  double z = (dim>2) ? center[2] : 0.;
  int_t i = search_elements_box(x-radius, y-radius, z-radius,
                                x+radius, y+radius, z+radius,
                                elements);
  return i;
}

/*! @brief Non-member method. Searches among the vertices in node_list for those within a distance
 *
 *   less than epsilon from the point (x,y,z). node_list contains indices of
 *   vertices in the coords array. The list of nodes satisfying the criterion is placed
 *   in node_list. Returns the index in the coords array of the nearest vertex.
 *
 */
template <typename _SIZE_>
typename Octree_Double_32_64<_SIZE_>::int_t  Octree_Double_32_64<_SIZE_>::search_nodes_close_to(double x, double y, double z,
                                                                                                const DoubleTab_t& coords, ArrOfInt_t& node_list,
                                                                                                double epsilon)
{
  const int_t n = node_list.size_array();
  double eps2 = epsilon * epsilon;
  int_t count = 0;
  const int dim = coords.dimension_int(1);
  double dmin = eps2;
  int_t nearest = -1;
  for (int_t i = 0; i < n; i++)
    {
      const int_t som = node_list[i];
      double dx = x - coords(som, 0);
      double dy = (dim >= 2) ? y - coords(som, 1) : 0.;
      double dz = (dim >= 3) ? z - coords(som, 2) : 0.;
      double d2 = dx * dx + dy * dy + dz * dz;
      if (d2 < eps2)
        {
          node_list[count] = som;
          if (d2 < dmin)
            {
              dmin = d2;
              nearest = som;
            }
          count++;
        }
    }
  node_list.resize_array(count);
  return nearest;
}

/*! @brief Same as search_nodes_close_to(double x, double y, double z, ...)
 *
 */
template <typename _SIZE_>
typename Octree_Double_32_64<_SIZE_>::int_t  Octree_Double_32_64<_SIZE_>::search_nodes_close_to(const ArrOfDouble& point,
                                                                                                const DoubleTab_t& coords, ArrOfInt_t& node_list,
                                                                                                double epsilon)
{
  int dim = point.size_array();
  double x = point[0];
  double y = (dim>=2) ? point[1] : 0.;
  double z = (dim>2) ? point[2] : 0.;
  int_t i = search_nodes_close_to(x, y, z, coords, node_list, epsilon);
  return i;
}

template class Octree_Double_32_64<int>;
#if INT_is_64_ == 2
template class Octree_Double_32_64<trustIdType>;
#endif

