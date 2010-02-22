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

#include <Python.h>
#include <PyDataRequest.h>
#include <PySILRequest.h>
#include <PySILRestrictionBase.h>
#include <PyDataSelection.h>
#include <PythonInterpreter.h>
#include <avtGhostData.h>

// ****************************************************************************
//  Modifications:
//
// ****************************************************************************
struct PyDataRequestObject
{
    PyObject_HEAD
    avtDataRequest_p *request;
};


//
// avtDataRequest methods
//


//
// Timestep
//


// ****************************************************************************
// Function: DataRequest_GetTimestep
//
// Purpose:
//   Wrap avtDataRequest::GetTimestep.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetTimestep(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    return Py_BuildValue("l",request->GetTimestep());
}

// ****************************************************************************
// Function: DataRequest_SetTimestep
//
// Purpose:
//   Wrap avtDataRequest::SetTimestep.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetTimestep(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    int val;
    if(!PyArg_ParseTuple(args, "l", &val))
        return NULL;

    request->SetTimestep(val);
    Py_RETURN_NONE;
}

//
// Variable
//

// ****************************************************************************
// Function: DataRequest_GetVariable
//
// Purpose:
//   Wrap avtDataRequest::GetVariable.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetVariable(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    return Py_BuildValue("s",request->GetVariable());
}

//
// SIL Related
//

// ****************************************************************************
// Function: DataRequest_GetSIL
//
// Purpose:
//   Wrap avtDataRequest::GetSIL.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetSIL(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    return PySILRequest_Wrap(&request->GetSIL());
}

// ****************************************************************************
// Function: DataRequest_GetRestriction
//
// Purpose:
//   Wrap avtDataRequest::GetRestriction.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetRestriction(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    return PySILRestriction_Wrap(request->GetRestriction());
}


//
// Original Variable
//

// ****************************************************************************
// Function: DataRequest_GetOriginalVariable
//
// Purpose:
//   Wrap avtDataRequest::GetOriginalVariable.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetOriginalVariable(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    return Py_BuildValue("s",request->GetOriginalVariable());
}

// ****************************************************************************
// Function: DataRequest_SetOriginalVariable
//
// Purpose:
//   Wrap avtDataRequest::SetOriginalVariable.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetOriginalVariable(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    char *var;
    if(!PyArg_ParseTuple(args, "s", &var))
        return NULL;
    request->SetOriginalVariable(var);
    Py_RETURN_NONE;
}

//
// Secondary Variables
//

// ****************************************************************************
// Function: DataRequest_AddSecondaryVariable
//
// Purpose:
//   Wrap avtDataRequest::AddSecondaryVariable.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_AddSecondaryVariable(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    char *var;
    if(!PyArg_ParseTuple(args, "s", &var))
        return NULL;
    request->AddSecondaryVariable(var);
    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_RemoveSecondaryVariable
//
// Purpose:
//   Wrap avtDataRequest::RemoveSecondaryVariable.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_RemoveSecondaryVariable(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    char *var;
    if(!PyArg_ParseTuple(args, "s", &var))
        return NULL;
    request->RemoveSecondaryVariable(var);
    Py_RETURN_NONE;
}


// ****************************************************************************
// Function: DataRequest_RemoveAllSecondaryVariables
//
// Purpose:
//   Wrap avtDataRequest::RemoveAllSecondaryVariables.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_RemoveAllSecondaryVariables(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->RemoveAllSecondaryVariables();
    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_HasSecondaryVariable
//
// Purpose:
//   Wrap avtDataRequest::HasSecondaryVariable.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_HasSecondaryVariable(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    char *var;
    if(!PyArg_ParseTuple(args, "s", &var))
        return NULL;

    if(request->HasSecondaryVariable(var))
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_GetSecondaryVariables
//
// Purpose:
//   Wrap avtDataRequest::GetSecondaryVariables.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetSecondaryVariables(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    const std::vector<CharStrRef> &vars = request->GetSecondaryVariables();
    int nvars = vars.size();
    PyObject *res = PyTuple_New(nvars);

    for(int i = 0; i < nvars; ++i)
    {
        PyObject *py_var = PyString_FromString(*(vars[i]));
        if(py_var == NULL)
            continue;
        PyTuple_SET_ITEM(res, i, py_var);
    }

    return res;
}

// ****************************************************************************
// Function: DataRequest_GetSecondaryVariablesWithoutDuplicates
//
// Purpose:
//   Wrap avtDataRequest::GetSecondaryVariablesWithoutDuplicates.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetSecondaryVariablesWithoutDuplicates(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    std::vector<CharStrRef> vars = request->GetSecondaryVariablesWithoutDuplicates();
    int nvars = vars.size();
    PyObject *res = PyTuple_New(nvars);

    for(int i = 0; i < nvars; ++i)
    {
        PyObject *py_var = PyString_FromString(*(vars[i]));
        if(py_var == NULL)
            continue;
        PyTuple_SET_ITEM(res, i, py_var);
    }
    return res;
}

//
// MIR
//

// ****************************************************************************
// Function: DataRequest_MustDoMaterialInterfaceReconstruction
//
// Purpose:
//   Wrap avtDataRequest::MustDoMaterialInterfaceReconstruction.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_MustDoMaterialInterfaceReconstruction(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->MustDoMaterialInterfaceReconstruction())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_ForceMaterialInterfaceReconstructionOn
//
// Purpose:
//   Wrap avtDataRequest::ForceMaterialInterfaceReconstructionOn.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_ForceMaterialInterfaceReconstructionOn(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->ForceMaterialInterfaceReconstructionOn();

    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_ForceMaterialInterfaceReconstructionOff
//
// Purpose:
//   Wrap avtDataRequest::ForceMaterialInterfaceReconstructionOff.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_ForceMaterialInterfaceReconstructionOff(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->ForceMaterialInterfaceReconstructionOff();

    Py_RETURN_NONE;
}

//
// Internal Surfaces
//

