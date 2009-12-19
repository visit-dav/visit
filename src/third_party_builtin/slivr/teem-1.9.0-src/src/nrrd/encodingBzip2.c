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

#if TEEM_BZIP2
#include <bzlib.h>
#endif

int
_nrrdEncodingBzip2_available(void) {

#if TEEM_BZIP2
  return AIR_TRUE;
#else
  return AIR_FALSE;
#endif
}

int
_nrrdEncodingBzip2_read(FILE *file, void *_data, size_t elNum,
                        Nrrd *nrrd, NrrdIoState *nio) {
  char me[]="_nrrdEncodingBzip2_read", err[BIFF_STRLEN];
#if TEEM_BZIP2
  size_t bsize, total_read, block_size;
  int read, i, bzerror=BZ_OK;
  char *data;
  BZFILE* bzfin;
  
  bsize = nrrdElementSize(nrrd)*elNum;

  /* Create the BZFILE* for reading in the gzipped data. */
  bzfin = BZ2_bzReadOpen(&bzerror, file, 0, 0, NULL, 0);
  if (bzerror != BZ_OK) {
    /* there was a problem */
    sprintf(err, "%s: error opening BZFILE: %s", me, 
            BZ2_bzerror(bzfin, &bzerror));
    biffAdd(NRRD, err);
    BZ2_bzReadClose(&bzerror, bzfin);
    return 1;
  }

  /* Here is where we do the byte skipping. */
  for(i = 0; i < nio->byteSkip; i++) {
    unsigned char b;
    /* Check to see if a single byte was able to be read. */
    read = BZ2_bzRead(&bzerror, bzfin, &b, 1);
    if (read != 1 || bzerror != BZ_OK) {
      sprintf(err, "%s: hit an error skipping byte %d of %d: %s",
              me, i, nio->byteSkip, BZ2_bzerror(bzfin, &bzerror));
      biffAdd(NRRD, err);
      return 1;
    }
  }
  
  /* bzip2 can handle data sizes up to INT_MAX, so we can't just 
     pass in the bsize, because it might be too large for an int.
     Therefore it must be read in chunks if the size is larger 
     than INT_MAX. */
  if (bsize <= INT_MAX) {
    block_size = bsize;
  } else {
    block_size = INT_MAX;
  }

  /* This counter will help us to make sure that we read as much data
     as we think we should. */
  total_read = 0;
  /* Pointer to the blocks as we read them. */
  data = (char *)_data;
  
  /* Ok, now we can begin reading. */
  bzerror = BZ_OK;
  while ((read = BZ2_bzRead(&bzerror, bzfin, data, block_size))
          && (BZ_OK == bzerror || BZ_STREAM_END == bzerror) ) {
    /* Increment the data pointer to the next available spot. */
    data += read;
    total_read += read;
    /* We only want to read as much data as we need, so we need to check
       to make sure that we don't request data that might be there but that
       we don't want.  This will reduce block_size when we get to the last
       block (which may be smaller than block_size).
    */
    if (bsize >= total_read 
        && bsize - total_read < block_size)
      block_size = bsize - total_read;
  }
  
  if (!( BZ_OK == bzerror || BZ_STREAM_END == bzerror )) {
    sprintf(err, "%s: error reading from BZFILE: %s",
            me, BZ2_bzerror(bzfin, &bzerror));
    biffAdd(NRRD, err);
    return 1;
  }

  /* Close the BZFILE. */
  BZ2_bzReadClose(&bzerror, bzfin);
  if (BZ_OK != bzerror) {
    sprintf(err, "%s: error closing BZFILE: %s", me,
            BZ2_bzerror(bzfin, &bzerror));
    biffAdd(NRRD, err);
    return 1;
  }
  
  /* Check to see if we got out as much as we thought we should. */
  if (total_read != bsize) {
    sprintf(err, "%s: expected " _AIR_SIZE_T_CNV " bytes and received "
            _AIR_SIZE_T_CNV " bytes",
            me, bsize, total_read);
    biffAdd(NRRD, err);
    return 1;
  }
  
  return 0;
#else
  AIR_UNUSED(file);
  AIR_UNUSED(_data);
  AIR_UNUSED(elNum);
  AIR_UNUSED(nrrd);
  AIR_UNUSED(nio);
  sprintf(err, "%s: sorry, this nrrd not compiled with bzip2 enabled", me);
  biffAdd(NRRD, err); return 1;
#endif
}

