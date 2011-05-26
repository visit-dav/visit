include(${VISIT_SOURCE_DIR}/config-site/windows.cmake)

# can provide overrides for windows.cmake settings below

# disable some warnings
# 4244 conversion double to float etc
# 4305 truncation from 'double' to 'float', etc
# 4800 'int' forcing value to bool (performance warning)
ADD_DEFINITIONS(/wd4244 /wd4305 /wd4800)

OPTION(VISIT_INSTALL_THIRD_PARTY "Install VisIt's 3rd party I/O libs and includes to permit plugin development" ON)

