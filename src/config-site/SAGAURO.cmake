INCLUDE(${VISIT_SOURCE_DIR}/config-site/windows.cmake)
ADD_DEFINITIONS(/wd4244 /wd4305 /wd4800)
OPTION(VISIT_INSTALL_THIRD_PARTY "Install VisIt's 3rd party I/O libs and includes to permit plugin development" ON)

