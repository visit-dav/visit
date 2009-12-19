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

#ifndef PUSH_PRIVATE_HAS_BEEN_INCLUDED
#define PUSH_PRIVATE_HAS_BEEN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* defaultsPush.c */
extern int _pushVerbose;

/* methodsPush.c */
extern push_t *_pushThingPos(pushThing *thg);

/* forces.c */
extern airEnum *pushForceEnum;

/* binning.c */
extern pushBin *_pushBinLocate(pushContext *pctx, push_t *pos);
extern int _pushBinPointNullify(pushContext *pctx,
                                pushBin *oldBin, pushPoint *point);
extern void _pushBinPointAdd(pushContext *pctx,
                             pushBin *bin, pushPoint *point);

/* corePush.c */
extern pushTask *_pushTaskNew(pushContext *pctx, int threadIdx);
extern int _pushProcessDummy(pushTask *task, int bin,
                              const push_t *parm);

/* setup.c */
extern pushTask *_pushTaskNew(pushContext *pctx, int threadIdx);
extern pushTask *_pushTaskNix(pushTask *task);
extern int _pushTensorFieldSetup(pushContext *pctx);
extern int _pushGageSetup(pushContext *pctx);
extern int _pushFiberSetup(pushContext *pctx);
extern int _pushTaskSetup(pushContext *pctx);
extern int _pushBinSetup(pushContext *pctx);
extern int _pushThingSetup(pushContext *pctx);

/* action.c */
extern push_t _pushThingPointCharge(pushContext *pctx, pushThing *thg);
extern int _pushForceSample(pushContext *pctx,
                            unsigned int sx, unsigned int sy);
void _pushTenInv(pushContext *pctx, push_t *inv, push_t *ten);
extern void _pushProbe(pushTask *task, pushPoint *point);
extern int _pushInputProcess(pushContext *pctx);
extern void _pushInitialize(pushContext *pctx);
extern int _pushForce(pushTask *task, int bin, const push_t *parm);
extern int _pushUpdate(pushTask *task, int bin, const push_t *parm);

#ifdef __cplusplus
}
#endif

#endif /* PUSH_PRIVATE_HAS_BEEN_INCLUDED */
