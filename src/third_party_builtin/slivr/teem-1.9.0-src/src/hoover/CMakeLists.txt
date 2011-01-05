ADD_LIBRARY(hoover
  defaultsHoover.c
  methodsHoover.c
  rays.c
  stub.c
)

TARGET_LINK_LIBRARIES(hoover limn ell nrrd biff air)
VISIT_INSTALL_TARGETS(hoover)
VISIT_TEEM_ADD_FOLDER(hoover)
