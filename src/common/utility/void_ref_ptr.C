// ************************************************************************* //
//                               void_ref_ptr.C                              //
// ************************************************************************* //

#include <void_ref_ptr.h>


VoidRefList::VoidRefList()
{
    list  = NULL;
    nList = 0;
}

VoidRefList::~VoidRefList()
{
    if (list != NULL)
    {
        delete [] list;
        list = NULL;
    }
}

