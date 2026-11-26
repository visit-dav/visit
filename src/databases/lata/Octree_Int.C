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
#include <Octree_Int.h>

static constexpr int max_levels_ = 32; // 1 more than the number of bits=1 in coords_max
// The following value must be a power of two
template<> const int Octree_Int_32_64<int>::root_octree_half_width_ = 1073741824; /* 2^30   = 0100 0000  0000 0000  0000 0000  0000 0000b */
// The following value must be equal to (root_octree_half_width_ * 2 - 1)
template<> const int Octree_Int_32_64<int>::coord_max_ = 2147483647;              /* 2^31-1 = 0111 1111  1111 1111  1111 1111  1111 1111b */


#if INT_is_64_ == 2
template<> const int Octree_Int_32_64<trustIdType>::root_octree_half_width_ = 1073741824;
template<> const int Octree_Int_32_64<trustIdType>::coord_max_ = 2147483647;
#endif

/*! @brief builds an octree_id (see octree_structure_)
 *
 *   If type==EMPTY, the octree_id is 0
 *   If type==OCTREE, index is assumed to be an index into octree_structure_
 *   If type==FLOOR, index is assumed to be an index into floor_elements_
 */
template <typename _SIZE_>
inline typename Octree_Int_32_64<_SIZE_>::int_t Octree_Int_32_64<_SIZE_>::octree_id(int_t index, Octree_Type type)
{
  switch(type)
    {
    case EMPTY:
      return 0;
    case OCTREE:
      return index + 1;
    case FLOOR:
      return - index - 1;
    }
  return -1;
}

/*! @brief computes the index of the octree in octree_structure or floor_elements based on the type of the octree and its octree_id.
 *
 *   In general the type is already known, so it is passed as a parameter for efficiency.
 *
 */
template <typename _SIZE_>
inline typename Octree_Int_32_64<_SIZE_>::int_t Octree_Int_32_64<_SIZE_>::octree_index(int_t octree_id, Octree_Type type)
{
  assert(type==octree_type(octree_id));
  switch(type)
    {
    case EMPTY:
      return -1;
    case OCTREE:
      return octree_id - 1;
    case FLOOR:
      return - octree_id - 1;
    }
  return -1;
}

/*! @brief Returns the type of an octree based on its octree_id.
 *
 */
template <typename _SIZE_>
inline typename Octree_Int_32_64<_SIZE_>::Octree_Type Octree_Int_32_64<_SIZE_>::octree_type(int_t octree_id)
{
  if (octree_id > 0)
    return OCTREE;
  else if (octree_id == 0)
    return EMPTY;
  else
    return FLOOR;
}

/*! @brief builds the octree.
 *
 * The dimension (1, 2 or 3) and an array of elements to store in the octree are given. Two possibilities:
 *   1) elements are point-like if elements_boxes.dimension(1) == dimension.
 *      In this case, each element belongs to exactly one octree_floor
 *   2) elements are parallelepipeds, if elements_boxes.dimension(1) == dimension*2
 *      The first "dimension" columns are the lower coordinates,
 *      the next "dimension" columns are the upper coordinates.
 *      A parallelepiped may be assigned to several octree_floors.
 *   Coordinates stored in elements_boxes range from 0 to coord_max_ inclusive.
 *   It is best to use the full integer range by multiplying by an appropriate factor.
 *
 */
template <typename _SIZE_>
void Octree_Int_32_64<_SIZE_>::build(const int dimension, const IntTab_t& elements_boxes)
{
  assert(dimension >= 1 && dimension <= 3);
  assert(elements_boxes.dimension(1) == dimension
         || elements_boxes.dimension(1) == dimension * 2 );

  const int_t nb_elems = elements_boxes.dimension(0);
  nb_elements_ = nb_elems;

  floor_elements_.resize_array(0);
  const int nb_octrees = 1 << dimension;  // = 2^dim
  octree_structure_.resize(0, nb_octrees);

  assert(elements_boxes.size_array() == 0
         || (min_array(elements_boxes) >= 0 && max_array(elements_boxes) <= coord_max_));

  AOFlagS_ tmp_elem_flags(max_levels_);
  ArrsOfInt_t tmp_elem_list(max_levels_);
  ArrOfInt_t& elements_list = tmp_elem_list[0];
  elements_list.resize_array(nb_elems, RESIZE_OPTIONS::NOCOPY_NOINIT);
  for (int_t i = 0; i < nb_elems; i++)
    elements_list[i] = i;

  root_octree_id_ = build_octree_recursively(root_octree_half_width_,root_octree_half_width_,root_octree_half_width_,
                                             root_octree_half_width_,
                                             elements_boxes,
                                             tmp_elem_list,
                                             0,
                                             tmp_elem_flags);
}