int
_nrrdEncodingBzip2_write(FILE *file, const void *_data, size_t elNum,
                         const Nrrd *nrrd, NrrdIoState *nio) {
  char me[]="_nrrdEncodingBzip2_write", err[BIFF_STRLEN];
#if TEEM_BZIP2
  size_t bsize, total_written, block_size;
  int bs, bzerror=BZ_OK;
  char *data;
  BZFILE* bzfout;

  bsize = nrrdElementSize(nrrd)*elNum;

  /* Set compression block size. */
  if (1 <= nio->bzip2BlockSize && nio->bzip2BlockSize <= 9) {
    bs = nio->bzip2BlockSize;
  } else {
    bs = 9;
  }
  /* Open bzfile for writing. Verbosity and work factor are set
     to default values. */
  bzfout = BZ2_bzWriteOpen(&bzerror, file, bs, 0, 0);
  if (BZ_OK != bzerror) {
    sprintf(err, "%s: error opening BZFILE: %s", me, 
            BZ2_bzerror(bzfout, &bzerror));
    biffAdd(NRRD, err);
    BZ2_bzWriteClose(&bzerror, bzfout, 0, NULL, NULL);
    return 1;
  }

  /* bzip2 can handle data sizes up to INT_MAX, so we can't just 
     pass in the bsize, because it might be too large for an int.
     Therefore it must be read in chunks if the bsize is larger 
     than INT_MAX. */
  if (bsize <= INT_MAX) {
    block_size = bsize;
  } else {
    block_size = INT_MAX;
  }

  /* This counter will help us to make sure that we write as much data
     as we think we should. */
  total_written = 0;
  /* Pointer to the blocks as we write them. */
  data = (char *)_data;
  
  /* Ok, now we can begin writing. */
  bzerror = BZ_OK;
  while (bsize - total_written > block_size) {
    BZ2_bzWrite(&bzerror, bzfout, data, block_size);
    if (BZ_OK != bzerror) break;
    /* Increment the data pointer to the next available spot. */
    data += block_size; 
    total_written += block_size;
  }
  /* write the last (possibly smaller) block when its humungous data;
     write the whole data when its small */
  if (BZ_OK == bzerror) {
    block_size = bsize >= total_written ? bsize - total_written : 0;
    BZ2_bzWrite(&bzerror, bzfout, data, block_size);
    total_written += block_size;
  }

  if (BZ_OK != bzerror) {
    sprintf(err, "%s: error writing to BZFILE: %s",
            me, BZ2_bzerror(bzfout, &bzerror));
    biffAdd(NRRD, err);
    return 1;
  }

  /* Close the BZFILE. */
  BZ2_bzWriteClose(&bzerror, bzfout, 0, NULL, NULL);
  if (BZ_OK != bzerror) {
    sprintf(err, "%s: error closing BZFILE: %s", me,
            BZ2_bzerror(bzfout, &bzerror));
    biffAdd(NRRD, err);
    return 1;
  }
  
  /* Check to see if we got out as much as we thought we should. */
  if (total_written != bsize) {
    sprintf(err, "%s: expected to write " _AIR_SIZE_T_CNV " bytes, but only "
            "wrote " _AIR_SIZE_T_CNV,
            me, bsize, total_written);
    biffAdd(NRRD, err);
    return 1;
  }
  
  return 0;
#else
  AIR_UNUSED(file);
  AIR_UNUSED(_data);
  AIR_UNUSED(elNum);
  AIR_UNUSED(nrrd);
  AIR_UNUSED(nio);
  sprintf(err, "%s: sorry, this nrrd not compiled with bzip2 enabled", me);
  biffAdd(NRRD, err); return 1;
#endif
}

const NrrdEncoding
_nrrdEncodingBzip2 = {
  "bzip2",      /* name */
  "raw.bz2",   /* suffix */
  AIR_TRUE,    /* endianMatters */
  AIR_TRUE,   /* isCompression */
  _nrrdEncodingBzip2_available,
  _nrrdEncodingBzip2_read,
  _nrrdEncodingBzip2_write
};

const NrrdEncoding *const
nrrdEncodingBzip2 = &_nrrdEncodingBzip2;