// ****************************************************************************
// Function: DataRequest_NeedInternalSurfaces
//
// Purpose:
//   Wrap avtDataRequest::NeedInternalSurfaces.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedInternalSurfaces(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedInternalSurfaces())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_TurnInternalSurfacesOn
//
// Purpose:
//   Wrap avtDataRequest::TurnInternalSurfacesOn.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnInternalSurfacesOn(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnInternalSurfacesOn();

    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_TurnInternalSurfacesOff
//
// Purpose:
//   Wrap avtDataRequest::TurnInternalSurfacesOff.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnInternalSurfacesOff(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnInternalSurfacesOff();

    Py_RETURN_NONE;
}

//
// Boundary Surface Representation
//

// ****************************************************************************
// Function: DataRequest_GetBoundarySurfaceRepresentation
//
// Purpose:
//   Wrap avtDataRequest::GetBoundarySurfaceRepresentation.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetBoundarySurfaceRepresentation(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->GetBoundarySurfaceRepresentation())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_TurnBoundarySurfaceRepresentationOn
//
// Purpose:
//   Wrap avtDataRequest::TurnBoundarySurfaceRepresentationOn.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnBoundarySurfaceRepresentationOn(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnBoundarySurfaceRepresentationOn();

    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_TurnBoundarySurfaceRepresentationOff
//
// Purpose:
//   Wrap avtDataRequest::TurnBoundarySurfaceRepresentationOff.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnBoundarySurfaceRepresentationOff(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnBoundarySurfaceRepresentationOff();

    Py_RETURN_NONE;
}


//
// Simplified Nesting Representation
//

// ****************************************************************************
// Function: DataRequest_GetSimplifiedNestingRepresentation
//
// Purpose:
//   Wrap avtDataRequest::GetSimplifiedNestingRepresentation.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetSimplifiedNestingRepresentation(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->GetSimplifiedNestingRepresentation())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_TurnSimplifiedNestingRepresentationOn
//
// Purpose:
//   Wrap avtDataRequest::TurnSimplifiedNestingRepresentationOn.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnSimplifiedNestingRepresentationOn(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnSimplifiedNestingRepresentationOn();

    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_TurnSimplifiedNestingRepresentationOff
//
// Purpose:
//   Wrap avtDataRequest::TurnSimplifiedNestingRepresentationOff.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnSimplifiedNestingRepresentationOff(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnSimplifiedNestingRepresentationOff();

    Py_RETURN_NONE;
}

//
// Structured Indices
//

// ****************************************************************************
// Function: DataRequest_NeedStructuredIndices
//
// Purpose:
//   Wrap avtDataRequest::NeedStructuredIndices.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedStructuredIndices(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedStructuredIndices())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}


// ****************************************************************************
// Function: DataRequest_SetNeedStructuredIndices
//
// Purpose:
//   Wrap avtDataRequest::SetNeedStructuredIndices.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetNeedStructuredIndices(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetNeedStructuredIndices(val);
    Py_RETURN_NONE;
}

//
// AMR Indices
//

// ****************************************************************************
// Function: DataRequest_NeedAMRIndices
//
// Purpose:
//   Wrap avtDataRequest::NeedAMRIndices.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedAMRIndices(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    int val = request->NeedAMRIndices();
    return PyInt_FromLong(val);
}


// ****************************************************************************
// Function: DataRequest_SetNeedAMRIndices
//
// Purpose:
//   Wrap avtDataRequest::SetNeedAMRIndices.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetNeedAMRIndices(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    int val;
    if(!PyArg_ParseTuple(args, "l", &val))
        return NULL;

    request->SetNeedAMRIndices(val);

    Py_RETURN_NONE;
}

//
// Zone Numbers
//

// ****************************************************************************
// Function: DataRequest_NeedZoneNumbers
//
// Purpose:
//   Wrap avtDataRequest::NeedZoneNumbers.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedZoneNumbers(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedZoneNumbers())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_TurnZoneNumbersOn
//
// Purpose:
//   Wrap avtDataRequest::TurnZoneNumbersOn.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnZoneNumbersOn(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnZoneNumbersOn();

    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_TurnZoneNumbersOff
//
// Purpose:
//   Wrap avtDataRequest::TurnZoneNumbersOff.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnZoneNumbersOff(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnZoneNumbersOff();

    Py_RETURN_NONE;
}

//
// Node Numbers
//

// ****************************************************************************
// Function: DataRequest_NeedNodeNumbers
//
// Purpose:
//   Wrap avtDataRequest::NeedNodeNumbers.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedNodeNumbers(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedNodeNumbers())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_TurnNodeNumbersOn
//
// Purpose:
//   Wrap avtDataRequest::TurnNodeNumbersOn.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnNodeNumbersOn(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnNodeNumbersOn();

    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_TurnNodeNumbersOff
//
// Purpose:
//   Wrap avtDataRequest::TurnNodeNumbersOff.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnNodeNumbersOff(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnNodeNumbersOff();

    Py_RETURN_NONE;
}

//
// Global Zone Numbers
//

// ****************************************************************************
// Function: DataRequest_NeedGlobalZoneNumbers
//
// Purpose:
//   Wrap avtDataRequest::NeedGlobalZoneNumbers.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedGlobalZoneNumbers(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedGlobalZoneNumbers())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_TurnGlobalZoneNumbersOn
//
// Purpose:
//   Wrap avtDataRequest::TurnGlobalZoneNumbersOn.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnGlobalZoneNumbersOn(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnGlobalZoneNumbersOn();

    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_TurnGlobalZoneNumbersOff
//
// Purpose:
//   Wrap avtDataRequest::TurnGlobalZoneNumbersOff.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnGlobalZoneNumbersOff(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnGlobalZoneNumbersOff();

    Py_RETURN_NONE;
}

//
// Global Node Numbers
//

// ****************************************************************************
// Function: DataRequest_NeedGlobalNodeNumbers
//
// Purpose:
//   Wrap avtDataRequest::NeedGlobalNodeNumbers.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedGlobalNodeNumbers(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedGlobalNodeNumbers())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_TurnGlobalNodeNumbersOn
//
// Purpose:
//   Wrap avtDataRequest::TurnGlobalNodeNumbersOn.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnGlobalNodeNumbersOn(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnGlobalNodeNumbersOn();

    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_TurnGlobalNodeNumbersOff
