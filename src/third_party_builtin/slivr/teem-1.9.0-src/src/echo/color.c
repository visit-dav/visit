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

#include "echo.h"
#include "privateEcho.h"

char _echoBuff[128] = "";

char *
_echoDot(int depth) {
  int i;

  _echoBuff[0] = '\0';
  for (i=1; i<=depth; i++) {
    strcat(_echoBuff, ".  ");
  }
  return _echoBuff;
}

void
echoTextureLookup(echoCol_t rgba[4], Nrrd *ntext,
                  echoPos_t u, echoPos_t v, echoRTParm *parm) {
  int su, sv, ui, vi;
  float uf, vf;
  unsigned char *tdata00, *tdata10, *tdata01, *tdata11;

  su = ntext->axis[1].size;
  sv = ntext->axis[2].size;
  if (parm->textureNN) {
    ui = airIndex(0.0, u, 1.0, su);
    vi = airIndex(0.0, v, 1.0, sv);
    tdata00 = (unsigned char*)(ntext->data) + 4*(ui + su*vi);
    ELL_4V_SET_TT(rgba, echoCol_t,
                  tdata00[0]/255.0, tdata00[1]/255.0,
                  tdata00[2]/255.0, tdata00[3]/255.0);
  } else {
    u = AIR_AFFINE(0.0, u, 1.0, 0.0, su-1);  u = AIR_CLAMP(0, u, su-1);
    v = AIR_AFFINE(0.0, v, 1.0, 0.0, sv-1);  v = AIR_CLAMP(0, v, sv-1);
    u -= (u == su-1);  ui = (int)u;  uf = AIR_CAST(float, u - ui);
    v -= (v == sv-1);  vi = (int)v;  vf = AIR_CAST(float, v - vi);
    tdata00 = (unsigned char*)(ntext->data) + 4*(ui + su*vi);
    tdata01 = tdata00 + 4;
    tdata10 = tdata00 + 4*su;
    tdata11 = tdata10 + 4;
    ELL_4V_SET_TT(rgba, echoCol_t,
                  ((1-vf)*(1-uf)*tdata00[0] + (1-vf)*uf*tdata01[0] +
                   vf*(1-uf)*tdata10[0] + vf*uf*tdata11[0])/255.0,
                  ((1-vf)*(1-uf)*tdata00[1] + (1-vf)*uf*tdata01[1] +
                   vf*(1-uf)*tdata10[1] + vf*uf*tdata11[1])/255.0,
                  ((1-vf)*(1-uf)*tdata00[2] + (1-vf)*uf*tdata01[2] +
                   vf*(1-uf)*tdata10[2] + vf*uf*tdata11[2])/255.0,
                  ((1-vf)*(1-uf)*tdata00[3] + (1-vf)*uf*tdata01[3] +
                   vf*(1-uf)*tdata10[3] + vf*uf*tdata11[3])/255.0);
  }
}

void
echoIntxMaterialColor(echoCol_t rgba[4], echoIntx *intx, echoRTParm *parm) {

  if (intx->obj->ntext) {
    _echoRayIntxUV[intx->obj->type](intx);
    echoTextureLookup(rgba, intx->obj->ntext, intx->u, intx->v, parm);
    rgba[0] *= intx->obj->rgba[0];
    rgba[1] *= intx->obj->rgba[1];
    rgba[2] *= intx->obj->rgba[2];
    rgba[3] *= intx->obj->rgba[3];
  } else {
    ELL_4V_COPY(rgba, intx->obj->rgba);
  }
}

