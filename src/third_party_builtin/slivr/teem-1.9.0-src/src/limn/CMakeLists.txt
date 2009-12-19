ADD_LIBRARY(limn
  defaultsLimn.c
  qn.c
  light.c
  envmap.c
  cam.c
  methodsLimn.c
  obj.c
  transform.c
  shapes.c
  renderLimn.c
  io.c
  hestLimn.c
  splineMisc.c
  splineMethods.c
  splineEval.c
  enumsLimn.c
  contour.c
  polydata.c
  )

TARGET_LINK_LIBRARIES(limn ell nrrd biff hest air)
VISIT_INSTALL_TARGETS(limn)
