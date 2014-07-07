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

#include <Lata_tools.h>
#include <ArrOfInt.h>
#include <ArrOfDouble.h>
#include <ArrOfFloat.h>
#include <ArrOfBit.h>
#include <sstream>
#include <string.h>
#include <stdlib.h>

static int journal_level = 0;

void set_Journal_level(entier level)
{
  journal_level = level;
  Journal() << "Changed lata journal level: " << journal_level << endl;
}

static std::ostringstream junk_journal;

ostream & Journal(entier level)
{
  if (level <= journal_level) {
    cerr << "[" << level << "] ";
    return cerr;
  } else {
    junk_journal.seekp(0);
    return junk_journal;
  }
}

// Description: this method must return the total memory consumption
//  of the object (used to compute the size of the data cache)
BigEntier LataObject::compute_memory_size() const
{
  Journal() << "Error in LataObject::compute_memory_size(): function not implemented" << endl;
  throw;
}

BigEntier memory_size(const ArrOfInt & tab)
{
  // On ne tient pas compte du caractere smart_resize ou ref du tableau
  // c'est pas tres grave pour l'instant pour ce qu'on en fait...
  return ((BigEntier)sizeof(tab)) + ((BigEntier)tab.size_array()) * sizeof(entier);
}

BigEntier memory_size(const ArrOfDouble & tab)
{
  // on ne tient pas compte du caractere smart_resize ou ref du tableau
  // c'est pas tres grave pour l'instant pour ce qu'on en fait...
  return ((BigEntier)sizeof(tab)) + ((BigEntier)tab.size_array()) * sizeof(double);
}

BigEntier memory_size(const ArrOfFloat & tab)
{
  // on ne tient pas compte du caractere smart_resize ou ref du tableau
  // c'est pas tres grave pour l'instant pour ce qu'on en fait...
  return ((BigEntier)sizeof(tab)) + ((BigEntier)tab.size_array()) * sizeof(float);
}

BigEntier memory_size(const ArrOfBit & tab)
{
  return ((BigEntier)sizeof(tab)) + ((BigEntier)tab.size_array()) * sizeof(int) / 32; 
}

void split_path_filename(const char *s, Nom & path, Nom & filename)
{
  size_t i;
  for (i=strlen(s);i-- > 0;)
    if ((s[i]==PATH_SEPARATOR) || (s[i]=='\\'))
      break;
  path = "";
  size_t j;
  for (j = 0; j <= i; j++)
    path += Nom(s[j]);
  
  // Parse basename : if extension given, remove it
  filename = s+i+1;
}

static const ArrOfInt * array_to_sort_ptr = 0;
int compare_indirect(const void *ptr1, const void *ptr2)
{
  entier i1 = *(const entier*)ptr1;
  entier i2 = *(const entier*)ptr2;
  entier diff = (*array_to_sort_ptr)[i1] - (*array_to_sort_ptr)[i2];
  return (diff>0) ? 1 : ((diff==0) ? 0 : -1);
}

void array_sort_indirect(const ArrOfInt & array_to_sort, ArrOfInt & index)
{
  const entier n = array_to_sort.size_array();
  index.set_smart_resize(1);
  index.resize_array(n);
  for (entier i = 0; i < n; i++)
    index[i] = i;
  array_to_sort_ptr = &array_to_sort;
  qsort(index.addr(), n, sizeof(entier), compare_indirect);
}
