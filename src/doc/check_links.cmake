# Indirectly called script for portability across all
# platforms of REGEX scanning of *generated* content
FILE(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/_linkcheck/output.txt SPHINX_LINKCHECK_BROKEN REGEX \\[[Bb][Rr][Oo][Kk][Ee][Nn]\\])

FOREACH(BROKEN_LINK ${SPHINX_LINKCHECK_BROKEN})
    MESSAGE(STATUS ${BROKEN_LINK})
ENDFOREACH()

LIST(LENGTH SPHINX_LINKCHECK_BROKEN NBROKEN)

IF(SPHINX_LINKCHECK_BROKEN)
    MESSAGE(FATAL_ERROR "Found ${NBROKEN} broken links.\n   Grep for 'broken' in ${CMAKE_CURRENT_BINARY_DIR}/_linkcheck/output.txt")
ELSE()
    FILE(STRINGS ${CMAKE_CURRENT_BINARY_DIR}/conf.py SPHINX_LINKCHECK_IGNORED REGEX linkcheck_ignore)
    IF(SPHINX_LINKCHECK_IGNORED)
        MESSAGE(WARNING "Some links ignored:\n   ${SPHINX_LINKCHECK_IGNORED}")
    ENDIF()
ENDIF()
