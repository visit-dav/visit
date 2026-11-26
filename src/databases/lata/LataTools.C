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

#include <LataTools.h>
#include <sstream>
#include <string.h>
#include <stdlib.h>

static int journal_level = 0;

void set_Journal_level(int level)
{
  if (journal_level==level) return;
  journal_level = level;
  Journal() << "Changed lata journal level: " << journal_level << endl;
}

static std::ostringstream junk_journal;

std::ostream& Journal(int level)
{
  if (level <= journal_level)
    {
      if (level != 0) cerr << "[" << level << "] ";
      return cerr;
    }
  else
    {
      junk_journal.seekp(0);
      return junk_journal;
    }
}

// Description: this method must return the total memory consumption
//  of the object (used to compute the size of the data cache)
Size_t LataObject::compute_memory_size() const
{
  Journal() << "Error in LataObject::compute_memory_size(): function not implemented" << endl;
  throw;
}

template<typename _TYPE_, typename _SIZE_>
Size_t memory_size(const TRUSTArray<_TYPE_,_SIZE_>& tab)
{
  return ((Size_t)sizeof(tab)) + ((Size_t)tab.size_array()) * sizeof(_TYPE_);
}

template Size_t memory_size<int,trustIdType>(const TRUSTArray<int,trustIdType>& tab);
template Size_t memory_size<float,int>(const TRUSTArray<float,int>& tab);
template Size_t memory_size<double,int>(const TRUSTArray<double,int>& tab);

#if INT_is_64_ == 2
template Size_t memory_size<trustIdType,trustIdType>(const TRUSTArray<trustIdType,trustIdType>& tab);
template Size_t memory_size<float,trustIdType>(const TRUSTArray<float,trustIdType>& tab);
template Size_t memory_size<double,trustIdType>(const TRUSTArray<double,trustIdType>& tab);
#endif


Size_t memory_size(const BigArrOfBit& tab)
{
  return ((Size_t)sizeof(tab)) + ((Size_t)tab.size_array()) * sizeof(int) / 32;
}

void split_path_filename(const char *s, Nom& path, Nom& filename)
{
  int i;
  for (i=(int)strlen(s)-1; i>=0; i--)
    if ((s[i]==PATH_SEPARATOR) || (s[i]=='\\'))
      break;
  path = "";
  int j;
  for (j = 0; j <= i; j++)
    path += Nom(s[j]);

  // Parse basename : if extension given, remove it
  filename = s+i+1;
}

void array_sort_indirect(const BigArrOfTID& array_to_sort, BigArrOfTID& index)
{
  const trustIdType n = array_to_sort.size_array();
  index.resize_array(n);
  for (trustIdType i = 0; i < n; i++)
    index[i] = i;
  std::sort(index.begin(), index.end(), [&](trustIdType a, trustIdType b)
  {
    return ( array_to_sort(a)<array_to_sort(b) );
  });
}
