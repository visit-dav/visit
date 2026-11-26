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

#ifndef TRUSTArr_device_TPP_included
#define TRUSTArr_device_TPP_included

#include <TRUSTArray.h>

// Methodes de verification que le tableau est a jour sur le host:
// ToDo OpenMP :Appels couteux (car non inlines?) depuis operator()[int] mais comment faire mieux ?
template<typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_,_SIZE_>::ensureDataOnHost() const
{
#if defined(TRUST_USE_GPU) && !defined(TRUST_USE_UVM)
  if (get_data_location()==DataLocation::Device)
    copyFromDevice(*this);
#endif
}

template<typename _TYPE_, typename _SIZE_>
inline void TRUSTArray<_TYPE_,_SIZE_>::ensureDataOnHost()
{
#if defined(TRUST_USE_GPU) && !defined(TRUST_USE_UVM)
  const DataLocation& loc = get_data_location();
  if (loc==DataLocation::Host || loc==DataLocation::HostOnly) return;
  else if (loc==DataLocation::Device)
    copyFromDevice(*this);
  // On va modifier le tableau (non const) sur le host:
  set_data_location(DataLocation::Host);
#endif
}

// Fonction pour connaitre la localisation du tableau
template<typename _TYPE_, typename _SIZE_>
inline bool TRUSTArray<_TYPE_,_SIZE_>::isDataOnDevice() const
{
  DataLocation loc = get_data_location();
  return loc == DataLocation::Device || loc == DataLocation::HostDevice;
}

// Fonctions checkDataOnDevice pour lancement conditionnel de kernels sur le device:
// -Si les tableaux passes en parametre sont sur a jour sur le device
// -Si ce n'est pas le cas, les tableaux sont copies sur le host via ensureDataOnHost
template<typename _TYPE_, typename _SIZE_>
inline bool TRUSTArray<_TYPE_,_SIZE_>::checkDataOnDevice() const
{
#ifdef TRUST_USE_GPU
  bool flag = isDataOnDevice();
  if (!flag)
    ensureDataOnHost();
  //else
  //  set_data_location(Device); // non const array will be computed on device
  return flag;
#else
  return false;
#endif
}

template<typename _TYPE_, typename _SIZE_>
inline bool TRUSTArray<_TYPE_,_SIZE_>::checkDataOnDevice()
{
#ifdef TRUST_USE_GPU
  bool flag = isDataOnDevice();
  if (!flag)
    ensureDataOnHost();
  else
    set_data_location(DataLocation::Device); // non const array will be computed on device
  return flag;
#else
  return false;
#endif
}

template<typename _TYPE_, typename _SIZE_>
inline bool TRUSTArray<_TYPE_,_SIZE_>::checkDataOnDevice(const TRUSTArray& tab_const)
{
#ifdef TRUST_USE_GPU
  bool flag = isDataOnDevice() && tab_const.isDataOnDevice();
  // Si un des deux tableaux n'est pas a jour sur le device alors l'operation se fera sur le host:
  if (!flag)
    {
      this->ensureDataOnHost();
      tab_const.ensureDataOnHost();
    }
  else
    set_data_location(DataLocation::Device); // non const array will be computed on device
  return flag;
#else
  return false;
#endif
}


#endif
