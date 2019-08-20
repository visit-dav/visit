// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

