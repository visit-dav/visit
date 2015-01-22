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
#****************************************************************************/

# Use the BOOST_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET(BOOST_LIBS NO_LIBS)

IF(NEKTAR++_FOUND)
  SET(BOOST_LIBS
   boost_iostreams
   boost_thread
   boost_date_time
   boost_filesystem
   boost_system
   boost_program_options)
ENDIF()

SET_UP_THIRD_PARTY(BOOST lib include ${BOOST_LIBS} )

# Notes to Windows developers on building boost:
# grab the .zip or .7z tarball and extract
# Open command prompt in the extracted boost_<version> directory
# To build everything and install to default C:\Boost location:
#   .\bootstrap
#   .\b2
#   .\b2 install
#
# To change install location, add --prefix="\path\to\boost" to
# all commands. (All might be overkill, but I experienced problems
# when specified for only bootrap or b2, so I added it to all).
#
# If you want shared libs only, linked with shared CRT, release only, 64-bit:
#
#   .\boostrap --prefix="C:\path\to\where\you\want\boost"
#   .\b2 --prefix="C:\path\to\where\you\want\boost" link=shared runtime-link=shared variant=release threading=multi address-model=64
#   .\b2 --prefix="C:\path\to\where\you\want\boost" link=shared runtime-link=shared variant=release threading=multi address-model=64 install
#
# If you only want a subset of the libraries add a '--with-<lib>' for each 
# library you want:
#   .\boostrap --prefix="C:\path\to\where\you\want\boost"
#   .\b2 --with-system --prefix="C:\path\to\where\you\want\boost" link=shared runtime-link=shared variant=release threading=multi address-model=64
#   .\b2 --with-system --prefix="C:\path\to\where\you\want\boost" link=shared runtime-link=shared variant=release threading=multi address-model=64 install
#
# Still not certain that all the arguments are needed for the 'install' step
# of running b2, but I ran into problems without using them, so ...
#
# I found the following links helpful, as well as running '.\b2 --help'
# once I had bootstrapped.
#
# http://www.boost.org/doc/libs/1_57_0/more/getting_started/windows.html#simplified-build-from-source
# 
# http://www.boost.org/build/doc/html/bbv2/overview/invocation.html
