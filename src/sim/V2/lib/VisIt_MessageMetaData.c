/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_MessageMetaData_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(MessageMetaData_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_MessageMetaData_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(MessageMetaData_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_MessageMetaData_setName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(MessageMetaData_setName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_MessageMetaData_getName(visit_handle h, char **val)
{
    VISIT_DYNAMIC_EXECUTE(MessageMetaData_getName,
        int, (visit_handle, char **),
        (h, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITMESSAGEMETADATAALLOC       F77_ID(visitmessagemetadataalloc_,visitmessagemetadataalloc,VISITMESSAGEMETADATAALLOC)
#define F_VISITMESSAGEMETADATAFREE        F77_ID(visitmessagemetadatafree_,visitmessagemetadatafree,VISITMESSAGEMETADATAFREE)
#define F_VISITMESSAGEMETADATASETNAME     F77_ID(visitmessagemetadatasetname_,visitmessagemetadatasetname, VISITMESSAGEMETADATASETNAME)
#define F_VISITMESSAGEMETADATAGETNAME     F77_ID(visitmessagemetadatagetname_,visitmessagemetadatagetname, VISITMESSAGEMETADATAGETNAME)

int
F_VISITMESSAGEMETADATAALLOC(visit_handle *h)
{
    return VisIt_MessageMetaData_alloc(h);
}

int
F_VISITMESSAGEMETADATAFREE(visit_handle *h)
{
    return VisIt_MessageMetaData_free(*h);
}

int
F_VISITMESSAGEMETADATASETNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_MessageMetaData_setName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITMESSAGEMETADATAGETNAME(visit_handle *h, char *val, int *lval)
{
    char *s = NULL;
    int retval = VisIt_MessageMetaData_getName(*h, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

