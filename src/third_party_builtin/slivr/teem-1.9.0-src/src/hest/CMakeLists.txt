ADD_LIBRARY(hest
  defaultsHest.c
  methodsHest.c
  parseHest.c
  usage.c
)

TARGET_LINK_LIBRARIES(hest air)

VISIT_INSTALL_TARGETS(hest)
VISIT_TEEM_ADD_FOLDER(hest)
