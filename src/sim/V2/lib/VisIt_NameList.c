/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
VisIt_NameList_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(NameList_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_NameList_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(NameList_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_NameList_addName(visit_handle h, const char *val)
{
    VISIT_DYNAMIC_EXECUTE(NameList_addName,
        int, (visit_handle, const char *),
        (h, val));
}

int
VisIt_NameList_getNumName(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(NameList_getNumName,
        int, (visit_handle, int *),
        (h, val));
}

int
VisIt_NameList_getName(visit_handle h, int i, char **val)
{
    VISIT_DYNAMIC_EXECUTE(NameList_getName,
        int, (visit_handle, int, char **),
        (h, i, val));
}


/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITNAMELISTALLOC              F77_ID(visitnamelistalloc_,visitnamelistalloc,VISITNAMELISTALLOC)
#define F_VISITNAMELISTFREE               F77_ID(visitnamelistfree_,visitnamelistfree,VISITNAMELISTFREE)
#define F_VISITNAMELISTADDNAME            F77_ID(visitnamelistaddname_,visitnamelistaddname, VISITNAMELISTADDNAME)
#define F_VISITNAMELISTGETNUMNAME         F77_ID(visitnamelistgetnumname_,visitnamelistgetnumname, VISITNAMELISTGETNUMNAME)
#define F_VISITNAMELISTGETNAME            F77_ID(visitnamelistgetname_,visitnamelistgetname, VISITNAMELISTGETNAME)

int
F_VISITNAMELISTALLOC(visit_handle *h)
{
    return VisIt_NameList_alloc(h);
}

int
F_VISITNAMELISTFREE(visit_handle *h)
{
    return VisIt_NameList_free(*h);
}

int
F_VISITNAMELISTADDNAME(visit_handle *h, const char *val, int *lval)
{
    char *f_val = NULL;
    int retval;
    COPY_FORTRAN_STRING(f_val, val, lval);
    retval = VisIt_NameList_addName(*h, f_val);
    FREE(f_val);
    return retval;
}

int
F_VISITNAMELISTGETNUMNAME(visit_handle *h, int *val)
{
    return VisIt_NameList_getNumName(*h, val);
}

int
F_VISITNAMELISTGETNAME(visit_handle *h, int *i, char *val, int *lval)
{
    char *s = NULL;
    int retval;
    retval = VisIt_NameList_getName(*h, *i, &s);
    if(s != NULL)
    {
        visit_cstring_to_fstring(s, val, *lval);
        free(s);
    }
    return retval;
}

