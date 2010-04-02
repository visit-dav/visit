INSTALL(FILES UltraCurve.py  UltraGrammar.py  ultraparse.py
    DESTINATION ${VISIT_INSTALLED_VERSION}/ultrawrapper
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ 
)

IF (WIN32)
    FILE(COPY UltraCurve.py UltraGrammar.py ultraparse.py
         DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ultrawrapper)

ENDIF (WIN32)
