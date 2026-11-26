/****************************************************************************
* Copyright (c) 2022, CEA
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

#ifndef Entree_Fichier_base_included
#define Entree_Fichier_base_included


#include <Entree.h>
#include <Objet_U.h>
#include <fstream>
using std::ifstream;
using std::ofstream;
using std::streampos;

/*! @brief File for reading. This class is to the C++ ifstream class what the Entree class is to the
 *
 *     C++ istream class. It virtually redefines the read operators for a file.
 *
 */

class Entree_Fichier_base : public Entree, public Objet_U
{
  Declare_base_sans_constructeur(Entree_Fichier_base);
public:
  Entree_Fichier_base();
  Entree_Fichier_base(const Entree_Fichier_base&) = default;
  Entree_Fichier_base(const char* name,IOS_OPEN_MODE mode=ios::in);
  ifstream& get_ifstream();
  void close();
  int eof() override;
  int fail() override;
  int good() override;
  void precision(int pre);
  void setf(IOS_FORMAT code);

  virtual int ouvrir(const char* name, IOS_OPEN_MODE mode=ios::in);

  static bool Can_be_read(const char * name);

protected:
  ifstream* ifstream_;

private:
  Entree_Fichier_base& operator=(const Entree_Fichier_base&);  // copy is forbidden
};

#endif
