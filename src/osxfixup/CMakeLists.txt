#convert all libraries using osxfixup
IF(APPLE)
    INSTALL(CODE
            "
            MESSAGE(STATUS \"Running osxfixup on libraries and executables\")
            EXECUTE_PROCESS(WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
             COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/osxfixup/osxfixup.py \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION} @executable_path/.. 0
             OUTPUT_VARIABLE OSXOUT)
             MESSAGE(STATUS \"\${OSXOUT}\")")
ENDIF(APPLE)