//
// Purpose:
//   Wrap avtDataRequest::TurnGlobalNodeNumbersOff.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TurnGlobalNodeNumbersOff(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->TurnGlobalNodeNumbersOff();

    Py_RETURN_NONE;
}

//
// Require Zones
//

// ****************************************************************************
// Function: DataRequest_MayRequireZones
//
// Purpose:
//   Wrap avtDataRequest::MayRequireZones.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_MayRequireZones(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->MayRequireZones())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetMayRequireZones
//
// Purpose:
//   Wrap avtDataRequest::SetMayRequireZones.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetMayRequireZones(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetMayRequireZones(val);
    Py_RETURN_NONE;
}


//
// Require Nodes
//

// ****************************************************************************
// Function: DataRequest_MayRequireNodes
//
// Purpose:
//   Wrap avtDataRequest::MayRequireNodes.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_MayRequireNodes(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->MayRequireNodes())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetMayRequireNodes
//
// Purpose:
//   Wrap avtDataRequest::SetMayRequireNodes.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetMayRequireNodes(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetMayRequireNodes(val);
    Py_RETURN_NONE;
}

//
// Maintain Original Connectivity
//

// ****************************************************************************
// Function: DataRequest_MustMaintainOriginalConnectivity
//
// Purpose:
//   Wrap avtDataRequest::MustMaintainOriginalConnectivity.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_MustMaintainOriginalConnectivity(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->MustMaintainOriginalConnectivity())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetMaintainOriginalConnectivity
//
// Purpose:
//   Wrap avtDataRequest::SetMaintainOriginalConnectivity.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetMaintainOriginalConnectivity(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetMaintainOriginalConnectivity(val);
    Py_RETURN_NONE;
}

//
// Ghost Data Type
//

// ****************************************************************************
// Function: DataRequest_SetDesiredGhostDataType
//
// Purpose:
//   Wrap avtDataRequest::SetDesiredGhostDataType.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetDesiredGhostDataType(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    int val = -1;
    PyObject *py_obj;
    if(!PyArg_ParseTuple(args, "o", &py_obj))
        return NULL;

    if(PyInt_Check(py_obj))
    {
        val = (int)PyInt_AS_LONG(py_obj);
    }
    else if(PyString_Check(py_obj))
    {
        string sval(PyString_AsString(py_obj));
        if(sval == "NO_GHOST_DATA")
            val = NO_GHOST_DATA;
        else if(sval == "GHOST_NODE_DATA")
            val = GHOST_NODE_DATA;
        else if(sval == "GHOST_ZONE_DATA")
            val = GHOST_ZONE_DATA;
    }

    if(val >= 0)
        request->SetDesiredGhostDataType((avtGhostDataType)val);
    else
        return NULL; // bad call
    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_GetDesiredGhostDataType
//
// Purpose:
//   Wrap avtDataRequest::GetDesiredGhostDataType.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetDesiredGhostDataType(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    int val = (int)request->GetDesiredGhostDataType();
    if(val == NO_GHOST_DATA)
        return PyString_FromString("NO_GHOST_DATA");
    else if(val == GHOST_NODE_DATA)
        return PyString_FromString("GHOST_NODE_DATA");
    else if(val == GHOST_ZONE_DATA)
        return PyString_FromString("GHOST_ZONE_DATA");

    return NULL; // bad call
}

//
// Valid Face Connectivity
//

// ****************************************************************************
// Function: DataRequest_NeedValidFaceConnectivity
//
// Purpose:
//   Wrap avtDataRequest::NeedValidFaceConnectivity.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedValidFaceConnectivity(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedValidFaceConnectivity())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetNeedValidFaceConnectivity
//
// Purpose:
//   Wrap avtDataRequest::SetNeedValidFaceConnectivity.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetNeedValidFaceConnectivity(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetNeedValidFaceConnectivity(val);
    Py_RETURN_NONE;
}

//
// Uses All Domains
//

// ****************************************************************************
// Function: DataRequest_UsesAllDomains
//
// Purpose:
//   Wrap avtDataRequest::UsesAllDomains.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_UsesAllDomains(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->UsesAllDomains())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetUsesAllDomains
//
// Purpose:
//   Wrap avtDataRequest::SetUsesAllDomains.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetUsesAllDomains(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetUsesAllDomains(val);
    Py_RETURN_NONE;
}

//
// mixed variable reconstruction
//

// ****************************************************************************
// Function: DataRequest_NeedMixedVariableReconstruction
//
// Purpose:
//   Wrap avtDataRequest::NeedMixedVariableReconstruction.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedMixedVariableReconstruction(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedMixedVariableReconstruction())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetNeedMixedVariableReconstruction
//
// Purpose:
//   Wrap avtDataRequest::SetNeedMixedVariableReconstruction.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetNeedMixedVariableReconstruction(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetNeedMixedVariableReconstruction(val);
    Py_RETURN_NONE;
}

//
// smooth material interfaces
//

// ****************************************************************************
// Function: DataRequest_NeedSmoothMaterialInterfaces
//
// Purpose:
//   Wrap avtDataRequest::NeedSmoothMaterialInterfaces.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedSmoothMaterialInterfaces(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedSmoothMaterialInterfaces())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetNeedSmoothMaterialInterfaces
//
// Purpose:
//   Wrap avtDataRequest::SetNeedSmoothMaterialInterfaces.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetNeedSmoothMaterialInterfaces(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetNeedSmoothMaterialInterfaces(val);
    Py_RETURN_NONE;
}


//
// clean zones only
//

