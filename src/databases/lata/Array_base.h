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

#ifndef Array_base_included
#define Array_base_included

#include <Objet_U.h>
class MD_Vector;

/*! Resize options:
*  RESIZE_OPTIONS::NOCOPY_NOINIT: do not copy old values, do not initialize new ones
*  RESIZE_OPTIONS::COPY_NOINIT: copy old values, do not initialize new ones
*  RESIZE_OPTIONS::COPY_INIT: copy old values and initialize new ones to zero
*/
enum class RESIZE_OPTIONS { NOCOPY_NOINIT, COPY_NOINIT, COPY_INIT };

/*! Storage options */
enum class STORAGE { STANDARD, TEMP_STORAGE };

/*! GPU Data location */
enum class DataLocation { HostOnly, Host, Device, HostDevice };

/*! @brief Empty class used as a base for all the arrays.
 *
 * Allows to pass a generic type, whatever the real derived nature of the array.
 * (e.g. Domaine::creer_tableau_sommets())
 */
class Array_base : public Objet_U
{
  Declare_base_sans_constructeur_ni_destructeur(Array_base);
};

#endif /* Array_base_included */
