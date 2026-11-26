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

#ifndef TRUSTList_TPP_included
#define TRUSTList_TPP_included

/*! @brief Ecriture d'une liste sur un flot de sortie les elements separes par des virgules figurent entre des accolades
 *
 */
template<typename _TYPE_>
inline Sortie& TRUSTList<_TYPE_>::printOn(Sortie& os) const
{
  TRUSTList_Curseur<_TYPE_> curseur(*this);
  Nom accouverte = "{", accfermee = "}", virgule = ",";
  os << accouverte << " ";
  while (curseur)
    {
      // if(est_dernier()) GF sinon on a une virgule de trop
      if (curseur.list().est_dernier()) os << curseur.valeur() << " ";
      else os << curseur.valeur() << " " << virgule << " ";
      ++curseur;
    }
  os << accfermee;
  return os;
}

/*! @brief Lecture d'une liste sur un flot d'entree les elements separes par des virgules figurent entre des accolades
 *
 */
template<typename _TYPE_>
inline Entree& TRUSTList<_TYPE_>::readOn(Entree& is)
{
  Nom accouverte = "{", accfermee = "}", virgule = ",", nom;
  _TYPE_ t;
  is >> nom;
  assert(nom == accouverte);
  while (nom != accfermee)
    {
      is >> t;
      add(t);
      is >> nom;
      assert((nom == accfermee) || (nom == virgule));
    }
  return is;
}

/*! @brief Affectation.
 *
 * Les elements sont copies
 *
 */
template<typename _TYPE_>
inline TRUSTList<_TYPE_>& TRUSTList<_TYPE_>::operator=(const TRUSTList<_TYPE_>& a_list)
{
  if (a_list.est_vide()) this->suivant_ = this;
  else
    {
      vide();
      TRUSTList_Curseur<_TYPE_> curseur(a_list);
      while (curseur)
        {
          add(curseur.valeur());
          ++curseur;
        }
    }
  return *this;
}

/*! @brief insertion en queue
 *
 */
template<typename _TYPE_>
inline TRUSTList<_TYPE_>& TRUSTList<_TYPE_>::add(_TYPE_ value_to_add)
{
  if (value_to_add < min_data) min_data = value_to_add;
  if (value_to_add > max_data) max_data = value_to_add;
  size_++;
  if (TRUSTListElem<_TYPE_>::est_vide())
    {
      this->data = value_to_add;
      this->suivant_ = 0;
      return *this;
    }
  else
    {
      if (TRUSTListElem<_TYPE_>::est_dernier())
        {
          TRUSTListElem<_TYPE_> *next = new TRUSTListElem<_TYPE_>(value_to_add);
          this->suivant_ = next;
          dernier_ = next;
        }
      else
        {
          dernier().add(value_to_add);
          dernier_ = &dernier_->suivant();
        }
      return *this;
    }
}

/*! @brief Ajout d'un element a la liste ssi il n'existe pas deja
 *
 */
template<typename _TYPE_>
inline TRUSTList<_TYPE_>& TRUSTList<_TYPE_>::add_if_not(_TYPE_ x)
{
  if (!contient(x)) return add(x);
  else return *this;
}

/*! @brief Verifie si un element appartient ou non a la liste
 *
 */
template<typename _TYPE_>
inline int TRUSTList<_TYPE_>::contient(_TYPE_ x) const
{
  if (TRUSTListElem<_TYPE_>::est_vide() || x > max_data || x < min_data) return 0;

  TRUSTList_Curseur<_TYPE_> curseur(*this);
  while (curseur)
    {
      if (curseur.valeur() == x) return 1;
      ++curseur;
    }
  return 0;
}

/*! @brief renvoie le rang d'un element dans la liste si un element apparait plusieurs fois, renvoie le rang du premier.
 *
 */
