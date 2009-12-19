ADD_LIBRARY(coil
  coreCoil.c
  defaultsCoil.c
  enumsCoil.c
  methodsCoil.c
  realmethods.c
  scalarCoil.c
  tensorCoil.c
)

TARGET_LINK_LIBRARIES(coil ten gage ell nrrd biff air)
VISIT_INSTALL_TARGETS(coil)