// ****************************************************************************
// Function: DataRequest_NeedCleanZonesOnly
//
// Purpose:
//   Wrap avtDataRequest::NeedCleanZonesOnly.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedCleanZonesOnly(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedSmoothMaterialInterfaces())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetNeedCleanZonesOnly
//
// Purpose:
//   Wrap avtDataRequest::SetNeedCleanZonesOnly.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetNeedCleanZonesOnly(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetNeedCleanZonesOnly(val);
    Py_RETURN_NONE;
}


//
// MIR algorithm
//

// ****************************************************************************
// Function: DataRequest_MIRAlgorithm
//
// Purpose:
//   Wrap avtDataRequest::MIRAlgorithm.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_MIRAlgorithm(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    int val = request->MIRAlgorithm();
    return PyInt_FromLong(val);
}


// ****************************************************************************
// Function: DataRequest_SetMIRAlgorithm
//
// Purpose:
//   Wrap avtDataRequest::SetMIRAlgorithm.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetMIRAlgorithm(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    int val;
    if(!PyArg_ParseTuple(args, "l", &val))
        return NULL;

    request->SetMIRAlgorithm(val);

    Py_RETURN_NONE;
}

//
// MIR number of iterations
//

// ****************************************************************************
// Function: DataRequest_MIRNumIterations
//
// Purpose:
//   Wrap avtDataRequest::MIRNumIterations.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_MIRNumIterations(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    int val = request->MIRNumIterations();
    return PyInt_FromLong(val);
}


// ****************************************************************************
// Function: DataRequest_SetMIRNumIterations
//
// Purpose:
//   Wrap avtDataRequest::SetMIRNumIterations.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetMIRNumIterations(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    int val;
    if(!PyArg_ParseTuple(args, "l", &val))
        return NULL;

    request->SetMIRNumIterations(val);

    Py_RETURN_NONE;
}


//
// MIR iteration damping
//

// ****************************************************************************
// Function: DataRequest_MIRIterationDamping
//
// Purpose:
//   Wrap avtDataRequest::MIRIterationDamping.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_MIRIterationDamping(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    double val = (double)request->MIRNumIterations();
    return PyFloat_FromDouble(val);
}


// ****************************************************************************
// Function: DataRequest_SetMIRIterationDamping
//
// Purpose:
//   Wrap avtDataRequest::SetMIRIterationDamping.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetMIRIterationDamping(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse float from args
    float val;
    if(!PyArg_ParseTuple(args, "f", &val))
        return NULL;

    request->SetMIRIterationDamping(val);

    Py_RETURN_NONE;
}

//
// isovolume mir volume fraction
//

// ****************************************************************************
// Function: DataRequest_IsovolumeMIRVF
//
// Purpose:
//   Wrap avtDataRequest::IsovolumeMIRVF.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_IsovolumeMIRVF(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    double val = (double)request->IsovolumeMIRVF();
    return PyFloat_FromDouble(val);
}


// ****************************************************************************
// Function: DataRequest_SetIsovolumeMIRVF
//
// Purpose:
//   Wrap avtDataRequest::SetIsovolumeMIRVF.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetIsovolumeMIRVF(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    float val;
    if(!PyArg_ParseTuple(args, "f", &val))
        return NULL;

    request->SetIsovolumeMIRVF(val);

    Py_RETURN_NONE;
}

//
// annealing time
//

// ****************************************************************************
// Function: DataRequest_AnnealingTime
//
// Purpose:
//   Wrap avtDataRequest::AnnealingTime.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_AnnealingTime(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    int val = request->AnnealingTime();
    return PyInt_FromLong(val);
}


// ****************************************************************************
// Function: DataRequest_SetAnnealingTime
//
// Purpose:
//   Wrap avtDataRequest::SetAnnealingTime.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetAnnealingTime(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    int val;
    if(!PyArg_ParseTuple(args, "l", &val))
        return NULL;

    request->SetAnnealingTime(val);

    Py_RETURN_NONE;
}


//
// simplify heavily mixed zones
//

// ****************************************************************************
// Function: DataRequest_SimplifyHeavilyMixedZones
//
// Purpose:
//   Wrap avtDataRequest::SimplifyHeavilyMixedZones.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SimplifyHeavilyMixedZones(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->SimplifyHeavilyMixedZones())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetSimplifyHeavilyMixedZones
//
// Purpose:
//   Wrap avtDataRequest::SetSimplifyHeavilyMixedZones.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetSimplifyHeavilyMixedZones(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetSimplifyHeavilyMixedZones(val);
    Py_RETURN_NONE;
}

//
// max number of materials per zone
//

// ****************************************************************************
// Function: DataRequest_MaxMaterialsPerZone
//
// Purpose:
//   Wrap avtDataRequest::MaxMaterialsPerZone.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_MaxMaterialsPerZone(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    int val = request->MaxMaterialsPerZone();
    return PyInt_FromLong(val);
}


// ****************************************************************************
// Function: DataRequest_SetMaxMaterialsPerZone
//
// Purpose:
//   Wrap avtDataRequest::SetMaxMaterialsPerZone.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetMaxMaterialsPerZone(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    int val;
    if(!PyArg_ParseTuple(args, "l", &val))
        return NULL;

    request->SetMaxMaterialsPerZone(val);

    Py_RETURN_NONE;
}


//
// variables are the same
//

// ****************************************************************************
// Function: DataRequest_SetMaxMaterialsPerZone
//
// Purpose:
//   Wrap avtDataRequest::SetMaxMaterialsPerZone.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_VariablesAreTheSame(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    PyObject *py_obj;
    if(!PyArg_ParseTuple(args, "o", &py_obj))
        return NULL;
    if(!PyDataRequest_Check(py_obj))
        return NULL;

    avtDataRequest_p request_b = PyDataRequest_FromPyObject(py_obj);

    if(request->VariablesAreTheSame(request_b))
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}


//
// data selections
//

// ****************************************************************************
// Function: DataRequest_AddDataSelection
//
// Purpose:
//   Wrap avtDataRequest::AddDataSelection.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_AddDataSelection(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    PyObject *py_obj;
    if(!PyArg_ParseTuple(args, "o", &py_obj))
        return NULL;

    if(!PyDataSelection_Check(py_obj))
        return NULL;

    avtDataSelection_p sel = PyDataSelection_FromPyObject(py_obj);

    int val = request->AddDataSelectionRefPtr(sel);

    return PyInt_FromLong((long)val);
}

