#include "mex.h"
#include <teem/nrrd.h>

mxClassID
typeNtoM(int ntype) {
  mxClassID mtype;

  switch(ntype) {
  case nrrdTypeChar:
    mtype = mxINT8_CLASS;
    break;
  case nrrdTypeUChar:
    mtype = mxUINT8_CLASS;
    break;
  case nrrdTypeShort:
    mtype = mxINT16_CLASS;
    break;
  case nrrdTypeUShort:
    mtype = mxUINT16_CLASS;
    break;
  case nrrdTypeInt:
    mtype = mxINT32_CLASS;
    break;
  case nrrdTypeUInt:
    mtype = mxUINT32_CLASS;
    break;
  case nrrdTypeLLong:
    mtype = mxINT64_CLASS;
    break;
  case nrrdTypeULLong:
    mtype = mxUINT64_CLASS;
    break;
  case nrrdTypeFloat:
    mtype = mxSINGLE_CLASS;
    break;
  case nrrdTypeDouble:
    mtype = mxDOUBLE_CLASS;
    break;
  default:
    mtype = mxUNKNOWN_CLASS;
    break;
  }
  return mtype;
}

void mexFunction(int nlhs, mxArray *plhs[],
  int nrhs, const mxArray *prhs[])
{
  char me[]="nrrdLoad", *filename, *errPtr, errBuff[AIR_STRLEN_MED];
  int filenameLen, sizeI[NRRD_DIM_MAX];
  mxClassID mtype;
  size_t sizeZ[NRRD_DIM_MAX];
  unsigned int axIdx;
  Nrrd *nrrd;
  NrrdIoState *nio;
  airArray *mop;

  if (!(1 == nrhs && mxIsChar(prhs[0]))) {
    sprintf(errBuff, "%s: requires one string argument (the name of the file)", me);
    mexErrMsgTxt(errBuff);
  }

  mop = airMopNew();
  filenameLen = mxGetM(prhs[0])*mxGetN(prhs[0])+1;
  filename = mxCalloc(filenameLen, sizeof(mxChar));  /* managed by Matlab */
  mxGetString(prhs[0], filename, filenameLen);

  nrrd = nrrdNew();
  airMopAdd(mop, nrrd, (airMopper)nrrdNix, airMopAlways);
  nio = nrrdIoStateNew();
  airMopAdd(mop, nio, (airMopper)nrrdIoStateNix, airMopAlways);
  nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);

  /* read header, but no data */
  if (nrrdLoad(nrrd, filename, nio)) {
    errPtr = biffGetDone(NRRD);
    airMopAdd(mop, errPtr, airFree, airMopAlways);
    sprintf(errBuff, "%s: trouble reading NRRD header:\n%s", me, errPtr);
    airMopError(mop);
    mexErrMsgTxt(errBuff);
  }
  mtype = typeNtoM(nrrd->type);
  if (mxUNKNOWN_CLASS == mtype) {
    sprintf(errBuff, "%s: sorry, can't handle type %s (%d)", me,
            airEnumStr(nrrdType, nrrd->type), nrrd->type);
    airMopError(mop);
    mexErrMsgTxt(errBuff);
  }

  /* allocate matlab array based on nrrd struct */
  for (axIdx=0; axIdx<nrrd->dim; axIdx++) {
    sizeI[axIdx] = nrrd->axis[axIdx].size;
  }
  plhs[0]=mxCreateNumericArray(nrrd->dim,sizeI,mtype,mxREAL);

  /* copy data pointer */
  nrrd->data = mxGetPr(plhs[0]);

  /* read second time, now loading data */
  if (nrrdLoad(nrrd, filename, NULL)) {
    errPtr = biffGetDone(NRRD);
    airMopAdd(mop, errPtr, airFree, airMopAlways);
    sprintf(errBuff, "%s: trouble reading NRRD:\n%s", me, errPtr);
    airMopError(mop);
    mexErrMsgTxt(errBuff);
  }

  airMopOkay(mop);
  return;
}
