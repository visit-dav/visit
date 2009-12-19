#*****************************************************************************
#
# Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
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

SET(METADATA_SOURCES
MetaData/avtArrayMetaData.C
MetaData/avtBaseVarMetaData.C
MetaData/avtCurveMetaData.C
MetaData/avtDatabaseMetaData.C
MetaData/avtDefaultPlotMetaData.C
MetaData/avtExpressionTypeConversions.C
MetaData/avtLabelMetaData.C
MetaData/avtMatSpeciesMetaData.C
MetaData/avtMaterialMetaData.C
MetaData/avtMeshMetaData.C
MetaData/avtScalarMetaData.C
MetaData/avtSimulationCommandSpecification.C
MetaData/avtSimulationInformation.C
MetaData/avtSpeciesMetaData.C
MetaData/avtSubsetsMetaData.C
MetaData/avtSymmetricTensorMetaData.C
MetaData/avtTensorMetaData.C
MetaData/avtTypes.C
MetaData/avtVarMetaData.C
MetaData/avtVectorMetaData.C
)

SET(SIL_SOURCES
SIL/avtSIL.C 
SIL/avtSILArray.C 
SIL/avtSILCollection.C 
SIL/avtSILEnumeratedNamespace.C 
SIL/avtSILMatrix.C 
SIL/avtSILNamespace.C 
SIL/avtSILRangeNamespace.C 
SIL/avtSILRestriction.C 
SIL/avtSILRestrictionTraverser.C 
SIL/avtSILSet.C
)

INCLUDE_DIRECTORIES(
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
)

# Add link directories needed.
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})

ADD_LIBRARY(avtdbatts ${METADATA_SOURCES} ${SIL_SOURCES})

TARGET_LINK_LIBRARIES(avtdbatts visitcommon)

VISIT_INSTALL_TARGETS(avtdbatts)