// ****************************************************************************
// Function: DataRequest_RemoveAllDataSelections
//
// Purpose:
//   Wrap avtDataRequest::RemoveAllDataSelections.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_RemoveAllDataSelections(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    request->RemoveAllDataSelections();

    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_GetDataSelection
//
// Purpose:
//   Wrap avtDataRequest::GetDataSelection.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetDataSelection(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse bool from args
    int val;
    if(!PyArg_ParseTuple(args, "l", &val))
        return NULL;

    avtDataSelection_p sel = request->GetDataSelection(val);
    return PyDataSelection_Wrap(sel);
}

// ****************************************************************************
// Function: DataRequest_GetAllDataSelections
//
// Purpose:
//   Wrap avtDataRequest::GetAllDataSelections.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetAllDataSelections(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    std::vector<avtDataSelection_p> sels = request->GetAllDataSelections();
    int nsels = sels.size();
    PyObject *res = PyTuple_New(nsels);

    for(int i = 0; i < nsels; ++i)
    {
        PyObject *py_var = PyDataSelection_Wrap(sels[i]);
        if(py_var == NULL)
            continue;
        PyTuple_SET_ITEM(res, i, py_var);
    }

    return res;
}


//
// need native precision
//

// ****************************************************************************
// Function: DataRequest_NeedNativePrecision
//
// Purpose:
//   Wrap avtDataRequest::NeedNativePrecision.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedNativePrecision(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedNativePrecision())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetNeedNativePrecision
//
// Purpose:
//   Wrap avtDataRequest::SetNeedNativePrecision.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetNeedNativePrecision(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetNeedNativePrecision(val);
    Py_RETURN_NONE;
}

//
// admissible data types
//

// ****************************************************************************
// Function: DataRequest_UpdateAdmissibleDataTypes
//
// Purpose:
//   Wrap avtDataRequest::UpdateAdmissibleDataTypes.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_UpdateAdmissibleDataTypes(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PySequence_Check(py_val) == 0)
        return NULL;

    PyObject *py_seq = PySequence_Fast(py_val,"expected sequence");
    int nvals = PySequence_Size(py_seq);
    std::vector<int> dtypes(nvals);
    int val;

    for(int i = 0; i < nvals ; i++)
    {
        PyObject *py_item = PySequence_Fast_GET_ITEM(py_seq,i); //borrowed
        if(PythonInterpreter::PyObjectToInteger(py_item,val))
            dtypes[i] = val;
    }
    Py_DECREF(py_seq);

    request->UpdateAdmissibleDataTypes(dtypes);
    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_IsAdmissibleDataType
//
// Purpose:
//   Wrap avtDataRequest::IsAdmissibleDataType.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_IsAdmissibleDataType(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    // parse int index from args
    int val;
    if(!PyArg_ParseTuple(args, "l", &val))
        return NULL;

    if(request->IsAdmissibleDataType(val))
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_GetAdmissibleDataTypes
//
// Purpose:
//   Wrap avtDataRequest::GetAdmissibleDataTypes.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_GetAdmissibleDataTypes(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    std::vector<int> dtypes = request->GetAdmissibleDataTypes();
    int ndtypes= dtypes.size();
    PyObject *res = PyTuple_New(ndtypes);

    for(int i = 0; i < ndtypes; ++i)
    {
        PyObject *py_var = PyInt_FromLong((long)dtypes[i]);
        if(py_var == NULL)
            continue;
        PyTuple_SET_ITEM(res, i, py_var);
    }

    return res;
}

//
// csg related
//


// ****************************************************************************
// Function: DataRequest_DiscTol
//
// Purpose:
//   Wrap avtDataRequest::DiscTol
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_DiscTol(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    double val = request->DiscTol();
    return PyFloat_FromDouble(val);
}

// ****************************************************************************
// Function: DataRequest_SetDiscTol
//
// Purpose:
//   Wrap avtDataRequest::SetDiscTol
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetDiscTol(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    double val;
    if(!PyArg_ParseTuple(args, "d", &val))
        return NULL;

    request->SetDiscTol(val);
    Py_RETURN_NONE;
}


// ****************************************************************************
// Function: DataRequest_FlatTol
//
// Purpose:
//   Wrap avtDataRequest::FlatTol
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_FlatTol(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    double val = request->FlatTol();
    return PyFloat_FromDouble(val);
}

// ****************************************************************************
// Function: DataRequest_SetFlatTol
//
// Purpose:
//   Wrap avtDataRequest::SetFlatTol
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetFlatTol(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    double val;
    if(!PyArg_ParseTuple(args, "d", &val))
        return NULL;

    request->SetFlatTol(val);
    Py_RETURN_NONE;
}


// ****************************************************************************
// Function: DataRequest_DiscMode
//
// Purpose:
//   Wrap avtDataRequest::DiscMode.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_DiscMode(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    return Py_BuildValue("l",request->DiscMode());
}

// ****************************************************************************
// Function: DataRequest_SetDiscMode
//
// Purpose:
//   Wrap avtDataRequest::SetDiscMode.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetDiscMode(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    int val;
    if(!PyArg_ParseTuple(args, "l", &val))
        return NULL;

    request->SetDiscMode(val);
    Py_RETURN_NONE;
}


// ****************************************************************************
// Function: DataRequest_DiscBoundaryOnly
//
// Purpose:
//   Wrap avtDataRequest::DiscBoundaryOnly.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_DiscBoundaryOnly(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->DiscBoundaryOnly())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetDiscBoundaryOnly
//
// Purpose:
//   Wrap avtDataRequest::SetDiscBoundaryOnly.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetDiscBoundaryOnly(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetDiscBoundaryOnly(val);
    Py_RETURN_NONE;
}

