#/what/is/the/path/to/bin/cmake

##
## Site configuration for a DEC OSF1 system.  Any special configuration that
## should be done on an OSF1 system should go in here.
##
## If we're on an alpha, use a different C compiler.
##

SET(VISIT_C_COMPILER /usr/bin/cc)
SET(VISIT_C_FLAGS "-std1")
   fi
#fi
