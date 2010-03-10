#include <stdio.h>
#include <string.h>

#include <VisItDataInterfaceRuntimeP.h>

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

static VisIt_ObjectBase **visit_pointers = NULL;
static int  visit_pointers_size = 0;

VisIt_ObjectBase *
VisItGetPointer(visit_handle h)
{
    VisIt_ObjectBase *retval = NULL;
    if(visit_pointers != NULL && h >= 0 && h < visit_pointers_size)
        retval = visit_pointers[h];
    return retval;
}

void
VisItFreePointer(visit_handle h)
{
    if(visit_pointers != NULL && h >= 0 && h < visit_pointers_size)
        visit_pointers[h] = NULL;
}

visit_handle
VisItStorePointer(VisIt_ObjectBase *ptr)
{
    /* Allocate the pointers array if we haven't already */
    if(visit_pointers == NULL)
    {
        visit_pointers_size = 100;
        visit_pointers = (VisIt_ObjectBase **)(VisIt_ObjectBase*)calloc(visit_pointers_size, sizeof(VisIt_ObjectBase *));
    }

    /* Look for a free slot in the pointers array */
    for(int i = 0; i < visit_pointers_size; ++i)
    {
        if(visit_pointers[i] == NULL)
        {
            visit_pointers[i] = ptr;
            return i;
        }
    }

    /* Grow the pointers array. */
    VisIt_ObjectBase **new_visit_pointers = (VisIt_ObjectBase **)(VisIt_ObjectBase*)calloc(visit_pointers_size*2, sizeof(VisIt_ObjectBase *));
    memcpy(new_visit_pointers, visit_pointers, visit_pointers_size*sizeof(VisIt_ObjectBase*));
    free(visit_pointers);
    visit_pointers = new_visit_pointers;
    int h = visit_pointers_size;
    visit_pointers_size = visit_pointers_size*2;

    /* Store the pointer. */
    visit_pointers[h] = ptr;
    return h;    
}

void
VisItError(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
}
