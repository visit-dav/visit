#include <VisItDataInterfaceRuntimeP.h>

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>

VisIt_ObjectBase::VisIt_ObjectBase(int t) : object_type(t)
{
}

VisIt_ObjectBase::~VisIt_ObjectBase()
{
}

int
VisIt_ObjectBase::objectType() const
{
    return object_type;
}



namespace {

typedef std::vector<VisIt_ObjectBase*> VisItPointerVec;
VisItPointerVec visit_pointers;

};

VisIt_ObjectBase *
VisItGetPointer(visit_handle h)
{
    VisIt_ObjectBase *retval = NULL;
    size_t i = static_cast<size_t>(h);
    if (i < visit_pointers.size())
    {
        retval = visit_pointers[i];
    }
    return retval;
}

void
VisItFreePointer(visit_handle h)
{
    size_t i = static_cast<size_t>(h);
    if (i < visit_pointers.size())
    {
        visit_pointers[i] = NULL;
    }
}

visit_handle
VisItStorePointer(VisIt_ObjectBase *ptr)
{
    visit_handle loc = VISIT_INVALID_HANDLE;

    /* Look for a free slot in the pointers array
     * if not just append the pointer at the end */
    VisItPointerVec::iterator vstart = visit_pointers.begin();
    VisItPointerVec::iterator vend = visit_pointers.end();
    VisItPointerVec::iterator vslot = std::find(vstart, vend, static_cast<VisIt_ObjectBase*>(0));
    if (vslot != vend)
    {
        *vslot = ptr;
        loc = static_cast<visit_handle>(vslot-vstart);
    }
    else
    {
        loc = static_cast<visit_handle>(visit_pointers.size());
        visit_pointers.push_back(ptr);
    }

    return loc;
}

void
VisItError(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}
