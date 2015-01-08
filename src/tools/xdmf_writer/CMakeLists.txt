#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#   Brad Whitlock, Tue Apr 16 09:31:42 PDT 2013
#   Add libm.
#
#****************************************************************************/

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${HDF5_INCLUDE_DIR}
)

# Add link directories
LINK_DIRECTORIES(${VISIT_LIBRARY_DIR}
${HDF5_LIBRARY_DIR}
)


#********************************* SERIAL ************************************
ADD_EXECUTABLE(xdmf_curv3d xdmf_curv3d.c)
TARGET_LINK_LIBRARIES(xdmf_curv3d xdmf_writer_ser m) 

ADD_EXECUTABLE(xdmf_ucd3d xdmf_ucd3d.c)
TARGET_LINK_LIBRARIES(xdmf_ucd3d xdmf_writer_ser m) 

ADD_EXECUTABLE(xdmf_curv3d_wtime xdmf_curv3d_wtime.c)
TARGET_LINK_LIBRARIES(xdmf_curv3d_wtime xdmf_writer_ser m) 

ADD_EXECUTABLE(xdmf_multi_curv3d xdmf_multi_curv3d.c)
TARGET_LINK_LIBRARIES(xdmf_multi_curv3d xdmf_writer_ser m) 

ADD_LIBRARY(xdmf_writer_ser xdmf_writer.c)
TARGET_LINK_LIBRARIES(xdmf_writer_ser ${HDF5_LIB} ${ZLIB_LIB} m)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_EXECUTABLE(xdmf_mpi_example xdmf_mpi_example.c)
    TARGET_LINK_LIBRARIES(xdmf_mpi_example xdmf_writer_par m)

    ADD_PARALLEL_EXECUTABLE(xdmf_mpi_example2 xdmf_mpi_example2.c)
    TARGET_LINK_LIBRARIES(xdmf_mpi_example2 xdmf_writer_par m)

    ADD_PARALLEL_EXECUTABLE(xdmf_mpi_example3 xdmf_mpi_example3.c)
    TARGET_LINK_LIBRARIES(xdmf_mpi_example3 xdmf_writer_par m)

    ADD_PARALLEL_LIBRARY(xdmf_writer_par xdmf_writer.c)
    TARGET_LINK_LIBRARIES(xdmf_writer_par ${HDF5_LIB} ${ZLIB_LIB} m)
ENDIF(VISIT_PARALLEL)