/*! @brief returns the list of elements potentially containing the point (x,y,z). Returns n=number of elements in the list, and the elements are in
 *
 *   floor_elements()[index+i] for 0 <= i < n.
 *   In practice, all elements with a non-empty intersection with the octree_floor
 *   containing the point (x,y,z) are returned.
 *
 */
template <typename _SIZE_>
typename Octree_Int_32_64<_SIZE_>::int_t Octree_Int_32_64<_SIZE_>::search_elements(int x, int y, int z, int_t& index) const
{
  const int nb_octrees = octree_structure_.dimension_int(1);
  if (nb_octrees == 2)
    y = 0; // important to avoid landing on non-existent cubes
  if (nb_octrees <= 4)
    z = 0; // same
  assert(x >= 0 && x <= coord_max_);
  assert(y >= 0 && y <= coord_max_);
  assert(z >= 0 && z <= coord_max_);

  const int_t my_octree_id = search_octree_floor(x, y, z);

  if (octree_type(my_octree_id) == EMPTY)
    return 0;

  const int_t idx = octree_index(my_octree_id, FLOOR);
  const int_t n = floor_elements_[idx];
  index = idx + 1;
  return n;
}

template <typename _SIZE_>
struct IntBoxData
{
  using ArrOfInt_t = ArrOfInt_T<_SIZE_>;
  using AOBit_ = ArrOfBit_32_64<_SIZE_>;

  IntBoxData(int xmin, int ymin, int zmin,
             int xmax, int ymax, int zmax,
             ArrOfInt_t& elements,
             AOBit_ *markers) :
    xmin_(xmin), ymin_(ymin), zmin_(zmin),
    xmax_(xmax), ymax_(ymax), zmax_(zmax),
    elements_(elements),
    markers_(markers) { };
  int xmin_, ymin_, zmin_;
  int xmax_, ymax_, zmax_;
  ArrOfInt_t& elements_;
  AOBit_ *markers_;
};

/*! @brief searches for elements potentially having a non-empty intersection
 * with the box xmin..zmax.
 *   Elements may appear more than once in the "elements" array.
 */
template <typename _SIZE_>
typename Octree_Int_32_64<_SIZE_>::int_t Octree_Int_32_64<_SIZE_>::search_elements_box(int xmin, int ymin, int zmin,
                                                                                       int xmax, int ymax, int zmax,
                                                                                       ArrOfInt_t& elements) const
{
  const int nb_octrees = octree_structure_.dimension_int(1);
  if (nb_octrees == 2)
    ymin = ymax = 0; // important to avoid landing on non-existent cubes
  if (nb_octrees <= 4)
    zmin = zmax = 0; // same
  assert(xmin >= 0 && xmin <= coord_max_);
  assert(ymin >= 0 && ymin <= coord_max_);
  assert(zmin >= 0 && zmin <= coord_max_);
  assert(xmax >= 0 && xmax <= coord_max_);
  assert(ymax >= 0 && ymax <= coord_max_);
  assert(zmax >= 0 && zmax <= coord_max_);

  elements.resize_array(0);
  IntBoxData<_SIZE_> boxdata(xmin, ymin, zmin, xmax, ymax, zmax, elements, 0);
  switch(octree_type(root_octree_id_))
    {
    case FLOOR:
      search_elements_box_floor(boxdata, root_octree_id_);
      break;
    case OCTREE:
      search_elements_box_recursively(boxdata, root_octree_id_,
                                      root_octree_half_width_,root_octree_half_width_,root_octree_half_width_,
                                      root_octree_half_width_);
      break;
    case EMPTY:
      break;
    }
  const int_t n = elements.size_array();
  return n;
}

