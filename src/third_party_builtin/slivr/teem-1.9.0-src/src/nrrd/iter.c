/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "nrrd.h"
#include "privateNrrd.h"

NrrdIter *
nrrdIterNew() {
  NrrdIter *iter;

  if ( (iter = (NrrdIter *)calloc(1, sizeof(NrrdIter))) ) {
    iter->nrrd = NULL;
    iter->ownNrrd = NULL;
    iter->val = AIR_NAN;
    iter->size = 0;
    iter->data = NULL;
    iter->left = 0;
    iter->load = NULL;
  }
  return iter;
}

void
nrrdIterSetValue(NrrdIter *iter, double val) {

  if (iter) {
    iter->nrrd = NULL;
    iter->ownNrrd = iter->ownNrrd ? nrrdNuke(iter->ownNrrd) : NULL;
    iter->val = val;
    iter->size = nrrdTypeSize[nrrdTypeDouble];
    iter->data = (char*)&(iter->val);
    iter->left = 0;
    iter->load = nrrdDLoad[nrrdTypeDouble];
  }
  return;
}

void
nrrdIterSetNrrd(NrrdIter *iter, const Nrrd *nrrd) {

  if (iter && nrrd && nrrd->data) {
    if (nrrdTypeBlock == nrrd->type) {
      /* we can't deal */
      nrrdIterSetValue(iter, AIR_NAN);
      return;
    }
    iter->nrrd = nrrd;
    iter->ownNrrd = iter->ownNrrd ? nrrdNuke(iter->ownNrrd) : NULL;
    iter->val = AIR_NAN;
    iter->size = nrrdTypeSize[nrrd->type];
    iter->data = (char *)nrrd->data;
    iter->left = nrrdElementNumber(nrrd)-1;
    iter->load = nrrdDLoad[nrrd->type];
  }
  return;
}

/*
** formerly known as nrrdIterSetNrrd
*/
void
nrrdIterSetOwnNrrd(NrrdIter *iter, Nrrd *nrrd) {

  if (iter && nrrd && nrrd->data) {
    if (nrrdTypeBlock == nrrd->type) {
      /* we can't deal */
      nrrdIterSetValue(iter, AIR_NAN);
      return;
    }
    iter->nrrd = NULL;
    iter->ownNrrd = iter->ownNrrd ? nrrdNuke(iter->ownNrrd) : NULL;
    iter->ownNrrd = nrrd;
    iter->val = AIR_NAN;
    iter->size = nrrdTypeSize[nrrd->type];
    iter->data = (char *)nrrd->data;
    iter->left = nrrdElementNumber(nrrd)-1;
    iter->load = nrrdDLoad[nrrd->type];
  }
  return;
}

double
nrrdIterValue(NrrdIter *iter) {
  double ret = 0.0;

  if (iter) {
    ret = iter->load(iter->data);
    if (iter->nrrd || iter->ownNrrd) {
      iter->data += iter->size;
      if (iter->left) {
        iter->left -= 1;
      } else {
        iter->data = (char *)(_NRRD_ITER_NRRD(iter)->data);
        iter->left = nrrdElementNumber(_NRRD_ITER_NRRD(iter))-1;
      }
    }
  }
  return ret;
}

/*
******** nrrdIterContent()
**
** ALLOCATES a string that is either the nrrd's content (or 
** nrrdStateUnknownContent) or a string version of the value; useful
** for when you's use the "content" of a nrrd
*/
char *
nrrdIterContent(NrrdIter *iter) {
  char *ret, buff[AIR_STRLEN_SMALL];

  ret = NULL;
  if (iter) {
    if (_NRRD_ITER_NRRD(iter)) {
      ret = _nrrdContentGet(_NRRD_ITER_NRRD(iter));
    } else {
      airSinglePrintf(NULL, buff, "%g", iter->val);
      ret = airStrdup(buff);
    }
  }  
  return ret;
}

NrrdIter *
nrrdIterNix(NrrdIter *iter) {

  if (iter) {
    if (iter->ownNrrd) {
      iter->ownNrrd = nrrdNuke(iter->ownNrrd);
    }
    free(iter);
  }
  return NULL;
}

