#*****************************************************************************
#
# Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# Modifications:
#
#****************************************************************************/

if (WIN32)
    return()
endif()

if(VISIT_LLVM_DIR)

message("VISIT_LLVM_DIR: ${VISIT_LLVM_DIR}")
    find_library(LLVM_LIB LLVM 
                 PATHS ${VISIT_LLVM_DIR}/lib
                 NO_DEFAULT_PATH)
   
    if(LLVM_LIB)
        get_filename_component(LLVM_LIBRARY_DIR ${LLVM_LIB} DIRECTORY)
        get_filename_component(LLVM_LIBRARY ${LLVM_LIB} NAME)
message("LLVM lib : ${LLVM_LIB}")
message("LLVM lib dr: ${LLVM_LIBRARY_DIR}")

        execute_process(COMMAND objdump -p ${LLVM_LIB}
                        COMMAND grep SONAME
                        RESULT_VARIABLE LLVM_SONAME_RESULT
                        OUTPUT_VARIABLE LLVM_SONAME
                        ERROR_VARIABLE LLVM_SONAME_ERROR)

        if(LLVM_SONAME)
            string(REPLACE "SONAME" "" LLVM_SONAME ${LLVM_SONAME})
            string(STRIP ${LLVM_SONAME} LLVM_SONAME)
message("LLVM soname: ${LLVM_SONAME}")
            set(LLVM_LIBRARY ${LLVM_SONAME})
        endif()


        # now want to create the symlinks
        install(CODE
                "execute_process(WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/
                COMMAND ${CMAKE_COMMAND} -E remove -f ${LLVM_LIBRARY}
                OUTPUT_VARIABLE LINK_CLEAN_OUT)
                MESSAGE(STATUS \"\${LINK_CLEAN_OUT}\")
            ")
        install(CODE
            "execute_process(WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VISIT_INSTALLED_VERSION_LIB}/
            COMMAND ${CMAKE_COMMAND} -E create_symlink ../${LLVM_LIBRARY} ${LLVM_LIBRARY}
            OUTPUT_VARIABLE LIB_SYMLINK)
            MESSAGE(STATUS \"\${LIB_SYMLINK}\")
             ")
 
    endif(LLVM_LIB)
# else
# how to find/use system LLVM ?
endif(VISIT_LLVM_DIR)
