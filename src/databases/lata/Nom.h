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



#ifndef Nom_included
#define Nom_included

#include <Objet_U.h>
#include <cstring>
#include <string>

/*! @brief class Nom: a character string for naming TRUST objects.
 *
 * @sa Motcle
 */

class Nom : public Objet_U
{
  Declare_instanciable_sans_constructeur_ni_destructeur(Nom);
public:
  Nom();                                         // constructs "??"
  Nom(int i);                                // Nom(485)="485"
  Nom(long i);                                    // Nom(485)="485"
  Nom(long long i);                                    // Nom(485)="485"
  Nom(const char* nom);
  Nom(const std::string& nom);
  Nom(const Nom&);
  Nom(char c);
  Nom(double );
  Nom(double le_reel, const char* format);
  ~Nom() override;

  operator const char*() const;

  Nom&   majuscule();
  int est_egal_a(const Objet_U&) const override;
  int longueur() const ;
  const Nom& le_nom() const override;

  Nom& operator=(const char* const);
  Nom& operator=(const Nom&);
  Nom operator +(const Nom&) const;
  Nom& operator +=(const Nom& x);
  Nom& operator +=(const char *);
  Nom& operator +=(char x);
  Nom& operator +=(unsigned char x);
  Nom& operator +=(int x);
  Nom& suffix(const char* const) ;
  const Nom getSuffix(const char* const) const ;
  Nom& prefix(const char* const) ;
  const Nom getPrefix(const char* const) const ;
  Nom nom_me(int, const char* prefix=0, int without_padding=0) const;
  Nom substr_old(const int, const int) const;
  Nom basename() const;

  virtual int find(const char* const n) const;
  virtual int debute_par(const char* const n) const;
  virtual int finit_par(const char* const n) const;

  int find(const std::string& n) const;
  int debute_par(const std::string&) const;
  int finit_par(const std::string&) const;

  friend int operator ==(const Nom& , const char* const) ;
  friend int operator !=(const Nom& , const char* ) ;
  friend int operator ==(const Nom& , const Nom&) ;
  friend int operator !=(const Nom& , const Nom& ) ;
  friend int operator ==(const char* const, const Nom&) ;
  friend int operator !=(const char* const, const Nom&) ;
  // so that Nom (and Motcle) can be used as a key in std::map :
  friend bool operator <(const Nom&, const Nom&) ;

  inline bool contient(const Nom& nom) const
  {
    std::size_t found = nom_.find(nom.nom_);
    return found!=std::string::npos;
  }
  inline const char* getChar() const  {  return nom_.c_str();  }
  inline const std::string& getString() const  {    return nom_;   }
  inline std::string& getString()  {  return nom_;   }

protected :

  std::string nom_;
};

#endif // NOM_H
