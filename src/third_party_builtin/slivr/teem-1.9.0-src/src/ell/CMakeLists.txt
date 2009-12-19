ADD_LIBRARY(ell
  cubicEll.c
  eigen.c
  genmat.c
  mat.c
  miscEll.c
  quat.c
  vecEll.c
)

TARGET_LINK_LIBRARIES(ell nrrd biff air)
VISIT_INSTALL_TARGETS(ell)
