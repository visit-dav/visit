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
#   Hank Childs, Sat Aug 21 11:53:13 PDT 2010
#   Rename directory DDF to DataBinning.
#
#   Brad Whitlock, Thu Jan 20 08:48:38 PST 2011
#   Do not build QtVisWindow on engine-only or server-components-only builds.
#
#****************************************************************************/

IF(VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(DBAtts)
    ADD_SUBDIRECTORY(Pipeline)
    ADD_SUBDIRECTORY(Database)
ELSE(VISIT_DBIO_ONLY)
    ADD_SUBDIRECTORY(DBAtts)
    ADD_SUBDIRECTORY(Math)
    ADD_SUBDIRECTORY(IVP)
    ADD_SUBDIRECTORY(Pipeline)
    ADD_SUBDIRECTORY(DataBinning)
    ADD_SUBDIRECTORY(MIR)
    ADD_SUBDIRECTORY(Shapelets)
    ADD_SUBDIRECTORY(View)
    ADD_SUBDIRECTORY(Filters)
    ADD_SUBDIRECTORY(Database)
    ADD_SUBDIRECTORY(Plotter)
    ADD_SUBDIRECTORY(Expressions)
    ADD_SUBDIRECTORY(Queries)
    ADD_SUBDIRECTORY(FileWriter)
    ADD_SUBDIRECTORY(VisWindow)
    IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY)
        ADD_SUBDIRECTORY(QtVisWindow)
    ENDIF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY)
    IF(VISIT_PYTHON_FILTERS)
        ADD_SUBDIRECTORY(PythonFilters)
    ENDIF(VISIT_PYTHON_FILTERS)
ENDIF(VISIT_DBIO_ONLY)
