ADD_LIBRARY(push
  defaultsPush.c methodsPush.c binning.c
  forces.c corePush.c setup.c action.c
)

TARGET_LINK_LIBRARIES(push ten gage ell nrrd biff air)

VISIT_INSTALL_TARGETS(push)
