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

#ifndef MD_Vector_mono_included
#define MD_Vector_mono_included

#include <MD_Vector_base.h>

/*! @brief Generic class for all mono-block MD_Vectors (i.e. non compoosite)
 *
 * The two main members of this class are blocs_items_to_sum_ and blocs_items_to_compute_.
 * Note that those two members are hence **not** present in a MD_Vector_composite. For this case
 * the redirection is made to the inner MD_Vector_mono member which aggregates the information.
 *
 * See also class MD_Vector_composite
 */
class MD_Vector_mono : public MD_Vector_base
{

  Declare_base(MD_Vector_mono);

public:
  const ArrOfInt& get_blocs_items_to_sum() const override { return blocs_items_to_sum_; }
  const ArrOfInt& get_items_to_sum() const override;
  const ArrOfInt& get_blocs_items_to_compute() const override { return blocs_items_to_compute_; }
  const ArrOfInt& get_items_to_compute() const override;

protected:
  // MD_Vector_composite needs to see inside MD_Vector_mono because of its global_md_ member:
  friend class MD_Vector_composite;

  // Methods to extend/complete a MD_Vector_mono with another MD_Vector_mono - not all combinations are possible.
  // See derived classes MD_Vector_seq and MD_Vector_std.
  // For example extending a MD_Vector_std with a MD_Vector_seq is not possible.
  virtual void append_from_other_std(const MD_Vector_std& src, int offset, int multiplier) { throw; }
  virtual void append_from_other_seq(const MD_Vector_seq& src, int offset, int multiplier) { throw; }

  // ***** The following members are used to compute sums, dot products, norms ******
  // Indices of all items owned by this processor (these are the "sequential items", defined as
  //  all items whose value is not received from another processor during an echange_espace_virtuel).
  //  To sum over all items, the values of all items in these blocks must be summed.
  //  The array contains start_bloc1, end_bloc1, start_bloc2, end_bloc2, etc...
  //  (end_bloc is the index of the last element + 1)
  //  (structure used for sequential saves (xyz or debog), vector norm computations, etc)
  ArrOfInt blocs_items_to_sum_;
  mutable ArrOfInt items_to_sum_; // All items (more suitable for TRUSTTab_tools kernels on GPU)
  // Indices of all items for which a value must be computed
  //  (used by DoubleTab::operator+=(const DoubleTab &) for example)
  // In theory, blocs_items_to_sum_ would suffice, but it is full of holes
  //  and can be inefficient. In practice, all real values are computed.
  ArrOfInt blocs_items_to_compute_;
  mutable ArrOfInt items_to_compute_; // All items (more suitable for TRUSTTab_tools kernels on GPU)
};
#endif