// ****************************************************************************
// Function: DataRequest_PassNativeCSG
//
// Purpose:
//   Wrap avtDataRequest::PassNativeCSG.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_PassNativeCSG(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->PassNativeCSG())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetPassNativeCSG
//
// Purpose:
//   Wrap avtDataRequest::SetPassNativeCSG.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetPassNativeCSG(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetPassNativeCSG(val);
    Py_RETURN_NONE;
}


//
// transform vectors
//

// ****************************************************************************
// Function: DataRequest_TransformVectorsDuringProject
//
// Purpose:
//   Wrap avtDataRequest::TransformVectorsDuringProject.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_TransformVectorsDuringProject(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->TransformVectorsDuringProject())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetTransformVectorsDuringProject
//
// Purpose:
//   Wrap avtDataRequest::SetTransformVectorsDuringProject.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetTransformVectorsDuringProject(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetTransformVectorsDuringProject(val);
    Py_RETURN_NONE;
}


//
// post ghost mat info
//

// ****************************************************************************
// Function: DataRequest_NeedPostGhostMaterialInfo
//
// Purpose:
//   Wrap avtDataRequest::NeedPostGhostMaterialInfo.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_NeedPostGhostMaterialInfo(PyObject *self, PyObject *args)
{
    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);

    if(request->NeedPostGhostMaterialInfo())
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

// ****************************************************************************
// Function: DataRequest_SetNeedPostGhostMaterialInfo
//
// Purpose:
//   Wrap avtDataRequest::SetNeedPostGhostMaterialInfo.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_SetNeedPostGhostMaterialInfo(PyObject *self, PyObject *args)
{

    PyDataRequestObject *obj = (PyDataRequestObject *)self;
    avtDataRequest_p request = *(obj->request);
    // parse bool from args
    PyObject *py_val;
    if(!PyArg_ParseTuple(args, "o", &py_val))
        return NULL;

    if(PyBool_Check(py_val) == 0)
        return NULL;

    bool val;
    if(py_val == Py_True)
        val = true;
    else if(py_val == Py_False)
        val = false;

    request->SetNeedPostGhostMaterialInfo(val);
    Py_RETURN_NONE;
}



//
// Method Table
//

