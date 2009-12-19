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


#include <teem/biff.h>
#include <teem/nrrd.h>

int
main(int argc, char *argv[]) {
  char *me, *err;
  int enc, form;

  AIR_UNUSED(argc);
  me = argv[0];
  if (!nrrdSanity()) {
    printf("%s: nrrd sanity check FAILED:\n%s\n", me, err = biffGet(NRRD));
    free(err);
    return 1;
  }
  else {
    printf("%s: nrrd sanity check passed.\n", me);
    printf("\n");
    printf("%s: encodings supported in this build:\n", me);
    for (enc=nrrdEncodingTypeUnknown+1; enc<nrrdEncodingTypeLast; enc++) {
      printf("%s: %s\n", airEnumStr(nrrdEncodingType, enc),
             nrrdEncodingArray[enc]->available() ? "yes" : "not available");
    }
    printf("%s: formats supported in this build:\n", me);
    for (form=nrrdFormatTypeUnknown+1; form<nrrdFormatTypeLast; form++) {
      printf("%s: %s\n", airEnumStr(nrrdFormatType, form),
             nrrdFormatArray[form]->available() ? "yes" : "not available");
    }
  }

  return 0;
}
