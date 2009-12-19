ADD_LIBRARY(moss
  defaultsMoss.c
  hestMoss.c
  methodsMoss.c
  sampler.c
  xform.c
)

TARGET_LINK_LIBRARIES(moss ell nrrd biff hest air)

VISIT_INSTALL_TARGETS(moss)