static struct PyMethodDef DataRequest_methods[] = {
    // timestep
    {"GetTimestep",                             DataRequest_GetTimestep, METH_VARARGS},
    {"SetTimestep",                             DataRequest_SetTimestep, METH_VARARGS},
    // variable
    {"GetVariable",                             DataRequest_GetVariable, METH_VARARGS},
    // sil related
    {"GetSIL",                                  DataRequest_GetSIL, METH_VARARGS},
    {"GetRestriction",                          DataRequest_GetRestriction, METH_VARARGS},
    // original variable
    {"GetOriginalVariable",                     DataRequest_GetOriginalVariable, METH_VARARGS},
    {"SetOriginalVariable",                     DataRequest_SetOriginalVariable, METH_VARARGS},
    // secondary variables
    {"AddSecondaryVariable",                    DataRequest_AddSecondaryVariable, METH_VARARGS},
    {"RemoveSecondaryVariable",                 DataRequest_RemoveSecondaryVariable, METH_VARARGS},
    {"RemoveAllSecondaryVariables",             DataRequest_RemoveAllSecondaryVariables, METH_VARARGS},
    {"HasSecondaryVariable",                    DataRequest_HasSecondaryVariable, METH_VARARGS},
    {"GetSecondaryVariables",                   DataRequest_GetSecondaryVariables, METH_VARARGS},
    {"GetSecondaryVariablesWithoutDuplicates",  DataRequest_GetSecondaryVariablesWithoutDuplicates, METH_VARARGS},
    // ,or
    {"MustDoMaterialInterfaceReconstruction",   DataRequest_MustDoMaterialInterfaceReconstruction, METH_VARARGS},
    {"ForceMaterialInterfaceReconstructionOn",  DataRequest_ForceMaterialInterfaceReconstructionOn, METH_VARARGS},
    {"ForceMaterialInterfaceReconstructionOff", DataRequest_ForceMaterialInterfaceReconstructionOff, METH_VARARGS},
    // internal surfaces
    {"NeedInternalSurfaces",                    DataRequest_NeedInternalSurfaces, METH_VARARGS},
    {"TurnInternalSurfacesOn",                  DataRequest_TurnInternalSurfacesOn, METH_VARARGS},
    {"TurnInternalSurfacesOff",                 DataRequest_TurnInternalSurfacesOff, METH_VARARGS},
    // boundary surface representation
    {"GetBoundarySurfaceRepresentation",        DataRequest_GetBoundarySurfaceRepresentation, METH_VARARGS},
    {"TurnBoundarySurfaceRepresentationOn",     DataRequest_TurnBoundarySurfaceRepresentationOn, METH_VARARGS},
    {"TurnBoundarySurfaceRepresentationOff",    DataRequest_TurnBoundarySurfaceRepresentationOff, METH_VARARGS},
    // simplifed nesting representation
    {"GetSimplifiedNestingRepresentation",      DataRequest_GetSimplifiedNestingRepresentation, METH_VARARGS},
    {"TurnSimplifiedNestingRepresentationOn",   DataRequest_TurnSimplifiedNestingRepresentationOn, METH_VARARGS},
    {"TurnSimplifiedNestingRepresentationOff",  DataRequest_TurnBoundarySurfaceRepresentationOff, METH_VARARGS},
    // structured indicies
    {"NeedStructuredIndices",                   DataRequest_NeedStructuredIndices, METH_VARARGS},
    {"SetNeedStructuredIndices",                DataRequest_SetNeedStructuredIndices, METH_VARARGS},
    // amr indicies
    {"NeedAMRIndices",                          DataRequest_NeedAMRIndices, METH_VARARGS},
    {"SetNeedAMRIndices",                       DataRequest_SetNeedAMRIndices, METH_VARARGS},
    // zone numbers
    {"NeedZoneNumbers",                         DataRequest_NeedZoneNumbers, METH_VARARGS},
    {"TurnZoneNumbersOn",                       DataRequest_TurnZoneNumbersOn, METH_VARARGS},
    {"TurnZoneNumbersOff",                      DataRequest_TurnZoneNumbersOff, METH_VARARGS},
    // node numbers
    {"NeedNodeNumbers",                         DataRequest_NeedNodeNumbers, METH_VARARGS},
    {"TurnNodeNumbersOn",                       DataRequest_TurnNodeNumbersOn, METH_VARARGS},
    {"TurnNodeNumbersOff",                      DataRequest_TurnNodeNumbersOff, METH_VARARGS},
    // global zone numbers
    {"NeedGlobalZoneNumbers",                   DataRequest_NeedGlobalZoneNumbers, METH_VARARGS},
    {"TurnGlobalZoneNumbersOn",                 DataRequest_TurnGlobalZoneNumbersOn, METH_VARARGS},
    {"TurnGlobalZoneNumbersOff",                DataRequest_TurnGlobalZoneNumbersOff, METH_VARARGS},
    // global node numbers
    {"NeedGlobalNodeNumbers",                   DataRequest_NeedGlobalNodeNumbers, METH_VARARGS},
    {"TurnGlobalNodeNumbersOn",                 DataRequest_TurnGlobalNodeNumbersOn, METH_VARARGS},
    {"TurnGlobalNodeNumbersOff",                DataRequest_TurnGlobalNodeNumbersOff, METH_VARARGS},
    // require zones
    {"MayRequireZones",                         DataRequest_MayRequireZones, METH_VARARGS},
    {"SetMayRequireZones",                      DataRequest_SetMayRequireZones, METH_VARARGS},
    // require nodes
    {"MayRequireNodes",                         DataRequest_MayRequireNodes, METH_VARARGS},
    {"SetMayRequireNodes",                      DataRequest_SetMayRequireNodes, METH_VARARGS},
    // maintain original connectivity
    {"MustMaintainOriginalConnectivity",        DataRequest_MustMaintainOriginalConnectivity, METH_VARARGS},
    {"SetMaintainOriginalConnectivity",         DataRequest_SetMaintainOriginalConnectivity, METH_VARARGS},
    // ghost data type
    {"GetDesiredGhostDataType",                 DataRequest_GetDesiredGhostDataType, METH_VARARGS},
    {"SetDesiredGhostDataType",                 DataRequest_SetDesiredGhostDataType, METH_VARARGS},
    // valid face connectivity
    {"NeedValidFaceConnectivity",               DataRequest_SetNeedValidFaceConnectivity, METH_VARARGS},
    {"SetNeedValidFaceConnectivity",            DataRequest_SetNeedValidFaceConnectivity, METH_VARARGS},
    // uses all domains
    {"UsesAllDomains",                          DataRequest_UsesAllDomains, METH_VARARGS},
    {"SetUsesAllDomains",                       DataRequest_SetUsesAllDomains, METH_VARARGS},
    // mixed var reconstruction
    {"NeedMixedVariableReconstruction",         DataRequest_NeedMixedVariableReconstruction, METH_VARARGS},
    {"SetNeedMixedVariableReconstruction",      DataRequest_SetNeedMixedVariableReconstruction, METH_VARARGS},
    // smooth material interfaces
    {"NeedSmoothMaterialInterfaces",            DataRequest_NeedSmoothMaterialInterfaces, METH_VARARGS},
    {"SetNeedSmoothMaterialInterfaces",         DataRequest_SetNeedSmoothMaterialInterfaces, METH_VARARGS},
    // clean zones only
    {"NeedCleanZonesOnly",                      DataRequest_NeedCleanZonesOnly, METH_VARARGS},
    {"SetNeedCleanZonesOnly",                   DataRequest_SetNeedCleanZonesOnly, METH_VARARGS},
    // mir algorithm
    {"MIRAlgorithm",                            DataRequest_MIRAlgorithm, METH_VARARGS},
    {"SetMIRAlgorithm",                         DataRequest_SetMIRAlgorithm, METH_VARARGS},
    // mir number of iterations
    {"MIRNumIterations",                        DataRequest_MIRNumIterations, METH_VARARGS},
    {"SetMIRNumIterations",                     DataRequest_SetMIRNumIterations, METH_VARARGS},
     // iteration damping
    {"MIRIterationDamping",                     DataRequest_MIRIterationDamping, METH_VARARGS},
    {"SetMIRIterationDamping",                  DataRequest_SetMIRIterationDamping, METH_VARARGS},
    // isovolume mir volume fraction
    {"IsovolumeMIRVF",                          DataRequest_IsovolumeMIRVF, METH_VARARGS},
    {"SetIsovolumeMIRVF",                       DataRequest_SetIsovolumeMIRVF, METH_VARARGS},
    // annealing time
    {"AnnealingTime",                           DataRequest_AnnealingTime, METH_VARARGS},
    {"SetAnnealingTime",                        DataRequest_SetAnnealingTime, METH_VARARGS},
    // simplify heavily mixed zones
    {"SimplifyHeavilyMixedZones",               DataRequest_SimplifyHeavilyMixedZones, METH_VARARGS},
    {"SetSimplifyHeavilyMixedZones",            DataRequest_SetSimplifyHeavilyMixedZones, METH_VARARGS},
    // max materials per zone
    {"MaxMaterialsPerZone",                     DataRequest_MaxMaterialsPerZone, METH_VARARGS},
    {"SetMaxMaterialsPerZone",                  DataRequest_SetMaxMaterialsPerZone, METH_VARARGS},
    // variables are the same
    {"VariablesAreTheSame",                     DataRequest_VariablesAreTheSame, METH_VARARGS},
    // data selections 
    {"AddDataSelection",                        DataRequest_AddDataSelection, METH_VARARGS},
    {"RemoveAllDataSelections",                 DataRequest_RemoveAllDataSelections, METH_VARARGS},
    {"GetDataSelection",                        DataRequest_GetDataSelection, METH_VARARGS},
    {"GetAllDataSelections",                    DataRequest_GetAllDataSelections, METH_VARARGS},
    // need native precision
    {"NeedNativePrecision",                     DataRequest_NeedNativePrecision, METH_VARARGS},
    {"SetNeedNativePrecision",                  DataRequest_SetNeedNativePrecision, METH_VARARGS},
    // admissiable data types
    {"UpdateAdmissibleDataTypes",               DataRequest_UpdateAdmissibleDataTypes, METH_VARARGS},
    {"IsAdmissibleDataType",                    DataRequest_IsAdmissibleDataType, METH_VARARGS},
    {"GetAdmissibleDataTypes",                  DataRequest_GetAdmissibleDataTypes, METH_VARARGS},
    // csg related
    {"DiscTol",                                 DataRequest_DiscTol, METH_VARARGS},
    {"SetDiscTol",                              DataRequest_SetDiscTol, METH_VARARGS},
    {"FlatTol",                                 DataRequest_FlatTol, METH_VARARGS},
    {"SetFlatTol",                              DataRequest_SetFlatTol, METH_VARARGS},
    {"DiscMode",                                DataRequest_DiscMode, METH_VARARGS},
    {"SetDiscMode",                             DataRequest_SetDiscMode, METH_VARARGS},
    {"DiscBoundaryOnly",                        DataRequest_DiscBoundaryOnly, METH_VARARGS},
    {"SetDiscBoundaryOnly",                     DataRequest_SetDiscBoundaryOnly, METH_VARARGS},
    {"PassNativeCSG",                           DataRequest_PassNativeCSG, METH_VARARGS},
    {"SetPassNativeCSG",                        DataRequest_SetPassNativeCSG, METH_VARARGS},
    // transform vectors
    {"TransformVectorsDuringProject",           DataRequest_TransformVectorsDuringProject, METH_VARARGS},
    {"SetTransformVectorsDuringProject",        DataRequest_SetTransformVectorsDuringProject, METH_VARARGS},
    // post ghost mat info
    {"NeedPostGhostMaterialInfo",               DataRequest_NeedPostGhostMaterialInfo, METH_VARARGS},
    {"SetNeedPostGhostMaterialInfo",            DataRequest_SetNeedPostGhostMaterialInfo, METH_VARARGS},
    {NULL, NULL}
};

