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

#include <ViewerState.h>
#include <DebugStream.h>

ViewerState::ViewerState() : objVector(), partialSendVector()
{
    ownsObjects = false;
}

ViewerState::ViewerState(const ViewerState &vs) : objVector(),
    partialSendVector()
{
    ownsObjects = true;

    debug1 << "ViewerState::ViewerState(const ViewerState &)" << endl;
    for(int i = 0; i < vs.objVector.size(); ++i)
    {
        const AttributeSubject *obj = (const AttributeSubject *)vs.objVector[i];
        bool partialOkay = (vs.partialSendVector[i] == 1);
        AttributeSubject *newObj = obj->NewInstance(true);

debug1 << "\tobject[" << i << "] = " << (void*)newObj;
if(newObj != 0)
    debug1 << "  type=" << newObj->TypeName().c_str() << endl;
else
    debug1 << endl;

        Add(newObj, partialOkay);
    }    
}

ViewerState::~ViewerState()
{
    if(ownsObjects)
    {
        for(int i = 0; i < objVector.size(); ++i)
        {
            AttributeSubject *obj = (AttributeSubject *)objVector[i];
            delete obj;
        }
    }
}

void
ViewerState::Add(AttributeSubject *obj, bool partialSendOkay)
{
    objVector.push_back(obj);
    partialSendVector.push_back(partialSendOkay ? 1 : 0);
}

AttributeSubject *
ViewerState::GetStateObject(int i)
{
    return (i >= 0 && i < objVector.size()) ?
           ((AttributeSubject *)objVector[i]) : 0;
}

const AttributeSubject *
ViewerState::GetStateObject(int i) const
{
    return (i >= 0 && i < objVector.size()) ?
           ((const AttributeSubject *)objVector[i]) : 0;
}

bool
ViewerState::GetPartialSendFlag(int i) const
{
    return (i >= 0 && i < partialSendVector.size()) ?
           (partialSendVector[i]==1) : false;
}

int
ViewerState::GetNObjects() const
{
    return objVector.size();
}