/*! @brief adds elements from the octree_floor to boxdata.
 *
 * elements_
 *
 */
template <typename _SIZE_>
void Octree_Int_32_64<_SIZE_>::search_elements_box_floor(IntBoxData<_SIZE_>& boxdata,
                                                         int_t octree_floor_id) const
{
  const int_t idx = octree_index(octree_floor_id, FLOOR);
  const int_t n = floor_elements_[idx];
  if (boxdata.markers_)
    for (int_t i = 0; i < n; i++)
      {
        const int_t elem = floor_elements_[idx+1+i];
        if (!boxdata.markers_->testsetbit(elem))
          boxdata.elements_.append_array(elem);
      }
  else
    for (int_t i = 0; i < n; i++)
      {
        const int_t elem = floor_elements_[idx+1+i];
        boxdata.elements_.append_array(elem);
      }
}

// For each direction, flags of the cubes in the lower row
static int sub_cube_flags_min[3] = { 1+4+16+64, /* flags of cubes 0,2,4,6 */
                                     1+2+16+32, /* flags of cubes 0,1,4,5 */
                                     1+2+4+8    /* flags of cubes 0,1,2,3 */
                                   };
static int sub_cube_flags_max[3] = { 2+8+32+128, /* flags of cubes 1,3,5,7 */
                                     4+8+64+128, /* flags of cubes 2,3,7,8 */
                                     16+32+64+128 /* flags of cubes 4,5,6,7 */
                                   };

/*! @brief recursively searches for elements included in the box boxdata for the given octree_id, centred at cx, cy, cz.
 *
 */
template <typename _SIZE_>
void Octree_Int_32_64<_SIZE_>::search_elements_box_recursively(IntBoxData<_SIZE_>& boxdata,
                                                               int_t the_octree_id,
                                                               int cx, int cy, int cz,
                                                               int half_width) const
{
  int flags = 255;
  if (cx > boxdata.xmax_) // upper cubes in x are not inside
    flags &= sub_cube_flags_min[0];
  if (cx <= boxdata.xmin_) // lower cubes are not inside
    flags &= sub_cube_flags_max[0];
  if (cy > boxdata.ymax_)
    flags &= sub_cube_flags_min[1];
  if (cy <= boxdata.ymin_)
    flags &= sub_cube_flags_max[1];
  if (cz > boxdata.zmax_)
    flags &= sub_cube_flags_min[2];
  if (cz <= boxdata.zmin_)
    flags &= sub_cube_flags_max[2];
  int test_flag = 1;
  const int_t idx = octree_index(the_octree_id, OCTREE);
  const int half_width_2 = half_width >> 1;
  const int mhalf_width = - half_width_2;
  int cx2, cy2, cz2;
  for (int i = 0; i < 8; i++, test_flag <<= 1)
    {
      if ((flags & test_flag) != 0)
        {
          const int_t id = octree_structure_(idx, i);
          switch(octree_type(id))
            {
            case FLOOR:
              search_elements_box_floor(boxdata, id);
              break;
            case OCTREE:
              cx2 = cx + ((i & 1) ? half_width_2 : mhalf_width);
              cy2 = cy + ((i & 2) ? half_width_2 : mhalf_width);
              cz2 = cz + ((i & 4) ? half_width_2 : mhalf_width);
              search_elements_box_recursively(boxdata, id,
                                              cx2, cy2, cz2,
                                              half_width_2);
              break;
            case EMPTY:
              break;
            }
        }
    }
}

template <typename _SIZE_>
void Octree_Int_32_64<_SIZE_>::reset()
{
  root_octree_id_ = octree_id(0, EMPTY);
  nb_elements_ = 0;
  octree_structure_.reset();
  floor_elements_.reset();
}

/*! @brief builds an octree_floor with the given list of elements and returns the octree_id of that octree_floor
 *
 */
