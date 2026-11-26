/****************************************************************************
* Copyright (c) 2023, CEA
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

#ifndef Motcle_included
#define Motcle_included

#include <TRUST_Vector.h>
#include <Nom.h>

/*! @brief A character string (Nom) in uppercase.
 *
 */
class Motcle : public Nom
{
  Declare_instanciable_sans_constructeur(Motcle);

public:

  Motcle();
  Motcle(const Motcle& nom);
  Motcle& operator=(const Motcle& );

  Motcle(const char * const nom);
  Motcle(const std::string& nom);
  Motcle(const Nom&);
  Motcle& operator=(const char * const);
  Motcle& operator=(const Nom&);
  Motcle& operator+=(const char * const);
  int selftest();

  int debute_par(const char* const n) const override;
  int finit_par(const char* const n) const override;
  int find(const char* const n) const override;

  friend int operator ==(const Motcle& , const Motcle& ) ;
  friend int operator !=(const Motcle& , const Motcle& ) ;
  friend int operator ==(const Motcle& , const char* const ) ;
  friend int operator !=(const Motcle& , const char* const ) ;
  friend int operator ==(const char* const , const Motcle& ) ;
  friend int operator !=(const char* const , const Motcle& ) ;
  friend int operator ==(const Motcle& , const Nom& ) ;
  friend int operator !=(const Motcle& , const Nom& ) ;
  friend int operator ==(const Nom& , const Motcle& ) ;
  friend int operator !=(const Nom& , const Motcle& ) ;
};

/*! @brief An array of Motcle objects.
 *
 */
class Motcles : public VECT(Motcle)
{
  Declare_instanciable(Motcles);
public:
  Motcles(int);
  int rang(const char* const ch) const;
  int contient_(const char* const ch) const;
  int search(const Motcle& t ) const ;
};

#endif
// MOTCLE_H
