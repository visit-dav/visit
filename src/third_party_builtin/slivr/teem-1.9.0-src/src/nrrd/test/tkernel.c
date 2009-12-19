/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
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

#include "../nrrd.h"

/*
** this program demonstrates parsing a string into a kernel with parms,
** verifies that all the kernel methods are consistent and/or equal, 
** and produces a text file of the kernel evaluated many times, which
** can be graphed in matlab with:
**
**   x = dlmread('out.txt', ' '); plot(x(:,1), x(:,2));
*/

void
usage(char *me) {
  /*                      0     1       2      3     4        5   (6) */
  fprintf(stderr, "usage: %s <kernel> <min> <step> <max> <txtOut>\n", me);
  exit(1);
}

#define CLOSE(a,b)      (fabs((a)-(b)) < 0.00001)
#define KINDACLOSE(a,b) (fabs((a)-(b)) < 0.0005)

int
main(int argc, char *argv[]) {
  char *me, *kS, *minS, *stepS, *maxS, *outS;
  const NrrdKernel *k;
  double parm[NRRD_KERNEL_PARMS_NUM], min, step, max, integral,
    *dom_d, *ran_d;
  float *dom_f, *ran_f, v, r_f, r_d;
  FILE *fout;
  int i, len;

  me = argv[0];
  if (6 != argc) {
    usage(me);
  }
  kS = argv[1];
  minS = argv[2];
  stepS = argv[3];
  maxS = argv[4];
  outS = argv[5];
  
  if (nrrdKernelParse(&k, parm, kS)) {
    fprintf(stderr, "%s: trouble:\n%s\n", me, biffGet(NRRD));
    exit(1);
  }
  if (3 != (sscanf(minS, "%lf", &min) +
            sscanf(stepS, "%lf", &step) +
            sscanf(maxS, "%lf", &max))) {
    fprintf(stderr, "%s: couldn't parse \"%s\", \"%s\", \"%s\" as 3 doubles\n",
            me, minS, stepS, maxS);
    exit(1);
  }
  if (!( min <= -k->support(parm) && max >= k->support(parm) )) {
    fprintf(stderr, "%s: WARNING: support=%g => lower min (%g) or raise max (%g)\n",
            me, k->support(parm), min, max);
  }

  /* see how many values are in the interval */
  len = 0;
  for (v=min; v<=max; v+=step) {
    len++;
  }
  /* allocate domain and range for both float and double */
  if (!( (dom_d = (double *)calloc(len, sizeof(double))) &&
         (ran_d = (double *)calloc(len, sizeof(double))) &&
         (dom_f = (float *)calloc(len, sizeof(float))) &&
         (ran_f = (float *)calloc(len, sizeof(float))) )) {
    fprintf(stderr, "%s: PANIC: couldn't allocate buffers\n", me);
    exit(1);
  }
  /* set values in both domains */
  i=0;
  for (v=min; v<=max; v+=step) {
    /* note that the value stored in dom_d[i] is only a 
       single-precision float, so that it is really equal to dom_f[i] */
    dom_d[i] = v;
    dom_f[i] = v;
    i++;
  }
  /* do the vector evaluations */
  k->evalN_f(ran_f, dom_f, len, parm);
  k->evalN_d(ran_d, dom_d, len, parm);
  /* do the single evaluations, and make sure everything agrees */
  i = 0;
  integral = 0;
  for (v=min; v<=max; v+=step) {
    /* compare two single evaluations */
    r_f = k->eval1_f(v, parm);
    r_d = k->eval1_d(v, parm);
    if (!CLOSE(r_f,r_d)) {
      fprintf(stderr, "%s: (eval1_f(%g)== %f) != (eval1_d(%g)== %f)\n",
              me, v, r_f, v, r_d);
    }
    /* compare single float with vector float */
    if (!CLOSE(r_f,ran_f[i])) {
      fprintf(stderr, "%s: (eval1_f(%g)== %f) != (evalN_f[%d]== %f)\n",
              me, v, r_f, i, ran_f[i]);
    }
    /* compare single float with vector double */
    r_d = ran_d[i];
    if (!CLOSE(r_f,r_d)) {
      fprintf(stderr, "%s: (eval1_f(%g)== %f) != (evalN_d[%d]== %f)\n",
              me, v, r_f, i, r_d);
    }
    integral += step*ran_d[i];
    i++;
  }
  if (!KINDACLOSE(integral, k->integral(parm))) {
    fprintf(stderr, 
            "discrete integral %f != %f\n", integral, k->integral(parm));
    /* not a fatal error */
  }
  
  /* it all checks out; write the file */
  if (!(fout = fopen(outS, "w"))) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing\n", me, outS);
    exit(1);
  }
  for (i=0; i<=len-1; i++) {
    fprintf(fout, "%g %g\n", dom_f[i], ran_f[i]);
  }
  fclose(fout);
  
  free(dom_d);
  free(ran_d);
  free(dom_f);
  free(ran_f);
  fprintf(stderr, "(for matlab:)\n");
  fprintf(stderr, "x = dlmread(\'%s\', \' \'); plot(x(:,1), x(:,2));\n", outS);
  exit(0);
}
