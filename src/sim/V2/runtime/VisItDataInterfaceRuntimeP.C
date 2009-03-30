#include <stdio.h>
#include <string.h>

#include <VisItDataInterfaceRuntimeP.h>

static void **visit_pointers = NULL;
static int  visit_pointers_size = 0;

void *
VisItGetPointer(visit_handle h)
{
    void *retval = NULL;
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
VisItStorePointer(void *ptr)
{
    /* Allocate the pointers array if we haven't already */
    if(visit_pointers == NULL)
    {
        visit_pointers_size = 100;
        visit_pointers = (void **)(void*)calloc(visit_pointers_size, sizeof(void *));
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
    void **new_visit_pointers = (void **)(void*)calloc(visit_pointers_size*2, sizeof(void *));
    memcpy(new_visit_pointers, visit_pointers, visit_pointers_size*sizeof(void*));
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
