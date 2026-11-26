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

#ifndef MD_Vector_seq_included
#define MD_Vector_seq_included

#include <MD_Vector_mono.h>
#include <TRUSTArray.h>

/*! @brief Dummy parallel descriptor used for sequential computations.
 *
 * Its only useful information is the total number of items hold by the array which can be big (hence the use of trustIdType).
 *
 * In this class, all the inherited counters accessors (get_nb_items*() methods, etc.) return this number. We however carefully check
 * that the number of items fit within 32b since thoses inherited accessors return 'int'. No piece of code is actually using those
 * methods in the 64b part of the execution (i.e. before Scatter).
 *
 * Historically we used a MD_Vector_std for sequential computations too, and this std MD_Vector had a single block covering
 * the whole array. But having a template version of the MD_Vector_std class (supporting both 32b and 64b) seemed unreasonable,
 * hence the creation of the current class.
 *
 * See arch.h.in for more explanations on 64b.
 *
 * In this class, members inherited from MD_Vector_mono are meaningless and their information is not updated.
 * Algorithm like get_sequential_items_flags() are actually overriden not to use this.
 */
class MD_Vector_seq : public MD_Vector_mono
{
  Declare_instanciable_sans_constructeur(MD_Vector_seq);

public:

  MD_Vector_seq() : MD_Vector_seq(-1) {  }
  MD_Vector_seq(const trustIdType& nb) : MD_Vector_mono(), nb_items_(nb)
  {
    // We should never create a MD_Vector_seq when parallel ... but unfortunately we do in Scatter :-(
    //    assert(Process::is_sequential());
  }

  trustIdType get_nb_items() const { return nb_items_; }

  // A sequential descriptor is never actually used. It is always OK, hence its validation is always true:
  bool validate(trustIdType sz_array, int line_size) const override { return true; }

  // For all those methods, return nb_items_, but check before it is smaller than 64b!
  // They should never be called from the 64b part of TRUST.
  int get_nb_items_reels() const override;
  int get_nb_items_tot() const override;
  trustIdType nb_items_seq_tot() const override { return nb_items_; }
  int nb_items_seq_local() const override;

  // All these should never be called in sequential:
  void initialize_comm(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, DoubleVect&) const override   { throw; }
  void prepare_send_data(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, DoubleVect&) const override { throw; }
  void process_recv_data(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, DoubleVect&) const override { throw; }

  void initialize_comm(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, FloatVect&) const override   { throw; }
  void prepare_send_data(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, FloatVect&) const override { throw; }
  void process_recv_data(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, FloatVect&) const override { throw; }

  void initialize_comm(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, IntVect&) const override   { throw; }
  void prepare_send_data(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, IntVect&) const override { throw; }
  void process_recv_data(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, IntVect&) const override { throw; }

#if INT_is_64_ == 2
  void initialize_comm(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, TIDVect&) const override   { throw; }
  void prepare_send_data(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, TIDVect&) const override { throw; }
  void process_recv_data(const Echange_EV_Options& opt, Schema_Comm_Vecteurs&, TIDVect&) const override { throw; }
#endif

  void append_from_other_seq(const MD_Vector_seq& src, int offset, int multiplier) override;
  void fill_md_vect_renum(const IntVect& renum, MD_Vector& md_vect) const override;

  bool use_blocks() const override { return false; }

protected:
  int get_seq_flags_impl(ArrOfBit& flags, int line_size) const override;

private:
  /*! The number of items in the array - all real, no virtuals */
  trustIdType nb_items_ = -1;
};
#endif
