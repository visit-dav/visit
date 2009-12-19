ADD_LIBRARY(nrrd
  accessors.c
  apply1D.c
  apply2D.c
  arith.c
  arraysNrrd.c
  axis.c
  cc.c
  ccmethods.c
  comment.c
  convertNrrd.c
  defaultsNrrd.c
  encoding.c
  encodingAscii.c
  encodingBzip2.c
  encodingGzip.c
  encodingHex.c
  encodingRaw.c
  endianNrrd.c
  enumsNrrd.c
  filt.c
  format.c
  formatEPS.c
  formatNRRD.c
  formatPNG.c
  formatPNM.c
  formatText.c
  formatVTK.c
  gzio.c
  hestNrrd.c
  histogram.c
  iter.c
  kernel.c
  keyvalue.c
  map.c
  measure.c
  methodsNrrd.c
  parseNrrd.c
  range.c
  read.c
  reorder.c
  resampleContext.c
  resampleNrrd.c
  simple.c
  subset.c
  superset.c
  tmfKernel.c
  winKernel.c
  write.c
)

TARGET_LINK_LIBRARIES(nrrd biff hest air)

IF(TEEM_BZIP2_LIB)
  TARGET_LINK_LIBRARIES(nrrd ${TEEM_BZIP2_LIB})
ENDIF(TEEM_BZIP2_LIB)
IF(TEEM_ZLIB_LIB)
  TARGET_LINK_LIBRARIES(nrrd ${TEEM_ZLIB_LIB})
  IF(TEEM_PNG_LIB)
    TARGET_LINK_LIBRARIES(nrrd ${TEEM_PNG_LIB})
  ENDIF(TEEM_PNG_LIB)
ENDIF(TEEM_ZLIB_LIB)

VISIT_INSTALL_TARGETS(nrrd)
