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

#ifndef Octree_Double_TPP_included
#define Octree_Double_TPP_included

/*! @brief Builds an octree from volumetric elements described by sets of vertices.
 *
 * Stores in the octree the bounding parallelepipeds of each
 *   element (containing all vertices of the element) plus a margin of epsilon.
 *   If include_virtual=1, stores elements.dimension_tot(0) elements, otherwise
 *   stores elements.dimension(0).
 *
 * Templated because the float version is needed for lata tools.
 *
 */
template<typename _SIZE_>
template<class _TAB_TYPE_>
void Octree_Double_32_64<_SIZE_>::build_elements(const _TAB_TYPE_& coords, const IntTab_t& elements,
                                                 const double epsilon, const bool include_virtual)
{
  octree_int_.reset();
  compute_origin_factors(coords, epsilon, include_virtual);

  const int_t nb_elems = include_virtual ? elements.dimension_tot(0) : elements.dimension(0);
  const int nb_som_elem = elements.dimension_int(1);
  const int dim = coords.dimension_int(1);
  IntTab_t elements_boxes;
  elements_boxes.resize(nb_elems, dim * 2, RESIZE_OPTIONS::NOCOPY_NOINIT);
  for (int_t i = 0; i < nb_elems; i++)
    {
      for (int j = 0; j < dim; j++)
        {
          double xmin = 1.e37;
          double xmax = -1.e37;
          for (int k = 0; k < nb_som_elem; k++)
            {
              const int_t som = elements(i, k);
              if (som < 0) continue; // for polyhedrons, som might be -1 (padding)
              const double x = coords(som, j);
              xmin = (x<xmin) ? x : xmin;
              xmax = (x>xmax) ? x : xmax;
            }
          int pos1 = 0, pos2 = 0;
          if (!integer_position(xmin, j, pos1) || !integer_position(xmax, j, pos2))
            {
              Cerr << "Fatal error in octree : integer position outside octree" << finl;
              Process::exit();
            }
          elements_boxes(i, j) = pos1;
          elements_boxes(i, j+dim) = pos2;
        }
    }
  octree_int_.build(dim, elements_boxes);
}

/*! @brief Helper method for build_nodes and build_elements: computes the conversion factors from real to integer coordinates for Octree_Int.
 *
 *   epsilon is added to the min and max coordinates for the octree bounding box.
 *
 * Templated because the float version is needed for lata tools.
 */
template<typename _SIZE_>
template<class _TAB_TYPE_>
void Octree_Double_32_64<_SIZE_>::compute_origin_factors(const _TAB_TYPE_& coords,
                                                         const double epsilon,
                                                         const int include_virtual)
{
  // Search for the min and max coordinates of the domain
  const int_t nb_som = include_virtual ? coords.dimension_tot(0) : coords.dimension(0);
  if (nb_som == 0) return; // empty octree

  dim_ = coords.dimension_int(1);
  origin_.resize_array(3);
  factor_.resize_array(3);
  ArrOfDouble_t xmin(dim_);
  xmin = 1.e37;
  ArrOfDouble_t xmax(dim_);
  xmax = -1.e-37;
  assert(dim_ >= 1 && dim_ <= 3);
  for (int_t i = 0; i < nb_som; i++)
    for (int j = 0; j < dim_; j++)
      {
        const double x = coords(i, j);
        if (x < xmin[j])
          xmin[j] = x;
        if (x > xmax[j])
          xmax[j] = x;
      }

  const double coord_max = (double) Octree_Int_32_64<_SIZE_>::coord_max_;
  for (int j = 0; j < dim_; j++)
    {
      xmin[j] -= epsilon;
      xmax[j] += epsilon;
      origin_[j] = xmin[j];
      if (xmax[j] - xmin[j] > 0.)
        factor_[j] = coord_max / (xmax[j] - xmin[j]);
      else
        factor_[j] = 0.;
    }
}

#endif
