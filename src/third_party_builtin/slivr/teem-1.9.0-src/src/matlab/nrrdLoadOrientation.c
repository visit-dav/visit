#include "mex.h"
#include <teem/nrrd.h>

void mexFunction(int nlhs, mxArray *plhs[],
  int nrhs, const mxArray *prhs[])
{
  char me[]="nrrdLoadOrientation", *filename, *errPtr, errBuff[AIR_STRLEN_MED];
  int filenameLen, sizeI[NRRD_DIM_MAX];
  mxClassID mtype;
  size_t sizeZ[NRRD_DIM_MAX];
  unsigned int axIdx;
  Nrrd *nrrd;
  NrrdIoState *nio;
  airArray *mop;
  unsigned int domainAxisNum, domainAxisIdx[NRRD_DIM_MAX], axisIdx,
    rowIdx, colIdx;
  double spacing, spaceDir[NRRD_SPACE_DIM_MAX];
  int spacingStatus;

  if (!(1 == nrhs && mxIsChar(prhs[0]))) {
    sprintf(errBuff, "%s: requires one string argument (the name of the file)",
            me);
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

  domainAxisNum = nrrdDomainAxesGet(nrrd, domainAxisIdx);
  plhs[0] = mxCreateDoubleMatrix(domainAxisNum /* # rows */, 
                                 nrrd->dim /* # cols */, mxREAL);
  for (colIdx=0; colIdx<nrrd->dim; colIdx++) {
    spacingStatus = nrrdSpacingCalculate(nrrd, domainAxisIdx[colIdx], 
                                         &spacing, spaceDir);
    switch(spacingStatus) {
    case nrrdSpacingStatusNone:
      for (rowIdx=0; rowIdx<domainAxisNum; rowIdx++) {
        mxGetPr(plhs[0])[rowIdx + domainAxisNum*colIdx] = AIR_NAN;
      }
      break;
    case nrrdSpacingStatusScalarNoSpace:
      for (rowIdx=0; rowIdx<domainAxisNum; rowIdx++) {
        mxGetPr(plhs[0])[rowIdx + domainAxisNum*colIdx] = 0;
      }
      if (colIdx < domainAxisNum) {
        mxGetPr(plhs[0])[colIdx + domainAxisNum*colIdx] = spacing;
      }
      break;
    case nrrdSpacingStatusDirection:
      for (rowIdx=0; rowIdx<domainAxisNum; rowIdx++) {
        mxGetPr(plhs[0])[rowIdx + domainAxisNum*colIdx] = 
          nrrd->axis[colIdx].spaceDirection[rowIdx];
      }
      break;
    case nrrdSpacingStatusUnknown:
      sprintf(errBuff, "%s: error interpreting axis %u spacing "
              "(nrrdSpacingStatusUnknown)", me, colIdx);
      airMopError(mop);
      mexErrMsgTxt(errBuff);
      break;
    case nrrdSpacingStatusScalarWithSpace:
      sprintf(errBuff, "%s: error interpreting axis %u spacing "
              "(nrrdSpacingScalarWithSpace)", me, colIdx);
      airMopError(mop);
      mexErrMsgTxt(errBuff);
      break;
    }
  }


  airMopOkay(mop);
  return;
}