//
// Type functions
//


// ****************************************************************************
// Function: DataRequest_dealloc
//
// Purpose:
//   Destructor for PyDataRequest.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static void
DataRequest_dealloc(PyObject *v)
{
    // DataRequest is  stored in a a ref ptr, so it will clean itself up.
}


// ****************************************************************************
// Function: DataRequest_getattr
//
// Purpose:
//   Attribute fetch for PyDataRequest.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static PyObject *
DataRequest_getattr(PyObject *self, char *name)
{
    return Py_FindMethod(DataRequest_methods, self, name);
}


// ****************************************************************************
// Function: DataRequest_print
//
// Purpose:
//   Print function for PyDataRequest.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************
static int
DataRequest_print(PyObject *v, FILE *fp, int flags)
{
    return 0;
}

#if PY_MAJOR_VERSION > 2 || (PY_MAJOR_VERSION == 2 && PY_MINOR_VERSION >= 5)
static const char *DataRequest_Doc = "This class provides access to the avt pipeline data request.";
#else
static char *DataRequest_Doc = "This class provides access to the avt pipeline data req";
#endif

//
// The type description structure
//
static PyTypeObject PyDataRequestType =
{
    //
    // Type header
    //
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                   // ob_size
    "DataRequest",                       // tp_name
    sizeof(PyDataRequestObject),         // tp_basicsize
    0,                                   // tp_itemsize
    //
    // Standard methods
    //
    (destructor)DataRequest_dealloc,     // tp_dealloc
    (printfunc)DataRequest_print,        // tp_print
    (getattrfunc)DataRequest_getattr,    // tp_getattr
    (setattrfunc)0,                      // tp_setattr
    (cmpfunc)0,                          // tp_compare
    (reprfunc)0,                         // tp_repr
    //
    // Type Categories
    //
    0,                                   // tp_as_number
    0,                                   // tp_as_sequence
    0,                                   // tp_as_mapping
    //
    // More methods
    //
    0,                                   // tp_hash
    0,                                   // tp_call
    0,                                   // tp_str
    0,                                   // tp_getattro
    0,                                   // tp_setattro
    0,                                   // tp_as_buffer
    Py_TPFLAGS_CHECKTYPES,               // tp_flags
    DataRequest_Doc,                     // tp_doc
    0,                                   // tp_traverse
    0,                                   // tp_clear
    0,                                   // tp_richcompare
    0                                    // tp_weaklistoffset
};


///////////////////////////////////////////////////////////////////////////////
//
// Interface used by Python Filter environment.
//
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Function: PyDataRequest_Wrap
//
// Purpose:
//   Wrap avtDataRequest_p into a python data request object.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

PyObject *
PyDataRequest_Wrap(avtDataRequest_p request)
{
    PyDataRequestObject *res;
    res = PyObject_NEW(PyDataRequestObject, &PyDataRequestType);
    if(res  == NULL)
        return NULL;
    res->request = new avtDataRequest_p;
    // set the contract
    *(res->request) = request;
    return (PyObject *)res;
}

// ****************************************************************************
// Function: PyDataRequest_Check
//
// Purpose:
//   Check if given python object is an instance of PyDataRequest.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

bool
PyDataRequest_Check(PyObject *obj)
{
    return (obj->ob_type == &PyDataRequestType);
}



// ****************************************************************************
// Function: PyDataRequest_FromPyObject
//
// Purpose:
//   Obtain the avtDataRequest from a python contract object.
//
//
// Programmer: Cyrus Harrison
// Creation:   Tue Feb  9 08:58:23 PST 2010
//
// Modifications:
//
// ****************************************************************************

avtDataRequest_p
PyDataRequest_FromPyObject(PyObject *obj)
{
    PyDataRequestObject *py_request = (PyDataRequestObject *)obj;
    return *(py_request->request);
}


