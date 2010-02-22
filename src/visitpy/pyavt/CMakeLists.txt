#*****************************************************************************
#
# Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

# custom install the pyavt module to ${LIBRARY_OUTPUT_DIRECTORY}/site-packages/pyavt

CONFIGURE_FILE(src/__init__.py
               ${LIBRARY_OUTPUT_DIRECTORY}/site-packages/pyavt/__init__.py @ONLY)
CONFIGURE_FILE(src/filters.py
               ${LIBRARY_OUTPUT_DIRECTORY}/site-packages/pyavt/filters.py @ONLY)

# custom install templates to ${LIBRARY_OUTPUT_DIRECTORY}/site-packages/pyavt-templates/
CONFIGURE_FILE(templates/simple_expression.py
       ${LIBRARY_OUTPUT_DIRECTORY}/site-packages/pyavt-templates/simple_expression.py @ONLY)
CONFIGURE_FILE(templates/advanced_expression.py
       ${LIBRARY_OUTPUT_DIRECTORY}/site-packages/pyavt-templates/advanced_expression.py @ONLY)

CONFIGURE_FILE(templates/simple_query.py
       ${LIBRARY_OUTPUT_DIRECTORY}/site-packages/pyavt-templates/simple_query.py @ONLY)
CONFIGURE_FILE(templates/advanced_query.py
       ${LIBRARY_OUTPUT_DIRECTORY}/site-packages/pyavt-templates/advanced_query.py @ONLY)

INSTALL(FILES src/__init__.py src/filters.py
              DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/pyavt/
              PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
)

INSTALL(FILES templates/simple_expression.py templates/advanced_expression.py
              templates/simple_query.py templates/advanced_query.py
              DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/pyavt-templates/
              PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
)