/*
******** echoIntxLightColor
**
** determines ambient, diffuse, and (Phong) specular contributions to lighting
** a given intersection.  "ambi" and "diff" MUST be given as non-NULL,
** "spec" can be given as NULL in order to bypass specular computations
*/
void
echoIntxLightColor(echoCol_t ambi[3], echoCol_t diff[3], echoCol_t spec[3],
                   echoCol_t sp,
                   echoIntx *intx, echoScene *scene, echoRTParm *parm, 
                   echoThreadState *tstate) {
  unsigned int Lidx;
  int blocked;
  echoRay shadRay;
  echoIntx shadIntx;
  echoPos_t Ldist, Ldir[3], Lpos[3], Ldot;
  echoCol_t Lcol[3], fracseen;

  if (parm->shadow) {
    /* from, neer, shadow */
    shadRay.shadow = AIR_TRUE;
    ELL_3V_COPY(shadRay.from, intx->pos);
    shadRay.neer = ECHO_EPSILON;
  }
  
  /* set ambient (easy) */
  ELL_3V_COPY(ambi, scene->ambi);
  
  /* environment contributes only to diffuse */
  if (scene->envmap) {
    echoEnvmapLookup(diff, intx->norm, scene->envmap);
  } else {
    ELL_3V_SET(diff, 0, 0, 0);
  }

  /* lights contribute to diffuse and specular */
  if (spec) {
    ELL_3V_SET(spec, 0, 0, 0);
  }
  for (Lidx=0; Lidx<scene->lightArr->len; Lidx++) {
    echoLightPosition(Lpos, scene->light[Lidx], tstate);
    ELL_3V_SUB(Ldir, Lpos, intx->pos);
    ELL_3V_NORM(Ldir, Ldir, Ldist);
    Ldot = ELL_3V_DOT(Ldir, intx->norm);
    /* HEY: HACK: we have to general per-object-type flag that says,
       this kind of object has no notion of in-versus-out facing ... */
    if (echoTypeRectangle == intx->obj->type) {
      Ldot = AIR_ABS(Ldot);
    }
    if (Ldot <= 0) {
      continue;
      /* to next light, we aren't facing this one.  NB: this means
         that there aren't diffuse or specular contributions on the
         backsides of even semi-transparent surfaces */
    }
    if (parm->shadow) {
      ELL_3V_COPY(shadRay.dir, Ldir);
      shadRay.faar = Ldist;
      if (echoRayIntx(&shadIntx, &shadRay, scene, parm, tstate)) {
        if (1.0 == parm->shadow) {
          /* skip to next light, this one is obscured by something,
             and we don't do any partial shadowing */
          continue;
        }
        blocked = AIR_TRUE;
      } else {
        blocked = AIR_FALSE;
      }
    } else {
      blocked = AIR_FALSE;
    }
    fracseen = AIR_CAST(echoCol_t, blocked ? 1.0 - parm->shadow : 1.0);
    echoLightColor(Lcol, Ldist, scene->light[Lidx], parm, tstate);
    ELL_3V_SCALE_INCR_TT(diff, echoCol_t, fracseen*Ldot, Lcol);
    if (spec) {
      Ldot = ELL_3V_DOT(Ldir, intx->refl);
      if (echoTypeRectangle == intx->obj->type) {
        Ldot = AIR_ABS(Ldot);
      }
      if (Ldot > 0) {
        Ldot = pow(Ldot, sp);
        ELL_3V_SCALE_INCR_TT(spec, echoCol_t, fracseen*Ldot, Lcol);
      }
    }
  }
  return;
}

void
_echoIntxColorPhong(INTXCOLOR_ARGS) {
  echoCol_t ambi[3], diff[3], spec[3], ka, kd, ks, sp;

  ka = intx->obj->mat[echoMatterPhongKa];
  kd = intx->obj->mat[echoMatterPhongKd];
  ks = intx->obj->mat[echoMatterPhongKs];
  sp = intx->obj->mat[echoMatterPhongSp];

  echoIntxMaterialColor(rgba, intx, parm);
  ELL_3V_SET(spec, 0, 0, 0);
  echoIntxLightColor(ambi, diff, ks ? spec : NULL, sp,
                     intx, scene, parm, tstate);
  rgba[0] = rgba[0]*(ka*ambi[0] + kd*diff[0]) + ks*spec[0];
  rgba[1] = rgba[1]*(ka*ambi[1] + kd*diff[1]) + ks*spec[1];
  rgba[2] = rgba[2]*(ka*ambi[2] + kd*diff[2]) + ks*spec[2];
  return;
}

