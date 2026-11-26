/****************************************************************************
* Copyright (c) 2025, CEA
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

#ifndef TRUSTTab_parts_TPP_included
#define TRUSTTab_parts_TPP_included

template<typename _TYPE_>
inline void init_parts(TRUSTVect<_TYPE_>& vect, TRUST_Vector<TRUSTTab<_TYPE_>>& parts, TRUSTTab<_TYPE_> *dummy_type_ptr)
{
  const MD_Vector& md = vect.get_md_vector();
  TRUSTTab<_TYPE_>* tab_ptr = dynamic_cast<TRUSTTab<_TYPE_>*>(&vect);
  if (! md.non_nul() || !sub_type(MD_Vector_composite, md.valeur()))
    {
      // Ce n'est pas un tableau a plusieurs sous-parties, on cree juste une partie qui pointe sur vect.
      parts.dimensionner(1);
      TRUSTTab<_TYPE_>& part = parts[0];
      // On preserve le 'shape' du tableau de depart, si c'est un TRUSTTab<_TYPE_>...
      if (tab_ptr) part.ref(*tab_ptr);
      else part.ref(vect);
    }
  else
    {
      ArrOfInt shape;
      const int line_size = vect.line_size();
      if (tab_ptr)
        {
          const TRUSTTab<_TYPE_>& tab = *tab_ptr;
          const int n = tab.nb_dim();
          shape.resize_array(n);
          for (int i = 0; i < n; i++) shape[i] = tab.dimension_tot(i);
        }
      else
        {
          if (line_size == 1)
            {
              shape.resize_array(1);
              shape[0] = vect.size_totale();
            }
          else
            {
              shape.resize_array(2);
              shape[0] = vect.size_totale() / line_size;
              shape[1] = line_size;
            }
        }

      const MD_Vector_composite& mdata = ref_cast(MD_Vector_composite, md.valeur());
      const int n = mdata.nb_parts();
      parts.dimensionner(n);
      for (int i = 0, j; i < n; i++)
        {
          ArrOfInt shape_i;
          if (mdata.get_shape(i) == 0)
            shape_i = shape; //si mdata.shapes_[i] > 0, alors la sous-partie a une dimension mineure en plus
          else
            {
              shape_i.resize(shape.size_array() + 1);
              shape_i[1] = mdata.get_shape(i);
              for (j = 1; j < shape.size_array(); j++)
                shape_i[j + 1] = shape[j];
            }
          const int offset = mdata.get_part_offset(i);
          const MD_Vector& md_part = mdata.get_desc_part(i);
          shape_i[0] = md_part->get_nb_items_tot();
          TRUSTTab<_TYPE_>& part = parts[i];
          // Fait pointer le domaine de memoire sur le sous-tableau (pour l'instant tableau monodimensionnel)
          part.ref_array(vect, offset * line_size, shape_i[0] * line_size * std::max(mdata.get_shape(i), (int)1)); // int cast necessary for 64b transcription
          // Change le "shape" du tableau pour mettre le nombre de lignes et de colonnes
          // (nombre total d'items inchange, donc resize autorise)
          part.resize(shape_i);
          // Associe le md_vector
          part.set_md_vector(md_part);
        }
    }
}

template <typename _TYPE_>
TRUSTTab_parts<_TYPE_>::TRUSTTab_parts(TRUSTVect<_TYPE_>& vect)
{
  init_parts<_TYPE_>(vect, parts_, static_cast<TRUSTTab<_TYPE_>*>(0));
}

template <typename _TYPE_>
void TRUSTTab_parts<_TYPE_>::initialize(TRUSTVect<_TYPE_>& vect)
{
  reset();
  init_parts<_TYPE_>(vect, parts_, static_cast<TRUSTTab<_TYPE_>*>(0));
}

// Pour ConstTRUSTTab_parts, on const_cast pour remplir les ref dans parts_.
// Pas grave, ensuite les sous parties ne seront accessibles qu'en const de l'exterieur.
template <typename _TYPE_>
ConstTRUSTTab_parts<_TYPE_>::ConstTRUSTTab_parts(const TRUSTVect<_TYPE_>& vect)
{
  TRUSTVect<_TYPE_>& vect2 = const_cast<TRUSTVect<_TYPE_>&>(vect);
  init_parts<_TYPE_>(vect2, parts_, static_cast<TRUSTTab<_TYPE_>*>(0));
}

template <typename _TYPE_>
void ConstTRUSTTab_parts<_TYPE_>::initialize(const TRUSTVect<_TYPE_>& vect)
{
  reset();
  TRUSTVect<_TYPE_>& vect2 = const_cast<TRUSTVect<_TYPE_>&>(vect);
  init_parts<_TYPE_>(vect2, parts_, static_cast<TRUSTTab<_TYPE_>*>(0));
}

#endif /* TRUSTTab_parts_TPP_included */
