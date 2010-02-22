/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <PySILRestriction.h>
#include <PySILRestrictionBase.h>
#include <ViewerProxy.h>
#include <VisItException.h>
#include <avtSILRestrictionTraverser.h>

#include <visitmodulehelpers.h>

// ****************************************************************************
//  File: PySILRestriction.C
//
//  Modifications:
//  Cyrus Harrison, Wed Feb 10 10:13:30 PST 2010
//   NOTE: I needed wrap SILRestrictions without any viewer proxy
//         dependancies for contract access in Python Filters. To accomplish
//         this I moved most of the functionality from PySILRestriction.{h,C}
//         into PySILRestrictionBase.{h,C} and left the visitmodule specific
//         pieces in PySILRestriction.{h,C}.
//
// ****************************************************************************

// ****************************************************************************
// Function: NewSILRestrictionObject
//
// Purpose:
//   Helper for constructing a new PySILRestriction object.
//
//
// Programmer: TODO
// Creation:   TODO
//
//
// Modifications:
//   Cyrus Harrison, Wed Feb 10 10:00:33 PST 2010
//   Changed to use PySILRestriction_Wrap.
//
// ****************************************************************************

static PyObject *
NewSILRestrictionObject()
{
    avtSILRestriction_p silr = GetViewerProxy()->GetPlotSILRestriction();
    return (PyObject *)PySILRestriction_Wrap(silr);
}

///////////////////////////////////////////////////////////////////////////////
//
// Interface that is exposed to the VisIt module.
//
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Function: SILRestriction_new
//
// Purpose:
//   Constructor for PySILRestriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
//
// Modifications:
//
//
// ****************************************************************************

PyObject *
SILRestriction_new(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    return (PyObject *)NewSILRestrictionObject();
}

//
// Plugin method table. These methods are added to the visitmodule's methods.
//

static PyMethodDef SILRestrictionMethods[] = {
    {"SILRestriction", SILRestriction_new, METH_VARARGS},
    {NULL,      NULL}        /* Sentinel */
};


// ****************************************************************************
// Function: PySILRestriction_GetMethodTable
//
// Purpose:
//   Provides method table for VisIt module.
//
//
// Programmer: TODO
// Creation:   TODO
//
//
// Modifications:
//
//
// ****************************************************************************

PyMethodDef *
PySILRestriction_GetMethodTable(int *nMethods)
{
    *nMethods = 1;
    return SILRestrictionMethods;
}

// ****************************************************************************
// Function: PySILRestriction_GetMethodTable
//
// Purpose:
//   Creates a new python wrapped sil restriction.
//
//
// Programmer: TODO
// Creation:   TODO
//
//
// Modifications:
//
//
// ****************************************************************************

PyObject *
PySILRestriction_NewPyObject()
{
    return NewSILRestrictionObject();
}