void
_echoIntxColorMetal(INTXCOLOR_ARGS) {
  echoCol_t ka, kd, kp, RA, RD, RS, ambi[3], diff[3], spec[4];
  echoPos_t c;
  echoRay reflRay;

  if (0 && tstate->verbose) {
    fprintf(stderr, "%s%s: t = %g\n",
            _echoDot(tstate->depth), "_echoIntxColorMetal", intx->t);
  }

  ELL_3V_SET(spec, 0, 0, 0);
  echoIntxMaterialColor(rgba, intx, parm);
  c = ELL_3V_DOT(intx->view, intx->norm);
  if (c <= 0) {
    /* see only surface color on backside of metal */
    return;
  }
  c = 1 - c;
  c = c*c*c*c*c;
  RS = intx->obj->mat[echoMatterMetalR0];
  RS = AIR_CAST(echoCol_t, RS + (1 - RS)*c);
  ka = intx->obj->mat[echoMatterMetalKa];
  kd = intx->obj->mat[echoMatterMetalKd];
  kp = ka + kd;
  /* neer, faar, shadow, from, dir */
  ELL_3V_COPY(reflRay.from, intx->pos);
  ELL_3V_COPY(reflRay.dir, intx->refl);
  reflRay.neer = ECHO_EPSILON;
  reflRay.faar = ECHO_POS_MAX;
  reflRay.shadow = AIR_FALSE;
  echoRayColor(spec, &reflRay, scene, parm, tstate);
  if (kp) {
    RA = (1 - RS)*ka/kp;
    RD = (1 - RS)*kd/kp;
    echoIntxLightColor(ambi, diff, NULL, 0.0,
                       intx, scene, parm, tstate);
    /* NB: surface color does attenuate reflected color (unlike phong) */
    rgba[0] *= RA*ambi[0] + RD*diff[0] + RS*spec[0];
    rgba[1] *= RA*ambi[1] + RD*diff[1] + RS*spec[1];
    rgba[2] *= RA*ambi[2] + RD*diff[2] + RS*spec[2];
  } else {
    rgba[0] *= RS*spec[0];
    rgba[1] *= RS*spec[1];
    rgba[2] *= RS*spec[2];
  }

  return;
}

/*
** "th" = theta = angle of incidence
** "ph" = phi = angle of refraction
** "index" = (index of outgoing material)/(index of incoming material)
*/
int
_echoRefract(echoPos_t T[3], echoPos_t V[3],
             echoPos_t N[3], echoCol_t index, echoThreadState *tstate) {
  echoPos_t cosTh, cosPh, sinPhSq, cosPhSq, tmp1, tmp2;

  cosTh = ELL_3V_DOT(V, N);
  sinPhSq = (1 - cosTh*cosTh)/(index*index);
  cosPhSq = 1 - sinPhSq;
  if (cosPhSq < 0) {
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: cosTh = %g --%g--> TIR!!\n",
              _echoDot(tstate->depth), "_echoRefract",
              cosTh, index);
    }
    return AIR_FALSE;
  }
  /* else we do not have total internal reflection */
  cosPh = sqrt(cosPhSq);
  if (tstate->verbose) {
    fprintf(stderr, "%s%s: cosTh = %g --%g--> cosPh = %g\n",
            _echoDot(tstate->depth), "_echoRefract",
            cosTh, index, cosPh);
  }
  tmp1 = -1.0/index; tmp2 = cosTh/index - cosPh; 
  ELL_3V_SCALE_ADD2(T, tmp1, V, tmp2, N);
  ELL_3V_NORM(T, T, tmp1);
  return AIR_TRUE;
}

