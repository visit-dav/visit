#*****************************************************************************
#
# Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
#*****************************************************************************

# helper functions for generating ctest compatible results
# these are all of the tags I am aware of

def ctestReportDiff(imdiff, imtype=-1, os=None):
    """write ctest tag for image difference value"""
    tag = '<DartMeasurement name="ImageError" type="numeric/double"> %f </DartMeasurement>\n'%(imdiff)
    if (imtype <= 0):
        tag += '<DartMeasurement name="BaselineImage" type="text/string">Standard</DartMeasurement>'
    else:
       tag += '<DartMeasurement name="BaselineImage" type="numeric/integer"> %d </DartMeasurement>'%(imtype)
    if os is not None:
        os.write(tag)
    return tag

def ctestReportDiffImages(testImage, diffImage, validImage, os=None):
    """write ctest tags for images of a failed test"""
    tag = '<DartMeasurementFile name="TestImage" type="image/png"> %s </DartMeasurementFile>\n'%(testImage)
    tag += '<DartMeasurementFile name="DifferenceImage" type="image/png"> %s </DartMeasurementFile>\n'%(diffImage)
    tag += '<DartMeasurementFile name="ValidImage" type="image/png"> %s </DartMeasurementFile>'%(validImage)
    if os is not None:
        os.write(tag)
    return tag

def ctestReportMissingBaseline(validImage, os=None):
    """write ctest tags for test failed because of missing baseline"""
    tag = '<DartMeasurement name="ImageNotFound" type="text/string"> %s </DartMeasurement>'%(validImage)
    if os is not None:
        os.write(tag)
    return tag

def ctestReportWallTime(wallTime, os=None):
    """write ctest tag for timing"""
    tag = '<DartMeasurement name="WallTime" type="numeric/double"> %f </DartMeasurement>'%(wallTime)
    if os is not None:
        os.write(tag)
    return tag

def ctestReportCPUTime(cpuTime, os=None):
    """write ctest tag for timing"""
    tag = '<DartMeasurement name="CPUTime" type="numeric/double"> %f </DartMeasurement>'%(cpuTime)
    if os is not None:
        os.write(tag)
    return tag