template <typename _SIZE_>
typename Octree_Int_32_64<_SIZE_>::int_t Octree_Int_32_64<_SIZE_>::build_octree_floor(const ArrOfInt_t& elements_list)
{
  const int_t nb_elems = elements_list.size_array();
  const int_t index = floor_elements_.size_array();
  floor_elements_.resize_array(index + nb_elems + 1, RESIZE_OPTIONS::COPY_NOINIT);
  floor_elements_[index] = nb_elems;
  for (int_t i = 0; i < nb_elems; i++)
    floor_elements_[index + 1 + i] = elements_list[i];
  return octree_id(index, FLOOR);
}

/*! @brief octree_center_i is the first int of the upper half of the octree in direction i.
 *
 * octree_half_width is a power of 2 equal to octree_center_i-octree_min_i (octree_min_i
 *    is the first int included in this octree in direction i)
 *  Return value: octree_id of the built octree (see octree_structure_)
 *
 */
template <typename _SIZE_>
typename Octree_Int_32_64<_SIZE_>::int_t Octree_Int_32_64<_SIZE_>::build_octree_recursively(const int octree_center_x,
                                                                                            const int octree_center_y,
                                                                                            const int octree_center_z,
                                                                                            const int octree_half_width,
                                                                                            const IntTab_t& elements_boxes,
                                                                                            ArrsOfInt_t& vect_elements_list,
                                                                                            const int level,
                                                                                            AOFlagS_& tmp_elem_flags)
{
  // Stopping criteria for subdivision:
  // Maximum number of elements in a floor sub-cube
  constexpr int OCTREE_FLOOR_MAX_ELEMS = 8;
  // If there are many duplicate elements, but not too many, and the number of elements
  //  in the octree is above this value, we still subdivide
  constexpr int OCTREE_DUPLICATE_ELEMENTS_LIMIT = 64;
  const ArrOfInt_t& elements_list = vect_elements_list[level];
  // If the number of elements is below the limit, create a floor_element,
  // otherwise subdivide
  const int_t nb_elems = elements_list.size_array();
  if (nb_elems == 0)
    return octree_id(0, EMPTY);

  if (nb_elems < OCTREE_FLOOR_MAX_ELEMS || octree_half_width == 1 /* last level */)
    {
      const int_t the_octree_id = build_octree_floor(elements_list);
      return the_octree_id;
    }

  AOFlag_& elem_flags = tmp_elem_flags[level];
  elem_flags.resize_array(nb_elems, RESIZE_OPTIONS::NOCOPY_NOINIT);

  const int nb_octrees = octree_structure_.dimension_int(1);
  assert(nb_octrees == 2 || nb_octrees == 4 || nb_octrees == 8);
  const int elem_box_dim = elements_boxes.dimension_int(1);
  // Either elements_boxes contains dimension columns, or dimension*2
  const int box_delta = (elem_box_dim > 3) ? (elem_box_dim >> 1) : 0;
  // Number of elements stored twice in the octree (due to elements straddling
  //  several sub-octrees)
  int_t nb_duplicate_elements = 0;
  // Assign the elements of the list to 8 sub-cubes (filling elem_flags)
  for (int_t i_elem = 0; i_elem < nb_elems; i_elem++)
    {
      const int_t elem = elements_list[i_elem];
      // dir_flag is 1 for direction x, 2 for y and 4 for z
      int dir_flag = 1;
      // sub_cube_flags contains 2^dim binary flags (1 per sub-cube),
      // indicating which sub-cubes are crossed by the element
      int octree_flags = 255;
      // in how many sub-octrees is this element stored?
      int_t nb_duplicates = 1;

      for (int direction = 0; direction < 3; direction++)
        {
          const int_t elem_min = elements_boxes(elem, direction);
          const int_t elem_max = elements_boxes(elem, box_delta+direction);
          assert(elem_max >= elem_min);
          // coordinate of the cube center in direction j:
          const int center = (direction==0) ? octree_center_x : ((direction==1) ? octree_center_y : octree_center_z);
          // Does the element cut both the lower and upper parts of the cube in "direction"?
          if (elem_min >= center) // no -> remove flags of cubes in the lower part
            octree_flags &= sub_cube_flags_max[direction];
          else if (elem_max < center) // no -> remove flags of cubes in the upper part
            octree_flags &= sub_cube_flags_min[direction];
          else
            nb_duplicates <<= 1; // the element crosses both parts!
          dir_flag = dir_flag << 1;
          if (dir_flag == nb_octrees)
            break;
        }
      elem_flags[i_elem] = octree_flags;
      nb_duplicate_elements += nb_duplicates - 1;
    }

  // Slightly complex criterion: if there are really many elements
  //  in this octree, we allow duplicating all elements,
  //  which handles very elongated elements that are necessarily
  //  duplicated in one direction (>OCTREE_DUPLICATE_ELEMENTS_LIMIT).
  if ((nb_duplicate_elements * 2 >= nb_elems && nb_elems < OCTREE_DUPLICATE_ELEMENTS_LIMIT)
      || nb_duplicate_elements > nb_elems)
    {
      const int_t the_octree_id = build_octree_floor(elements_list);
      // Return an octreefloor index
      return the_octree_id;
    }

  // Reserve a slot at the end of octree_structure to store this octree:
  const int_t index_octree = octree_structure_.dimension(0);
  octree_structure_.resize_dim0(index_octree + 1, RESIZE_OPTIONS::COPY_NOINIT);
  ArrOfInt_t& new_liste_elems = vect_elements_list[level+1];
  new_liste_elems.resize_array(0);
  const int width = octree_half_width >> 1;
  const int m_width = - width;
  // Recursive processing of the sub-cubes of the octree:
  for (int i_cube = 0; i_cube < nb_octrees; i_cube++)
    {
      const int octree_flag = 1 << i_cube;
      new_liste_elems.resize_array(nb_elems, RESIZE_OPTIONS::NOCOPY_NOINIT);
      int_t count = 0;
      // List of elements included in the sub-cube:
      for (int_t i_elem = 0; i_elem < nb_elems; i_elem++)
        if ((elem_flags[i_elem] & octree_flag) != 0)
          new_liste_elems[count++] = elements_list[i_elem];
      new_liste_elems.resize_array(count);

      int_t sub_octree_id;
      if (new_liste_elems.size_array() == 0)
        sub_octree_id = octree_id(-1, EMPTY);
      else
        {
          // Coordinates of the new sub-cube
          const int cx = octree_center_x + ((i_cube&1) ? width : m_width);
          const int cy = octree_center_y + ((i_cube&2) ? width : m_width);
          const int cz = octree_center_z + ((i_cube&4) ? width : m_width);
          sub_octree_id = build_octree_recursively(cx, cy, cz, width,
                                                   elements_boxes,
                                                   vect_elements_list,
                                                   level+1,
                                                   tmp_elem_flags);
        }
      octree_structure_(index_octree, i_cube) = sub_octree_id;
    }

  return octree_id(index_octree, OCTREE);
}

