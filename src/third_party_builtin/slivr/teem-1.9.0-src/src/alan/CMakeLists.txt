ADD_LIBRARY(alan
coreAlan.c
enumsAlan.c
methodsAlan.c
)


TARGET_LINK_LIBRARIES(alan air biff ell nrrd)
VISIT_INSTALL_TARGETS(alan)
