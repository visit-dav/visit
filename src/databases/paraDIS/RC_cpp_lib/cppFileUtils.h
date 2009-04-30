/* return number of bytes read, throw string on error */
#ifndef CPPFILEUTIL_H
#define CPPFILEUTIL_H
#include "stringutil.h"

template <class W>
u_int32_t fread_loop(FILE *theFile, u_int32_t elems2Read, W *bufp) {
  u_int32_t totalRead = fread(bufp, sizeof(W), elems2Read, theFile);
  bufp += totalRead; 
  while (elems2Read > totalRead) {
    u_int32_t numread = fread(bufp+numread, sizeof(W), elems2Read - totalRead, theFile); 
    if (!numread) {
      string errstring = string("Data read failed after ") +
    doubleToString(totalRead+numread) +
    "bytes: "; 
      int err = ferror(theFile);
      if (err) throw errstring + "error "+doubleToString(err);
      if (feof(theFile)) throw errstring + "unexpected EOF";
      else throw errstring + "zero items read, but no error or EOF (impossible!)";
    }
    bufp += numread; 
    totalRead += numread; 
  }
  return totalRead; 
}

#endif

