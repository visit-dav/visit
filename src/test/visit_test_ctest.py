# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

# ----------------------------------------------------------------------------
#  Method: ctestReport
#       Diff -- reports the image error and diff type, start the sub test, always
#       DiffImages -- reports images for a failed ctest, only failed test
#       MissingBaseline -- reports missing baseline
#       ReportWallTime -- report wall time for the sub test, always
#       ReportCPUTime -- report CPU time for sub test, always
#
#  Programmer: Burlen Loring
#  Date:       2014-01-30 18:27:03
#
#  Modifications:
#
#   Burlen Loring, Fri Oct  2 09:56:26 PDT 2015
#   Make ctest output for each subtest. I added methods to track
#   the elapsed time spent in each subtest
#
# ----------------------------------------------------------------------------

import time

def ctestReportDiff(imdiff, imtype=-1, oss=None):
    """write ctest tag for image difference value"""
    tag = '<DartMeasurement name="ImageError" type="numeric/double"> %f </DartMeasurement>\n'%(imdiff)
    if (imtype <= 0):
        tag += '<DartMeasurement name="BaselineImage" type="text/string">Standard</DartMeasurement>'
    else:
       tag += '<DartMeasurement name="BaselineImage" type="numeric/integer"> %d </DartMeasurement>'%(imtype)
    if oss is not None:
        oss.write(tag)
    return tag

def ctestReportDiffImages(testImage, diffImage, validImage, oss=None):
    """write ctest tags for images of a failed test"""
    tag = '<DartMeasurementFile name="TestImage" type="image/png"> %s </DartMeasurementFile>\n'%(testImage)
    tag += '<DartMeasurementFile name="DifferenceImage" type="image/png"> %s </DartMeasurementFile>\n'%(diffImage)
    tag += '<DartMeasurementFile name="ValidImage" type="image/png"> %s </DartMeasurementFile>'%(validImage)
    if oss is not None:
        oss.write(tag)
    return tag

def ctestReportMissingBaseline(validImage, oss=None):
    """write ctest tags for test failed because of missing baseline"""
    tag = '<DartMeasurement name="ImageNotFound" type="text/string"> %s </DartMeasurement>'%(validImage)
    if oss is not None:
        oss.write(tag)
    return tag

def ctestReportWallTime(wallTime, oss=None):
    """write ctest tag for timing"""
    tag = '<DartMeasurement name="WallTime" type="numeric/double"> %f </DartMeasurement>'%(wallTime)
    if oss is not None:
        oss.write(tag)
    return tag

def ctestReportCPUTime(cpuTime, oss=None):
    """write ctest tag for timing"""
    tag = '<DartMeasurement name="CPUTime" type="numeric/double"> %f </DartMeasurement>'%(cpuTime)
    if oss is not None:
        oss.write(tag)
    return tag

visit_cur_time = time.time()
def ctestInitTestTimer():
    """
    Initialize the base time for elapsed time function
    """
    global visit_cur_time
    visit_cur_time = time.time()
    return

def ctestGetElapsedTime():
    """
    Get the time elapsed since the last call
    """
    global visit_cur_time
    new_time = time.time()
    elapsed_time = new_time - visit_cur_time
    visit_cur_time = new_time
    return elapsed_time
