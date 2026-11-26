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

#ifndef Connectivite_som_elem_H_INCLU
#define Connectivite_som_elem_H_INCLU

#include <TRUSTTabs_forward.h>

template <typename _SIZE_>
class Static_Int_Lists_32_64;

// We do not differentiate here _SIZE_ and _TYPE_ because those functions all work with entity indices.
// So we are either manipulating IntTab or BigTIDTab, nothing in between.
template <typename _SIZE_>
void construire_connectivite_som_elem(const _SIZE_ nb_sommets, const IntTab_T<_SIZE_>& les_elems, Static_Int_Lists_32_64<_SIZE_>& som_elem, bool include_virtual);

template <typename _SIZE_>
void find_adjacent_elements(const Static_Int_Lists_32_64<_SIZE_>& som_elem, const SmallArrOfTID_T<_SIZE_>& sommets_to_find, SmallArrOfTID_T<_SIZE_>& elements);

#endif
