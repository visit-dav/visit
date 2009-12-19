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


#include "../air.h"

#if TEEM_DIO == 0
#else
/* HEY: these may be SGI-specific */
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#endif

int
main(int argc, char *argv[]) {
#if TEEM_DIO == 0

  AIR_UNUSED(argc);
  fprintf(stderr, "%s: no direct-io testing for you\n", argv[0]);
  return 1;
#else 
  char *me, *fname, *multS, *data;
  FILE *file;
  double time0, time1, time2;
  int fd, align, mult, min, max, ret;
  size_t size;
  airArray *mop;

  me = argv[0];
  if (3 != argc) {
    /*                      0      1         2    (3) */
    fprintf(stderr, "usage: %s <filename> <mult>\n", me);
    return 1;
  }
  fname = argv[1];
  multS = argv[2];
  if (1 != sscanf(multS, "%d", &mult)) {
    fprintf(stderr, "%s: couln't parse mult %s as int\n", me, multS);
    return 1;
  }

  mop = airMopNew();
  if (!(file = fopen(fname, "w"))) {
    fprintf(stderr, "%s: couldn't open %s for writing\n", me, fname);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);
  fd = fileno(file);
  if (-1 == fd) {
    fprintf(stderr, "%s: couldn't get underlying descriptor\n", me);
    airMopError(mop); return 1;
  }
  fprintf(stderr, "%s: fd(%s) = %d\n", me, fname, fd);

  ret = airDioTest(fd, NULL, 0);
  if (airNoDio_okay != ret) {
    fprintf(stderr, "%s: no good: \"%s\"\n", me, airNoDioErr(ret));
    airMopError(mop); return 1;
  }

  airDioInfo(&align, &min, &max, fd);
  fprintf(stderr, "%s: --> align=%d, min=%d, max=%d\n", me, align, min, max);
  size = (size_t)max*mult;
  data = airDioMalloc(size, fd);
  if (!data) {
    fprintf(stderr, "%s: airDioMalloc(" _AIR_SIZE_T_CNV ") failed\n", me,
            size);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, data, airFree, airMopAlways);
  fprintf(stderr, "\ndata size = %g MB\n", (double)size/(1024*1024));
  
  /* -------------------------------------------------------------- */
  fprintf(stderr, "(1) non-aligned memory, regular write:\n");
  time0 = airTime();
  if (size-1 != write(fd, data+1, size-1)) {
    fprintf(stderr, "%s: write failed\n", me);
    airMopError(mop); return 1;
  }
  time1 = airTime();
  fsync(fd);
  time2 = airTime();
  fprintf(stderr, "   time = %g + %g = %g (%g MB/sec)\n",
          time1 - time0, time2 - time1, time2 - time0,
          (size/(1024*1024)) / (time2 - time0));
  airMopSub(mop, file, (airMopper)airFclose);
  fclose(file);
  /* -------------------------------------------------------------- */
  /* -------------------------------------------------------------- */
  fprintf(stderr, "(2) aligned memory, regular write:\n");
  file = fopen(fname, "w");
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);
  fd = fileno(file);

  time0 = airTime();
  if (size != write(fd, data, size)) {
    fprintf(stderr, "%s: write failed\n", me);
    airMopError(mop); return 1;
  }
  time1 = airTime();
  fsync(fd);
  time2 = airTime();
  fprintf(stderr, "   time = %g + %g = %g (%g MB/sec)\n",
          time1 - time0, time2 - time1, time2 - time0,
          (size/(1024*1024)) / (time2 - time0));
  airMopSub(mop, file, (airMopper)airFclose);
  fclose(file);
  /* -------------------------------------------------------------- */
  /* -------------------------------------------------------------- */
  fprintf(stderr, "(3) aligned memory, air's direct IO:\n");
  file = fopen(fname, "w");
  airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);
  fd = fileno(file);

  time0 = airTime();
  if (size != airDioWrite(fd, data, size)) {
    fprintf(stderr, "%s: write failed\n", me);
    airMopError(mop); return 1;
  }
  time1 = airTime();
  fsync(fd);
  time2 = airTime();
  fprintf(stderr, "   time = %g + %g = %g (%g MB/sec)\n",
          time1 - time0, time2 - time1, time2 - time0,
          (size/(1024*1024)) / (time2 - time0));
  airMopSub(mop, file, (airMopper)airFclose);
  fclose(file);
  /* -------------------------------------------------------------- */
  /* -------------------------------------------------------------- */
  fprintf(stderr, "(4) aligned memory, direct IO by hand:\n");
  {
    /* "input": fname, size, data */
    int flags;
    struct dioattr dio;
    char *ptr;
    size_t remain, totalrit, rit, part;

    file = fopen(fname, "w");
    if (-1 == (fd = fileno(file))) {
      fprintf(stderr, "%s: couldn't get underlying descriptor\n", me);
      airMopError(mop); return 1;
    }
    airMopAdd(mop, file, (airMopper)airFclose, airMopAlways);

    flags = fcntl(fd, F_GETFL);
    if (-1 == fcntl(fd, F_SETFL, flags | FDIRECT)) {
      fprintf(stderr, "%s: couldn't turn on direct IO\n", me);
      airMopError(mop); return 1;
    }
    if (0 != fcntl(fd, F_DIOINFO, &dio)) {
      fprintf(stderr, "%s: couldn't learn direct IO specifics", me);
      airMopError(mop); return 1;
    }
    
    remain = size;
    totalrit = 0;
    ptr = data;
    time0 = airTime();
    do {
      part = remain > dio.d_maxiosz ? dio.d_maxiosz : remain;
      rit = write(fd, ptr, part);
      if (rit != part) {
        fprintf(stderr, "%s: write failed\n", me);
        airMopError(mop); return 1;
      }
      totalrit += rit;
      ptr += rit;
      remain -= rit;
    } while (remain);
    time1 = airTime();
    fsync(fd);
    time2 = airTime();
    fprintf(stderr, "   time = %g + %g = %g (%g MB/sec)\n",
            time1 - time0, time2 - time1, time2 - time0,
            (size/(1024*1024)) / (time2 - time0));
    airMopSub(mop, file, (airMopper)airFclose);
    fclose(file);
  }
  /* -------------------------------------------------------------- */

  airMopError(mop); 
  exit(0);
#endif
}
