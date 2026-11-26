/****************************************************************************
* Copyright (c) 2026, CEA
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
#include <Connectivite_som_elem.h>
#include <Static_Int_Lists.h>
#include <TRUSTTabs.h>

namespace
{
template <typename _SIZE_>
void copy_list_internal(const Static_Int_Lists_32_64<_SIZE_>& som_elem, const _SIZE_ sommet,
                        SmallArrOfTID_T<_SIZE_>& elements)
{ throw; }

template <>
void copy_list_internal(const Static_Int_Lists_32_64<int>& som_elem, const int sommet,
                        SmallArrOfTID_T<int>& elements)
{
  som_elem.copy_list_to_array(sommet, elements);
}
#if INT_is_64_ == 2
template <>
void copy_list_internal(const Static_Int_Lists_32_64<trustIdType>& som_elem, const trustIdType sommet,
                        SmallArrOfTID_T<trustIdType>& elements)
{
  BigArrOfTID elem_as_big;
  elements.ref_as_big(elem_as_big);
  som_elem.copy_list_to_array(sommet, elem_as_big);
}
#endif
}

/*! @brief Builds the som_elem structure for the given domain. Creates for each vertex i the list of elements adjacent to this vertex
 *
 *   (i.e. the list of elements k such that there exists j with les_elems(k,j) == i)
 *
 * @param (nb_sommets)
 * @param (les_elems)
 * @param (som_elem)
 * @param (include_virtual)
 */
template <typename _SIZE_>
void construire_connectivite_som_elem(const _SIZE_       nb_sommets,
                                      const IntTab_T<_SIZE_>&      les_elems,
                                      Static_Int_Lists_32_64<_SIZE_>& som_elem,
                                      bool       include_virtual)
{
  // Number of elements in the domain
  const _SIZE_ nb_elem = (include_virtual) ? les_elems.dimension_tot(0) : les_elems.dimension(0);
  // Number of vertices per element
  const _SIZE_ nb_sommets_par_element = les_elems.dimension(1);

  // Build an array initialized to zero: for each vertex,
  // the number of neighboring elements of this vertex
  ArrOfInt_T<_SIZE_> nb_elements_voisins(nb_sommets);

  // First pass: compute the number of neighboring elements of each
  // vertex in order to create the data structure
  ToDo_Kokkos("critical");
  for (_SIZE_ elem = 0; elem < nb_elem; elem++)
    {
      for (int i = 0; i < nb_sommets_par_element; i++)
        {
          _SIZE_ sommet = les_elems(elem, i);
          // GF case of polyhedra
          if (sommet==-1) break;
          nb_elements_voisins[sommet]++;
        }
    }
  som_elem.set_list_sizes(nb_elements_voisins);

  // Reuse the array to store the number of elements in
  // each list while filling it
  nb_elements_voisins = 0;

  // Fill the array of neighboring elements.
  ToDo_Kokkos("critical");
  for (_SIZE_ elem = 0; elem < nb_elem; elem++)
    {
      for (int i = 0; i < nb_sommets_par_element; i++)
        {
          _SIZE_ sommet = les_elems(elem, i);
          // GF case of polyhedra
          if (sommet==-1) break;
          _SIZE_ n = (nb_elements_voisins[sommet])++;
          som_elem.set_value(sommet, n, elem);
        }
    }

  // Sort all lists in ascending order
  som_elem.trier_liste(-1);
}

/*! @brief Finds the elements that contain all the vertices of the sommets_to_find array (allows finding elements
 *
 *   adjacent to a face or an edge)
 *
 * @param (som_elem) for each vertex, sorted list of adjacent elements (see construire_connectivite_som_elem)
 * @param (sommets_to_find) a list of vertices
 * @param (elements) result of the search: the list of elements containing all vertices of sommets_to_find. If sommets_to_find is empty, an empty array is returned. (for repeated calls to this function, it is advised to set the "smart_resize" flag)
 */
template <typename _SIZE_>
void find_adjacent_elements(const Static_Int_Lists_32_64<_SIZE_>& som_elem,
                            const SmallArrOfTID_T<_SIZE_>& sommets_to_find,
                            SmallArrOfTID_T<_SIZE_>& elements)
{
  int nb_som_to_find = sommets_to_find.size_array();
  // remove vertices equal to -1 (case of multiple face types)
  while (sommets_to_find[nb_som_to_find-1]==-1) nb_som_to_find--;
  if (nb_som_to_find == 0)
    {
      elements.resize_array(0);
      return;
    }
  // Algorithm: initialise elements with all elements adjacent
  //  to the first vertex of the list.
  //  Then for each of the other vertices in the list, remove from the array
  //  "elements" those elements that are not neighbors of the vertex.
  //  At the end, only elements that are in all lists remain.
  {
    // Initialization with the elements adjacent to the first vertex
    const _SIZE_ sommet = sommets_to_find[0];
    // OK this is a bit technical here: 'elements' is a small array, even in 64b.
    // But copy_list_to_array() might return a Big array (in 64b). So we cheat, we pass it a big array
    // which is actually pointing to the same internal memory block as the small one.
    // Just need to start with the correct size because copy_list_to_array will resize otherwise:
    int sz = (int)som_elem.get_list_size(sommet);
    elements.resize_array(sz);
    ::copy_list_internal<_SIZE_>(som_elem, sommet, elements);
  }
  int nb_elem_found = elements.size_array();
  int i_sommet;
  for (i_sommet = 1; i_sommet < nb_som_to_find; i_sommet++)
    {
      const _SIZE_ sommet = sommets_to_find[i_sommet];
      // Compute the common elements between elements[.] and som_elem(sommet,.)
      // Number of common elements between elements and the new vertex list
      int nb_elems_restants = 0;
      // Number of elements adjacent to "sommet"
      const int nb_elem_liste = (int)som_elem.get_list_size(sommet);
      // Assume the element lists are sorted in ascending order
      // Traverse both lists simultaneously and keep the common elements
      int i=0, j=0;
      if (nb_elem_found == 0)
        break;
      if (nb_elem_liste > 0)
        {
          while (1)
            {
              const _SIZE_ elem_i = elements[i];
              const _SIZE_ elem_j = som_elem(sommet, j);
              if (elem_i == elem_j)
                {
                  // Element common to both lists, keep it
                  elements[nb_elems_restants] = elem_i;
                  nb_elems_restants++;
                }
              if (elem_i >= elem_j)
                {
                  j++;
                  if (j >= nb_elem_liste)
                    break;
                }
              if (elem_j >= elem_i)
                {
                  i++;
                  if (i >= nb_elem_found)
                    break;
                }
            }
        }
      else
        {
          nb_elems_restants = 0;
        }
      nb_elem_found = nb_elems_restants;
    }
  elements.resize_array(nb_elem_found);
}

template void construire_connectivite_som_elem(const int nb_sommets, const IntTab_T<int>& les_elems, Static_Int_Lists_32_64<int>& som_elem, bool include_virtual);
template void find_adjacent_elements(const Static_Int_Lists_32_64<int>& som_elem, const SmallArrOfTID_T<int>& sommets_to_find, SmallArrOfTID_T<int>& elements);

#if INT_is_64_ == 2
template void construire_connectivite_som_elem(const trustIdType nb_sommets, const IntTab_T<trustIdType>& les_elems, Static_Int_Lists_32_64<trustIdType>& som_elem, bool include_virtual);
template void find_adjacent_elements(const Static_Int_Lists_32_64<trustIdType>& som_elem, const SmallArrOfTID_T<trustIdType>& sommets_to_find, SmallArrOfTID_T<trustIdType>& elements);
#endif
