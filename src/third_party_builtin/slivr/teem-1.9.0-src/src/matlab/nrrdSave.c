#include "mex.h"
#include <teem/nrrd.h>

int
typeMtoN(mxClassID mtype) {
  int ntype;

  switch(mtype) {
  case mxINT8_CLASS:
    ntype = nrrdTypeChar;
    break;
  case mxUINT8_CLASS:
    ntype = nrrdTypeUChar;
    break;
  case mxINT16_CLASS:
    ntype = nrrdTypeShort;
    break;
  case mxUINT16_CLASS:
    ntype = nrrdTypeUShort;
    break;
  case mxINT32_CLASS:
    ntype = nrrdTypeInt;
    break;
  case mxUINT32_CLASS:
    ntype = nrrdTypeUInt;
    break;
  case mxINT64_CLASS:
    ntype = nrrdTypeLLong;
    break;
  case mxUINT64_CLASS:
    ntype = nrrdTypeULLong;
    break;
  case mxSINGLE_CLASS:
    ntype = nrrdTypeFloat;
    break;
  case mxDOUBLE_CLASS:
    ntype = nrrdTypeDouble;
    break;
  default:
    ntype = nrrdTypeUnknown;
    break;
  }
  return ntype;
}

void mexFunction(int nlhs, mxArray *plhs[],
  int nrhs, const mxArray *prhs[])
{
  char me[]="nrrdSave", *filename, *errPtr, errBuff[AIR_STRLEN_MED];
  int filenameLen, ntype;
  size_t sizeZ[NRRD_DIM_MAX];
  unsigned int dim, axIdx;
  Nrrd *nrrd;
  airArray *mop;
  const mxArray *filenameMx, *arrayMx;
  
  if (!(2 == nrhs && mxIsChar(prhs[0]) )) {
    sprintf(errBuff, "%s: requires two args: one string, one array", me);
    mexErrMsgTxt(errBuff);
  }
  filenameMx = prhs[0];
  arrayMx = prhs[1];

  if (mxIsComplex(arrayMx)) {
    sprintf(errBuff, "%s: sorry, array must be real", me);
    mexErrMsgTxt(errBuff);
  }
  ntype = typeMtoN(mxGetClassID(arrayMx));
  if (nrrdTypeUnknown == ntype) {
    sprintf(errBuff, "%s: sorry, can't handle type %s",
            me, mxGetClassName(arrayMx));
    mexErrMsgTxt(errBuff);
  }
  dim = mxGetNumberOfDimensions(arrayMx);
  if (!( 1 <= dim && dim <= NRRD_DIM_MAX )) {
    sprintf(errBuff, "%s: number of array dimensions %d outside range [1,%d]",
            me, dim, NRRD_DIM_MAX);
    mexErrMsgTxt(errBuff);
  }
  
  filenameLen = mxGetM(filenameMx)*mxGetN(filenameMx)+1;
  filename = mxCalloc(filenameLen, sizeof(mxChar));    /* managed by Matlab */
  mxGetString(filenameMx, filename, filenameLen);
  
  for (axIdx=0; axIdx<dim; axIdx++) {
    sizeZ[axIdx] = mxGetDimensions(arrayMx)[axIdx];
  }
  nrrd = nrrdNew();
  airMopAdd(mop, nrrd, (airMopper)nrrdNix, airMopAlways);
  if (nrrdWrap_nva(nrrd, mxGetPr(arrayMx), ntype, dim, sizeZ)
      || nrrdSave(filename, nrrd, NULL)) {
    errPtr = biffGetDone(NRRD);
    airMopAdd(mop, errPtr, airFree, airMopAlways);
    sprintf(errBuff, "%s: error saving NRRD:\n%s", me, errPtr);
    airMopError(mop);
    mexErrMsgTxt(errBuff);
  }

  airMopOkay(mop);
  return;
}
