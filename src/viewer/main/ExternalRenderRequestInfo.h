// ************************************************************************* //
//                       ExternalRenderRequestInfo.h                         //
// ************************************************************************* //

#ifndef EXTERNAL_RENDER_REQUEST_INFO_H
#define EXTERNAL_RENDER_REQUEST_INFO_H

#include <vector>
#include <string>

#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <AttributeSubject.h>
#include <EngineKey.h>
#include <VisualCueList.h>
#include <WindowAttributes.h>

using std::vector;
using std::string;

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
// ****************************************************************************


typedef struct _ExternalRenderRequestInfo
{
    vector<const char*>                   pluginIDsList;
    vector<EngineKey>                     engineKeysList;
    vector<int>                           plotIdsList;
    vector<const AttributeSubject *>      attsList;
    WindowAttributes                      winAtts;
    AnnotationAttributes                  annotAtts;
    AnnotationObjectList                  annotObjs;
    string                                extStr;
    VisualCueList                         visCues;
    int                                   frameAndState[7];
    double                                viewExtents[6];
} ExternalRenderRequestInfo;

#endif
