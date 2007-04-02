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