void
_echoIntxColorGlass(INTXCOLOR_ARGS) {
  char me[]="_echoIntxColorGlass";
  echoCol_t
    ambi[3], diff[3],
    ka, kd, RP, RS, RT, R0,
    index,         /* (index of material we're going into) /
                      (index of material we're leaving) */
    k[3],          /* attenuation of color due to travel through medium */
    matlCol[4],    /* inherent color */
    reflCol[4],    /* color from reflected ray */
    tranCol[4];    /* color from transmitted ray */
  echoPos_t tmp,
    negnorm[3];
  double c;
  echoRay tranRay, reflRay;

  echoIntxMaterialColor(matlCol, intx, parm);

  /* from, neer, faar, shadow */
  ELL_3V_COPY(tranRay.from, intx->pos);
  ELL_3V_COPY(reflRay.from, intx->pos);
  tranRay.neer = reflRay.neer = ECHO_EPSILON;
  tranRay.faar = reflRay.faar = ECHO_POS_MAX;
  tranRay.shadow = reflRay.shadow = AIR_FALSE;
  ELL_3V_COPY(reflRay.dir, intx->refl);
  /* tranRay.dir set below */
  index = intx->obj->mat[echoMatterGlassIndex];

  RS = 0.0;  /* this is a flag meaning: "AFAIK, there's no total int refl" */
  tmp = ELL_3V_DOT(intx->norm, intx->view);
  if (tmp > 0) {
    /* "d.n < 0": we're coming from outside the glass, and we
       assume this means that we're going into a HIGHER index material,
       which means there is NO total internal reflection */
    _echoRefract(tranRay.dir, intx->view, intx->norm, index, tstate);
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: V=(%g,%g,%g),N=(%g,%g,%g),n=%g -> T=(%g,%g,%g)\n",
              _echoDot(tstate->depth), me,
              intx->view[0], intx->view[1], intx->view[2],
              intx->norm[0], intx->norm[1], intx->norm[2], index,
              tranRay.dir[0], tranRay.dir[1], tranRay.dir[2]);
    }
    c = tmp;
    ELL_3V_SET(k, 1, 1, 1);
  } else {
    /* we're coming from inside the glass */
    /* the reasoning for my Beer's law implementation is this: if a
       channel (r, g, or b) is full on (1.0), then there should be no
       attenuation in its color.  The more the color is below 1.0, the
       more it should be damped with distance. */
    k[0] = AIR_CAST(echoCol_t, exp(parm->glassC*(matlCol[0]-1)*intx->t));
    k[1] = AIR_CAST(echoCol_t, exp(parm->glassC*(matlCol[1]-1)*intx->t));
    k[2] = AIR_CAST(echoCol_t, exp(parm->glassC*(matlCol[2]-1)*intx->t));
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: internal refl @ t = %g -> k = %g %g %g\n",
              _echoDot(tstate->depth), me, intx->t, k[0], k[1], k[2]);
    }
    ELL_3V_SCALE(negnorm, -1, intx->norm);
    if (_echoRefract(tranRay.dir, intx->view, negnorm, 1/index, tstate)) {
      c = -ELL_3V_DOT(tranRay.dir, negnorm);
    } else {
      /* its total internal reflection time! */
      c = 0.0;
      RS = 1.0;
    }
  }

  if (RS) {
    /* total internal reflection */
    RT = 0;
  } else {
    R0 = (index - 1)/(index + 1);
    R0 *= R0;
    c = 1 - c;
    c = c*c*c*c*c;
    RS = AIR_CAST(echoCol_t, R0 + (1-R0)*c);
    RT = 1 - RS;
  }
  ka = intx->obj->mat[echoMatterMetalKa];
  kd = intx->obj->mat[echoMatterMetalKd];
  RP = ka + kd;
  if (RP) {
    RS *= 1 - RP;
    RT *= 1 - RP;
    echoIntxLightColor(ambi, diff, NULL, 0.0,
                       intx, scene, parm, tstate);
  } else {
    ELL_3V_SET(ambi, 0, 0, 0);
    ELL_3V_SET(diff, 0, 0, 0);
  }
  if (tstate->verbose) {
    fprintf(stderr, "%s%s: --- reflRay (reflected)\n",
            _echoDot(tstate->depth), me);
  }
  echoRayColor(reflCol, &reflRay, scene, parm, tstate);
  if (RT) {
    if (tstate->verbose) {
      fprintf(stderr, "%s%s: --- tranRay (refracted)\n",
              _echoDot(tstate->depth), me);
    }
    echoRayColor(tranCol, &tranRay, scene, parm, tstate);
  } else {
    ELL_3V_SET(tranCol, 0, 0, 0);
  }
  rgba[0] = (matlCol[0]*(ka*ambi[0] + kd*diff[0])  +
             k[0]*(RS*reflCol[0] + RT*tranCol[0]));
  rgba[1] = (matlCol[1]*(ka*ambi[1] + kd*diff[1])  +
             k[1]*(RS*reflCol[1] + RT*tranCol[1]));
  rgba[2] = (matlCol[2]*(ka*ambi[2] + kd*diff[2])  +
             k[2]*(RS*reflCol[2] + RT*tranCol[2]));
  rgba[3] = 1.0;
  return;
}

