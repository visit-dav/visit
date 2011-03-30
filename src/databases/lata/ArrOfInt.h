/*****************************************************************************
*
* Copyright (c) 2011, CEA
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of CEA, nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

 
#ifndef ArrOfInt_H
#define ArrOfInt_H

#include <assert.h>
#include <arch.h>
#include <Objet_U.h>


class VIntdata;

class ArrOfInt
{
 public:
  //
  // Destructeur
  // 
  virtual ~ArrOfInt();
  //
  // Constructeurs
  //   
  ArrOfInt();
  ArrOfInt(entier size);
  ArrOfInt(entier size, entier initial_value);
  // Constructeur par copie : deep copy (on duplique les donnees)
  ArrOfInt(const ArrOfInt& );
  //
  // Methodes de construction tardive (on cree un tableau vide avec ArrOfInt()
  // puis on appelle ces methodes pour modifier les caracteristiques du tableau :
  //
  // Change le nombre d'elements du tableau
  inline ArrOfInt & resize_array(entier new_size);

  // Methodes de gestion de l'allocation memoire:
  // Assigne une valeur au drapeau "smart_resize" 
  // (reallocation uniquement si la taille augmente)
  void    set_smart_resize(entier flag);
  // Gestion du type de memoire alouee (standard ou pool de memoire Trio-U)
  enum    Storage { STANDARD, TEMP_STORAGE };
  void    set_mem_storage(const Storage storage);
  Storage get_mem_storage() const;

  // Construction de tableaux qui pointent vers des donnees existantes
  // !!! Utiliser ref_data avec precaution (attention a size_array_)
  ArrOfInt& ref_data(entier* ptr, entier size); 
  ArrOfInt& ref_array(const ArrOfInt&);
  // Operateur copie
  ArrOfInt & operator=(const ArrOfInt &);
  // Remise du tableau dans l'etat initial (obtenu par le constructeur par defaut)
  virtual void reset();

  //
  // Methodes d'acces aux donnees et aux caracteristiques du tableau
  // 
  // Remplit le tableau avec la valeur en parametre
  ArrOfInt & operator=(entier valeur);

  inline       entier & operator[](entier i);
  inline const entier & operator[](entier i) const;

  // Ces methodes renvoient un pointeur vers le premier element du tableau.
  const entier * addr() const;
        entier * addr();
  // Renvoie le nombre d'elements du tableau (et non la taille allouee)
  inline entier size_array() const;
  // Renvoie le nombre de tableaux qui pointent vers la stucture "*p_"
  entier ref_count() const;
  // Ajoute une case en fin de tableau et y stocke la "valeur"
  inline void   append_array(entier valeur);

  //
  // Operateurs divers
  //
  ArrOfInt& operator+=(const ArrOfInt&);
  ArrOfInt& operator+=(const entier);
  ArrOfInt& operator-=(const ArrOfInt&);
  ArrOfInt& operator-=(const entier);
  ArrOfInt& inject_array(const ArrOfInt & source,
                                   entier nb_elements = -1,
                                   entier first_element_dest = 0,
                                   entier first_element_source = 0);
  ArrOfInt& copy_array(const ArrOfInt&);


  void             ordonne_array();

 protected:
  //
  // Methodes accessibles depuis les descendants de ArrOfInt
  //
  void   attach_array(const ArrOfInt&);
  entier detach_array();
  void   fill_default_value(entier first, entier nb);
 private:
  // B. Mathieu 22/06/2004 : je mets ces membres "private" pour forcer
  // le passage par les accesseurs dans les classes derivees, au cas ou
  // on voudrait modifier l'implementation.

  // Zone de memoire contenant les valeurs du tableau.
  // Pointeur nul => le tableau est "detache" ou "ref_data"
  // Pointeur non nul => le tableau est "normal"
  VIntdata* p_;

  // Pointeur vers le premier element du tableau (egal a p_->data si p_!=0)
  // Pointeur nul => le tableau est "detache".
  // Pointeur non nul => le tableau est "normal" ou "ref_data"
  entier*   data_;

  // Nombre d'elements du tableau (inferieur ou egal a memory_size_).
  // Si le tableau est "detache", alors size_array_=0
  entier    size_array_;
  // Taille memoire reellement allouee pour le tableau
  // (pour le mecanisme smart_resize_). memory_size_ est nul
  // si le tableau est de type "ref_data". Sinon memory_size_
  // est egal a p_->size_.
  entier    memory_size_;

  // Drapeau indiquant si on applique une strategie d'allocation
  // preventive (la memoire alouee augmente d'un facteur constant
  // si la taille devient insuffisante).
  // Si smart_resize_ == 0, alors on a toujours p_->size_ == size
  entier    smart_resize_;

  // Drapeau indiquant si l'allocation memoire a lieu avec un new classique
  // ou dans le pool de memoire temporaire de Trio
  Storage   storage_type_;

  // Partie non inline de resize_array():
  // Declaration des constantes pour les options de memory_resize
  static const entier COPY_OLD;
  static const entier INITIALIZE_NEW;
  void memory_resize(entier new_size, entier options);
};

#define MAXDIMIntTab 2

class IntTab : public ArrOfInt
{
 public:
  IntTab();
  IntTab(const IntTab &);
  IntTab(const entier i, const entier j);
  IntTab &   operator=(const IntTab &);
  void              reset();
  
  inline entier & operator()(entier i, entier j);
  inline entier   operator()(entier i, entier j) const;

  inline entier resize(entier i, entier j);
  inline entier dimension(entier i) const;
  inline entier dimension_tot(entier i) const;

 protected:
  // In order to mimic Trio_U behavior, operator[] is forbidden
  // for 2 dimensionnal matrices, you must cast to ArrOf to use it..
  double &       operator[](entier i);
  const double & operator[](entier i) const;

 private:
  entier nb_dim_;
  entier dimensions_[MAXDIMIntTab];
};

inline entier & IntTab::operator()(entier i, entier j)
{
  assert(nb_dim_ == 2);
  assert(i >= 0 && i < dimensions_[0] && j >= 0 && j < dimensions_[1]);
  const entier n = i * dimensions_[1] + j;
  entier & x = ArrOfInt::operator[] (n);
  return x;
}

inline entier   IntTab::operator()(entier i, entier j) const 
{
  assert(nb_dim_ == 2);
  assert(i >= 0 && i < dimensions_[0] && j >= 0 && j < dimensions_[1]);
  const entier n = i * dimensions_[1] + j;
  entier x = ArrOfInt::operator[] (n);
  return x;
}

inline entier IntTab::resize(entier i, entier j)
{
  assert(i >= 0 && j >= 0);
  nb_dim_ = 2;
  dimensions_[0] = i;
  dimensions_[1] = j;
  ArrOfInt::resize_array(i * j);
  return i*j;
}

inline entier IntTab::dimension(entier i) const
{
  assert(i >= 0 && i < nb_dim_);
  return dimensions_[i];
}

// Description: renvoie la meme valeur que dimension.
inline entier IntTab::dimension_tot(entier i) const
{
  return dimension(i);
}

//
// Declarations des fonctions non membres de la classe ArrOfInt
//
entier operator==(const ArrOfInt& x, const ArrOfInt& y) ;
entier imin_array(const ArrOfInt&) ;
entier imax_array(const ArrOfInt&) ;
entier min_array(const ArrOfInt&) ;
entier max_array(const ArrOfInt&) ;


// ******************************************************************
//                   FONCTIONS MEMBRES DE ArrOfInt
// ******************************************************************

// Description :
//  Change le nombre d'elements du tableau. Cette fonction est inline
//  car elle doit etre tres rapide dans le cas ou smart_resize_==1
//  (utilisation frequente de resize_array())
//  Si smart_resize est non nul :
//   Si la nouvelle taille est inferieure ou egale a la taille
//   alouee (p->get_size()) on ne realloue pas de memoire
//   sinon, on realloue et on copie les donnees existantes.
//  Astuce pour ne pas copier les anciennes donnees:
//   resize(0); resize(n);
//  Si smart_resize est nul, on realloue une nouvelle zone memoire
//   uniquement si la nouvelle taille est differente de l'ancienne.
// Precondition :
//  Si "new_size" est egal a la taille du tableau, aucune condition.
//  Sinon, le tableau doit etre soit detache, soit normal (pas de type "ref_data")
//  et ref_count doit etre egal a 1 (pas d'autre reference au tableau).
//  
inline ArrOfInt & ArrOfInt::resize_array(entier new_size)
{
  assert(new_size >= 0);
  // Soit le tableau est detache (data_==0), soit il est normal (p_!=0)
  // S'il est normal, il ne faut pas qu'il y ait d'autre reference au tableau,
  // ou alors la taille ne doit pas changer. 
  assert(new_size == size_array_ || data_ == 0 || (p_ != 0 && ref_count() == 1));

  if ((smart_resize_ == 0) || (new_size > memory_size_))
    memory_resize(new_size, COPY_OLD + INITIALIZE_NEW);
  size_array_ = new_size;
  return *this;
}

// Description:
//     operateur [] retourne le ieme element du tableau
// Precondition:
// Parametre: entier i
//    Signification: indice dans l'intervalle 0 <= i < size_array()
// Exception:
//    assert si i n'est pas dans l'intervalle
inline entier& ArrOfInt::operator[](entier i)
{
  assert(i >= 0 && i < size_array_);
  return data_[i];
}

// Description:
//     operateur [] retourne le ieme element du tableau
// Precondition:
// Parametre: entier i
//    Signification: indice dans l'intervalle 0 <= i < size_array()
// Exception:
//    assert si i n'est pas dans l'intervalle
inline const entier& ArrOfInt::operator[](entier i) const
{
  assert(i >= 0 && i < size_array_);
  return data_[i];
}

// Description:
//    Renvoie la taille du tableau (nombre d'elements declares
//    a la construction ou a resize_array()).
//    C'est le nombre d'elements accessibles a operator[]
// Retour: entier
inline entier  ArrOfInt::size_array() const
{
  return size_array_;
}

// Description:
//  Ajoute une case en fin de tableau et y stocke la "valeur"
// Precondition:
//  Tableau doit etre de type "smart_resize" (sinon, ecroulement
//  des performances). De plus, le tableau ne doit pas etre "ref_data",
//  et il ne doit pas y avoir plus d'une reference a la zone de
//  memoire pointee (meme precondition que resize_array())
inline void   ArrOfInt::append_array(entier valeur)
{
  assert(smart_resize_);
  // Soit le tableau est detache (data_==0), soit il est normal (p_!=0)
  // S'il est normal, il ne faut pas qu'il y ait d'autre reference au tableau.
  assert(data_ == 0 || (p_ != 0 && ref_count() == 1));

  if (size_array_+1 > memory_size_)
    memory_resize(size_array_+1, COPY_OLD);
  data_[size_array_] = valeur;
  size_array_++;
}

// ArrOfInt_H
#endif
