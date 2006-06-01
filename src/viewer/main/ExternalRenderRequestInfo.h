/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
//
//    Mark C. Miller, Tue Oct 19 19:21:49 PDT 2004
//    Added lastChangedCtName
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
    string                                lastChangedCtName;
} ExternalRenderRequestInfo;

#endif
