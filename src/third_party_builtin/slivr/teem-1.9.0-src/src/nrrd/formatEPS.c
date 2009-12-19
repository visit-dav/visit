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

int
_nrrdFormatEPS_available(void) {

  /* but only for writing ... */
  return AIR_TRUE;
}

int
_nrrdFormatEPS_nameLooksLike(const char *filename) {
  
  return airEndsWith(filename, NRRD_EXT_EPS);
}

int
_nrrdFormatEPS_fitsInto(const Nrrd *nrrd, const NrrdEncoding *encoding,
                        int useBiff) {
  char me[]="_nrrdFormatEPS_fitsInto", err[BIFF_STRLEN];
  int ret;

  AIR_UNUSED(encoding);
  /* encoding information is ignored- its always going to be hex */
  if (!nrrd) {
    sprintf(err, "%s: got NULL nrrd (%p)", me, AIR_CAST(void*, nrrd));
    biffMaybeAdd(NRRD, err, useBiff); 
    return AIR_FALSE;
  }
  if (nrrdTypeUChar != nrrd->type) {
    sprintf(err, "%s: type must be %s (not %s)", me,
            airEnumStr(nrrdType, nrrdTypeUChar),
            airEnumStr(nrrdType, nrrd->type));
    biffMaybeAdd(NRRD, err, useBiff); 
    return AIR_FALSE;
  }
  if (2 == nrrd->dim) {
    /* its a gray-scale image */
    ret = 2;
  } else if (3 == nrrd->dim) {
    if (1 == nrrd->axis[0].size) {
      /* its a faux-3D image, really grayscale */
      ret = 2;
    } else if (3 == nrrd->axis[0].size) {
      /* its a real color image */
      ret = 3;
    } else {
      /* else its no good */
      sprintf(err, "%s: dim is 3, but 1st axis size is " _AIR_SIZE_T_CNV
              ", not 1 or 3", me, nrrd->axis[0].size);
      biffMaybeAdd(NRRD, err, useBiff); 
      return AIR_FALSE;
    }
  } else {
    sprintf(err, "%s: dimension is %d, not 2 or 3", me, nrrd->dim);
    biffMaybeAdd(NRRD, err, useBiff); 
    return AIR_FALSE;
  }
  return ret;
}

int
_nrrdFormatEPS_contentStartsLike(NrrdIoState *nio) {

  AIR_UNUSED(nio);
  /* this is a write-only format */
  return AIR_FALSE;
}

int
_nrrdFormatEPS_read(FILE *file, Nrrd *nrrd, NrrdIoState *nio) {
  char me[]="_nrrdFormatEPS_read", err[BIFF_STRLEN];

  AIR_UNUSED(file);
  AIR_UNUSED(nrrd);
  AIR_UNUSED(nio);
  sprintf(err, "%s: sorry, this is a write-only format", me);
  biffAdd(NRRD, err);
  return 1;
}

int
_nrrdFormatEPS_write(FILE *file, const Nrrd *_nrrd, NrrdIoState *nio) {
  char me[]="_nrrdFormatEPS_write", err[BIFF_STRLEN];
  int color, sx, sy;
  Nrrd *nrrd;
  double aspect, minX, minY, maxX, maxY, scale;
  airArray *mop;
  
  mop = airMopNew();
  airMopAdd(mop, nrrd = nrrdNew(), (airMopper)nrrdNuke, airMopAlways);
  if (nrrdCopy(nrrd, _nrrd)) {
    sprintf(err, "%s: couldn't make private copy", me);
    biffAdd(NRRD, err); airMopError(mop); return 1;
  }
  if (3 == nrrd->dim && 1 == nrrd->axis[0].size) {
    if (nrrdAxesDelete(nrrd, nrrd, 0)) {
      sprintf(err, "%s:", me);
      biffAdd(NRRD, err); airMopError(mop); return 1;
    }
  }
  color = (3 == nrrd->dim);
  if (color) {
    sx = nrrd->axis[1].size;
    sy = nrrd->axis[2].size;
  } else {
    sx = nrrd->axis[0].size;
    sy = nrrd->axis[1].size;
  }
  aspect = AIR_CAST(double, sx)/sy;
  if (aspect > 7.5/10) {
    /* image has a wider aspect ratio than safely printable page area */
    minX = 0.5;
    maxX = 8.0;
    minY = 5.50 - 7.5*sy/sx/2;
    maxY = 5.50 + 7.5*sy/sx/2;
    scale = 7.5/sx;
  } else {
    /* image is taller ... */
    minX = 4.25 - 10.0*sx/sy/2;
    maxX = 4.25 + 10.0*sx/sy/2;
    minY = 0.5;
    maxY = 10.5;
    scale = 10.0/sy;
  }
  minX *= 72; minY *= 72;
  maxX *= 72; maxY *= 72;
  scale *= 72;

  fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(file, "%%%%Creator: Nrrd Utilities From the "
          "Great Nation of Deseret\n");
  fprintf(file, "%%%%Title: %s\n", 
          nrrd->content ? nrrd->content : "A lovely little image");
  fprintf(file, "%%%%Pages: 1\n");
  fprintf(file, "%%%%BoundingBox: %d %d %d %d\n",
          (int)floor(minX), (int)floor(minY),
          (int)ceil(maxX), (int)ceil(maxY));
  fprintf(file, "%%%%HiResBoundingBox: %g %g %g %g\n", 
          minX, minY, maxX, maxY);
  fprintf(file, "%%%%EndComments\n");
  fprintf(file, "%%%%BeginProlog\n");
  fprintf(file, "%% linestr creates an empty string to hold "
          "one scanline\n");
  fprintf(file, "/linestr %d string def\n", sx*(color ? 3 : 1));
  fprintf(file, "%%%%EndProlog\n");
  fprintf(file, "%%%%Page: 1 1\n");
  fprintf(file, "gsave\n");
  fprintf(file, "%g %g moveto\n", minX, minY);
  fprintf(file, "%g %g lineto\n", maxX, minY);
  fprintf(file, "%g %g lineto\n", maxX, maxY);
  fprintf(file, "%g %g lineto\n", minX, maxY);
  fprintf(file, "closepath\n");
  fprintf(file, "clip\n");
  fprintf(file, "gsave newpath\n");
  fprintf(file, "%g %g translate\n", minX, minY);
  fprintf(file, "%g %g scale\n", sx*scale, sy*scale);
  fprintf(file, "%d %d 8\n", sx, sy);
  fprintf(file, "[%d 0 0 -%d 0 %d]\n", sx, sy, sy);
  fprintf(file, "{currentfile linestr readhexstring pop} %s\n",
          color ? "false 3 colorimage" : "image");
  nrrdEncodingHex->write(file, nrrd->data, nrrdElementNumber(nrrd),
                         nrrd, nio);
  fprintf(file, "\n");
  fprintf(file, "grestore\n");
  fprintf(file, "grestore\n");
  
  airMopError(mop); 
  return 0;
}

const NrrdFormat
_nrrdFormatEPS = {
  "EPS",
  AIR_FALSE,  /* isImage */
  AIR_FALSE,  /* readable */
  AIR_FALSE,  /* usesDIO */
  _nrrdFormatEPS_available,
  _nrrdFormatEPS_nameLooksLike,
  _nrrdFormatEPS_fitsInto,
  _nrrdFormatEPS_contentStartsLike,
  _nrrdFormatEPS_read,
  _nrrdFormatEPS_write
};

const NrrdFormat *const
nrrdFormatEPS = &_nrrdFormatEPS;