void
_echoIntxColorLight(INTXCOLOR_ARGS) {

  AIR_UNUSED(scene);
  AIR_UNUSED(tstate);
  echoIntxMaterialColor(rgba, intx, parm);
}

void
_echoIntxColorUnknown(INTXCOLOR_ARGS) {
  
  AIR_UNUSED(rgba);
  AIR_UNUSED(intx);
  AIR_UNUSED(scene);
  AIR_UNUSED(parm);
  fprintf(stderr, "%s%s: can't color intx with object with unset material\n",
          _echoDot(tstate->depth), "_echoIntxColorNone");
}

_echoIntxColor_t
_echoIntxColor[ECHO_MATTER_MAX+1] = {
  _echoIntxColorUnknown,
  _echoIntxColorPhong,
  _echoIntxColorGlass,
  _echoIntxColorMetal,
  _echoIntxColorLight,
};

/*
******** echoIntxFuzzify()
**
** this modifies the refl and norm fields of a given intx
*/
void
echoIntxFuzzify(echoIntx *intx, echoCol_t fuzz, echoThreadState *tstate) {
  echoPos_t tmp, *jitt, oldNorm[3], perp0[3], perp1[3], j0, j1;
  int side;

  /* at some point I thought this was important to avoid bias when
     going through glass, but now I'm not so sure ... It is likely
     totally moot if jitter vectors are NOT reused between pixels. */
  if (ELL_3V_DOT(intx->norm, intx->view) > 0) {
    jitt = tstate->jitt + 2*echoJittableNormalA;
  } else {
    jitt = tstate->jitt + 2*echoJittableNormalB;
  }
  j0 = fuzz*jitt[0];
  j1 = fuzz*jitt[1];
  ELL_3V_COPY(oldNorm, intx->norm);
  side = ELL_3V_DOT(intx->refl, oldNorm) > 0;
  ell_3v_PERP(perp0, oldNorm);
  ELL_3V_NORM(perp0, perp0, tmp);
  ELL_3V_CROSS(perp1, perp0, oldNorm);
  ELL_3V_SCALE_ADD3(intx->norm, 1, oldNorm, j0, perp0, j1, perp1);
  ELL_3V_NORM(intx->norm, intx->norm, tmp);
  _ECHO_REFLECT(intx->refl, intx->norm, intx->view, tmp);
  if (side != (ELL_3V_DOT(intx->refl, oldNorm) > 0)) {
    ELL_3V_SCALE_ADD3(intx->norm, 1, oldNorm, -j0, perp0, -j1, perp1);
    ELL_3V_NORM(intx->norm, intx->norm, tmp);
    _ECHO_REFLECT(intx->refl, intx->norm, intx->view, tmp);
  }
  if (tstate->verbose) {
    fprintf(stderr, "%s%s: fuzz[%g](%g,%g,%g) --> (%g,%g,%g)\n",
            _echoDot(tstate->depth), "echoIntxFuzzify", fuzz,
            oldNorm[0], oldNorm[1], oldNorm[2],
            intx->norm[0], intx->norm[1], intx->norm[2]);
  }
  return;
}

void
echoIntxColor(echoCol_t rgba[4], echoIntx *intx,
              echoScene *scene, echoRTParm *parm, echoThreadState *tstate) {
  echoCol_t fuzz;
  
  switch(intx->obj->matter) {
  case echoMatterGlass:
    fuzz = intx->obj->mat[echoMatterGlassFuzzy];
    break;
  case echoMatterMetal:
    fuzz = intx->obj->mat[echoMatterMetalFuzzy];
    break;
  default:
    fuzz = 0;
    break;
  }
  if (fuzz) {
    echoIntxFuzzify(intx, fuzz, tstate);
  }
  _echoIntxColor[intx->obj->matter](rgba, intx, scene, parm, tstate);
}

