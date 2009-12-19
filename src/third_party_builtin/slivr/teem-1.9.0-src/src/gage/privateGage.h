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

#ifndef GAGE_PRIVATE_HAS_BEEN_INCLUDED
#define GAGE_PRIVATE_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define GT gage_t

#if GAGE_TYPE_FLOAT
#  define ell_3v_PRINT ell_3v_print_f
#  define ell_3m_PRINT ell_3m_print_f
#  define ell_3v_PERP ell_3v_perp_f
#  define nrrdLOOKUP nrrdFLookup
#  define EVALN evalN_f               /* NrrdKernel method */
#else
#  define ell_3v_PRINT ell_3v_print_d
#  define ell_3m_PRINT ell_3m_print_d
#  define ell_3v_PERP ell_3v_perp_d
#  define nrrdLOOKUP nrrdDLookup
#  define EVALN evalN_d               /* NrrdKernel method */
#endif

/* shape.c */
extern int _gageShapeSet(const gageContext *ctx, gageShape *shape,
                         const Nrrd *nin, unsigned int baseDim);

/* pvl.c */
extern gagePerVolume *_gagePerVolumeCopy(gagePerVolume *pvl, int fd);
extern gage_t *_gageAnswerPointer(const gageContext *ctx, 
                                  gagePerVolume *pvl, int item);

/* print.c */
extern void _gagePrint_off(FILE *, gageContext *ctx);
extern void _gagePrint_fslw(FILE *, gageContext *ctx);

/* filter.c */
extern int _gageLocationSet(gageContext *ctx, gage_t x, gage_t y, gage_t z);

/* sclprint.c */
extern void _gageSclIv3Print(FILE *, gageContext *ctx, gagePerVolume *pvl);

/* sclfilter.c */
extern void _gageSclFilter(gageContext *ctx, gagePerVolume *pvl);

/* sclanswer.c */
extern void _gageSclAnswer(gageContext *ctx, gagePerVolume *pvl);

/* vecprint.c */
extern void _gageVecIv3Print(FILE *, gageContext *ctx, gagePerVolume *pvl);

#ifdef __cplusplus
}
#endif

#endif /* GAGE_PRIVATE_HAS_BEEN_INCLUDED */