template<typename _TYPE_>
inline int TRUSTList<_TYPE_>::rang(_TYPE_ x) const
{
  if (TRUSTListElem<_TYPE_>::est_vide() || x > max_data || x < min_data) return -1;

  int compteur = 0;
  TRUSTList_Curseur<_TYPE_> curseur(*this);
  while (curseur)
    {
      if (curseur.valeur() == x) return compteur;
      ++compteur;
      ++curseur;
    }
  return -1;
}

/*! @brief Operateur d'acces au ieme int de la liste
 *
 */
template<typename _TYPE_>
inline _TYPE_& TRUSTList<_TYPE_>::operator[](int i)
{
  TRUSTList_Curseur<_TYPE_> curseur(*this);
  while (curseur && i--) ++curseur;

  if (i != -1)
    {
      Process::exit("Overflow list ");
    }
  return curseur.valeur();
}

template<typename _TYPE_>
inline const _TYPE_& TRUSTList<_TYPE_>::operator[](int i) const
{
  TRUSTList_Curseur<_TYPE_> curseur(*this);
  while (curseur && i--) ++curseur;

  if (i != -1)
    {
      Process::exit("Overflow list ");
    }
  return curseur.valeur();
}

/*! @brief Operateur de comparaison de deux listes
 *
 */
template<typename _TYPE_>
int operator ==(const TRUSTList<_TYPE_>& list1, const TRUSTList<_TYPE_>& list2)
{
  int retour = 1;
  if (list1.data != list2.data) retour = 0;
  if ((!list1.est_dernier()) && (list2.est_dernier())) retour = 0;
  if ((list1.est_dernier()) && (!list2.est_dernier())) retour = 0;
  if ((!list1.est_dernier()) && (!list2.est_dernier())) retour = (*list1.suivant_ == *list2.suivant_);
  return retour;
}

/*! @brief Supprime un element contenu dans la liste
 *
 */
template<typename _TYPE_>
inline void TRUSTList<_TYPE_>::suppr(_TYPE_ obj)
{
  if (TRUSTListElem<_TYPE_>::valeur() == obj)
    {
      if (this->suivant_)
        {
          TRUSTListElem<_TYPE_> *next = this->suivant_;
          this->suivant_ = next->suivant_;
          this->data = next->valeur();
          next->suivant_ = 0;
          delete next;
        }
      else
        {
          this->suivant_ = this;
          dernier_ = this;
        }
      calcule_min_max();
      size_--;
      return;
    }
  TRUSTList_Curseur<_TYPE_> curseur_pre = *this;
  TRUSTList_Curseur<_TYPE_> curseur = *(this->suivant_);

  while (curseur)
    {
      if (curseur.valeur() == obj)
        {
          TRUSTListElem<_TYPE_> *next = &curseur_pre.list().suivant();
          curseur_pre.list().suivant_ = curseur.list().suivant_;
          if (next->suivant_ == 0) dernier_ = &curseur_pre.list();
          else next->suivant_ = 0;

          delete next;
          calcule_min_max();
          size_--;
          return;
        }
      ++curseur;
      ++curseur_pre;
    }
  Cerr << "WARNING during deletion of an element in a list " << finl;
  Cerr << "One has not found object == : " << obj << finl;
}

template<typename _TYPE_>
inline void TRUSTList<_TYPE_>::calcule_min_max()
{
  min_data = std::numeric_limits<_TYPE_>::max();
  max_data = std::numeric_limits<_TYPE_>::lowest();
  TRUSTList_Curseur<_TYPE_> curseur = *this;
  while (curseur)
    {
      _TYPE_ la_valeur = curseur.valeur();
      if (la_valeur < min_data) min_data = la_valeur;
      if (la_valeur > max_data) max_data = la_valeur;
      ++curseur;
    }
}

/*! @brief Vide la liste
 *
 */
template<typename _TYPE_>
inline void TRUSTList<_TYPE_>::vide()
{
  if (!TRUSTListElem<_TYPE_>::est_vide())
    if (this->suivant_) delete this->suivant_;
  this->suivant_ = this;
  dernier_ = this;
  calcule_min_max();
  size_=0;
}

#endif /* TRUSTList_TPP_included */
