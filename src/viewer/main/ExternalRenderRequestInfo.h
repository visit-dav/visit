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
} ExternalRenderRequestInfo;

#endif
