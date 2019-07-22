// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       ExternalRenderRequestInfo.h                         //
// ************************************************************************* //

#ifndef EXTERNAL_RENDER_REQUEST_INFO_H
#define EXTERNAL_RENDER_REQUEST_INFO_H
#include <viewercore_exports.h>
#include <vector>
#include <string>

#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <AttributeSubject.h>
#include <EngineKey.h>
#include <VisualCueList.h>
#include <WindowAttributes.h>

// ****************************************************************************
//  Datatype: ExternalRenderRequestInfo 
//
//  Purpose: Gather together in one place all the info involved in making an
//  external render request. Permit it to be shared by ViewerWindow and
//  ViewerEngineManager.
//
//  Programmer: Mark C. Miller 
//  Creation:   May 25, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added member for visual cue list
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added member for frame and state info
//
//    Mark C. Miller, Wed Oct  6 18:36:47 PDT 2004
//    Added array of view extents
//
//    Mark C. Miller, Tue Oct 19 19:21:49 PDT 2004
//    Added lastChangedCtName
//
//    Mark C. Miller, Fri Jul 21 08:05:15 PDT 2006
//    Added leftEye;
//
//    Brad Whitlock, Wed Mar 21 22:13:26 PST 2007
//    Added plot names.
//
// ****************************************************************************


struct VIEWERCORE_API ExternalRenderRequestInfo
{
    std::vector<std::string>              plotNames;
    std::vector<const char*>              pluginIDsList;
    std::vector<EngineKey>                engineKeysList;
    std::vector<int>                      plotIdsList;
    std::vector<const AttributeSubject *> attsList;
    WindowAttributes                      winAtts;
    AnnotationAttributes                  annotAtts;
    AnnotationObjectList                  annotObjs;
    std::string                           extStr;
    VisualCueList                         visCues;
    int                                   frameAndState[7];
    double                                viewExtents[6];
    std::string                           lastChangedCtName;
    bool                                  leftEye;
};

#endif
