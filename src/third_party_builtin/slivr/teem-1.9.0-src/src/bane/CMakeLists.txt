ADD_LIBRARY(bane
  clip.c
  defaultsBane.c
  gkmsFlotsam.c
  gkmsHvol.c
  gkmsInfo.c
  gkmsMite.c
  gkmsOpac.c
  gkmsPvg.c
  gkmsScat.c
  gkmsTxf.c
  hvol.c
  inc.c
  measr.c
  methodsBane.c
  rangeBane.c
  scat.c
  trex.c
  trnsf.c
  valid.c
)


TARGET_LINK_LIBRARIES(bane gage unrrdu nrrd biff air)
VISIT_INSTALL_TARGETS(bane)