/*! @brief returns the octree_id of the octree_floor containing the vertex (x,y,z) (may return the EMPTY octree)
 *
 */
template <typename _SIZE_>
typename Octree_Int_32_64<_SIZE_>::int_t Octree_Int_32_64<_SIZE_>::search_octree_floor(int x, int y, int z) const
{
  if (octree_type(root_octree_id_) != OCTREE)
    return root_octree_id_;
  // The test to determine whether we are in the upper or lower part of an octree at level i
  // simply consists of testing the i-th bit of the position.
  int flag = root_octree_half_width_;

  int_t index = octree_index(root_octree_id_, OCTREE);

  // Descend the hierarchy of subdivided octrees down to the smallest cube
  while (1)
    {
      // Index of the sub-cube containing the vertex x,y,z
      const int ix = (x & flag) ? 1 : 0;
      const int iy = (y & flag) ? 2 : 0;
      const int iz = (z & flag) ? 4 : 0;
      int i_sous_cube = ix + iy + iz;
      // Enter the sub-cube:
      const int_t the_octree_id = octree_structure_(index, i_sous_cube);
      if (octree_type(the_octree_id) != OCTREE) // floor or empty
        return the_octree_id;

      index = octree_index(the_octree_id, OCTREE);
      flag >>= 1;
    }
  //return -1; // We never reach here!
}



template class Octree_Int_32_64<int>;
#if INT_is_64_ == 2
template class Octree_Int_32_64<trustIdType>;
#endif
