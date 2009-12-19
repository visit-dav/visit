ADD_LIBRARY(mite
  defaultsMite.c
  kindnot.c
  ray.c
  renderMite.c
  shade.c
  thread.c
  txf.c
  user.c
)

TARGET_LINK_LIBRARIES(mite ten hoover limn gage ell nrrd biff air)
VISIT_INSTALL_TARGETS(mite)
