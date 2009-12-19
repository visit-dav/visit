ADD_LIBRARY(air
  754.c
  array.c
  dio.c
  endianAir.c
  enum.c
  miscAir.c
  mop.c 
  parseAir.c
  randMT.c
  sane.c
  math.c
  string.c
  threadAir.c
  )

IF(UNIX)
  TARGET_LINK_LIBRARIES(air -lm)
ENDIF(UNIX)

VISIT_INSTALL_TARGETS(air)
