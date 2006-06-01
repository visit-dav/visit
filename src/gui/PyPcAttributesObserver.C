/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <PyPcAttributesObserver.h>
#include <PcAttributes.h>
#include <Interpreter.h>

// Static members of the PyPcAttributesObserver class.
PyPcAttributesObserver *PyPcAttributesObserver::_this;

// *******************************************************************
// Method: PyPcAttributesObserver::PyPcAttributesObserver
//
// Purpose: 
//   Constructs a new Python module that knows how to manipulate
//   the PcAttributes state object. It also watches that PcAttributes
//   state object so when any other observer changes it, this object
//   can write the changes, in Python, to the log file.
//
// Arguments:
//   attr        : The PcAttributes state object that this class
//                 will watch for changes.
//   interpreter : The interpreter that this class will use.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 3 16:45:46 PST 2000
//
// Modifications:
//   
// *******************************************************************

PyPcAttributesObserver::PyPcAttributesObserver(PcAttributes *attr,
    Interpreter *_int) : Observer(attr)
{
    // Save some pointers.
    pcAtts = attr;
    interpreter = _int;

    // Save a "this" pointer for later so the static member functions
    // have a reference to a PyPcAttributesObserver object. There should
    // never be more than one, so it should not matter that much.
    _this = this;

    static PyMethodDef PcMethods[] = {
        {"SetCentering",  PyPcAttributesObserver::SetCentering, METH_VARARGS},
        {"SetScaling",    PyPcAttributesObserver::SetScaling,   METH_VARARGS},
        {"SetLegendFlag",  PyPcAttributesObserver::SetLegendFlag, METH_VARARGS},
        {"SetLightingFlag",  PyPcAttributesObserver::SetLightingFlag, METH_VARARGS},
        {"SetMin",  PyPcAttributesObserver::SetMin, METH_VARARGS},
        {"SetMax",  PyPcAttributesObserver::SetMax, METH_VARARGS},
        {"SetMinFlag",  PyPcAttributesObserver::SetMinFlag, METH_VARARGS},
        {"SetMaxFlag",  PyPcAttributesObserver::SetMaxFlag, METH_VARARGS},
        {"SetPointSize",  PyPcAttributesObserver::SetPointSize, METH_VARARGS},
        {"SetSkewFactor",  PyPcAttributesObserver::SetSkewFactor, METH_VARARGS},
        {"GetCentering",  PyPcAttributesObserver::GetCentering, METH_VARARGS},
        {"GetScaling",    PyPcAttributesObserver::GetScaling,   METH_VARARGS},
        {"GetLegendFlag",  PyPcAttributesObserver::GetLegendFlag, METH_VARARGS},
        {"GetLightingFlag",  PyPcAttributesObserver::GetLightingFlag, METH_VARARGS},
        {"GetMin",  PyPcAttributesObserver::GetMin, METH_VARARGS},
        {"GetMax",  PyPcAttributesObserver::GetMax, METH_VARARGS},
        {"GetPointSize",  PyPcAttributesObserver::GetPointSize, METH_VARARGS},
        {"GetSkewFactor",  PyPcAttributesObserver::GetSkewFactor, METH_VARARGS},
        {NULL,      NULL}        /* Sentinel */
    };

    // Initialize the new Python module.
    PyObject *m = Py_InitModule("pc", PcMethods);

    // Tell the Python interpreter to import the pc module.
    interpreter->ExecuteString("import pc");
}

// *******************************************************************
// Method: PyPcAttributesObserver
//
// Purpose: 
//   Destructor for the PyPcAttributesObserver class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 3 16:47:47 PST 2000
//
// Modifications:
//   
// *******************************************************************

PyPcAttributesObserver::~PyPcAttributesObserver()
{
}

// *******************************************************************
// Method: PyPcAttributesObserver::Update
//
// Purpose: 
//   This method writes the Python commands to the log file that are
//   needed to do the state changes that this object is being told
//   about.
//
// Notes:
//   This method is only called when other objects have told the 
//   state object to update. If that is not the case, check the
//   Python handler functions and make sure that they have a
//   SetUpdate(false) before the call to Notify.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 3 16:04:47 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
PyPcAttributesObserver::Update(Subject *)
{
    char *tf[] = {"false", "true"};
    char buf[100];

    // If there is no Interpreter object then return.
    if(interpreter == 0)
        return;

    // Loop through all the attributes and do something for
    // each of them that changed. 
    for(int i = 0; i < pcAtts->NumAttributes(); ++i)
    {
        if(!pcAtts->IsSelected(i))
            continue;

        switch(i)
        {
        case 0: // legendFlag
            sprintf(buf, "pc.SetLegendFlag(%s)", 
                tf[pcAtts->GetLegendFlag() ? 1 : 0]);
            break;
        case 1: // lightingFlag
            sprintf(buf, "pc.SetLightingFlag(%s)", 
                tf[pcAtts->GetLightingFlag() ? 1 : 0]);
            break;
        case 2: // minFlag
            sprintf(buf, "pc.SetMinFlag(%s)", 
                tf[pcAtts->GetLegendFlag() ? 1 : 0]);
            break;
        case 3: // maxFlag
            sprintf(buf, "pc.SetMaxFlag(%s)", 
                tf[pcAtts->GetMaxFlag() ? 1 : 0]);
            break;
        case 4: // centering
            sprintf(buf, "pc.SetCentering(%d)", pcAtts->GetCentering());
            break;
        case 5: // scaling
            sprintf(buf, "pc.SetScaling(%d)", pcAtts->GetScaling());
            break;
        case 6: // min
            sprintf(buf, "pc.SetMin(%g)", pcAtts->GetMin());
            break;
        case 7: // max
            sprintf(buf, "pc.SetMax(%g)", pcAtts->GetMax());
            break;
        case 8: // pointSize
            sprintf(buf, "pc.SetPointSize(%g)", pcAtts->GetPointSize());
            break;
        case 9: // skewFactor
            sprintf(buf, "pc.SetSkewFactor(%g)", pcAtts->GetSkewFactor());
            break;
        }

        // Write the string to the log.
        interpreter->WriteLog(buf);
    } // end for
}

///////////////////////////////////////////////////////////////////////////
/// Python wrapper functions (Set)
///////////////////////////////////////////////////////////////////////////

PyObject *
PyPcAttributesObserver::SetCentering(PyObject *, PyObject *args)
{
    int ival;

    if (!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    _this->pcAtts->SetCentering(ival);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetCentering: centering=%d\n", ival);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PyPcAttributesObserver::SetScaling(PyObject *, PyObject *args)
{
    int ival;

    if (!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    _this->pcAtts->SetScaling(ival);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetScaling: scaling=%d\n", ival);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PyPcAttributesObserver::SetLegendFlag(PyObject *, PyObject *args)
{
    int ival;

    if (!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    bool legend = (ival == 1);
    _this->pcAtts->SetLegendFlag(legend);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetLegendFlag: legend=%d\n", ival);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PyPcAttributesObserver::SetMinFlag(PyObject *, PyObject *args)
{
    int ival;

    if (!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    bool min = (ival == 1);
    _this->pcAtts->SetMinFlag(min);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetMinFlag: minFlag=%d\n", ival);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PyPcAttributesObserver::SetMaxFlag(PyObject *, PyObject *args)
{
    int ival;

    if (!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    bool max = (ival == 1);
    _this->pcAtts->SetMaxFlag(max);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetMaxFlag: maxFlag=%d\n", ival);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PyPcAttributesObserver::SetLightingFlag(PyObject *, PyObject *args)
{
    int ival;

    if (!PyArg_ParseTuple(args, "i", &ival))
        return NULL;

    bool lighting = (ival == 1);
    _this->pcAtts->SetLightingFlag(lighting);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetLightingFlag: lighting=%d\n", ival);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PyPcAttributesObserver::SetMin(PyObject *, PyObject *args)
{
    double val;

    if (!PyArg_ParseTuple(args, "d", &val))
        return NULL;

    _this->pcAtts->SetMin(val);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetMin: min=%g\n", val);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PyPcAttributesObserver::SetMax(PyObject *, PyObject *args)
{
    double val;

    if (!PyArg_ParseTuple(args, "d", &val))
        return NULL;

    _this->pcAtts->SetMax(val);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetMax: max=%g\n", val);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PyPcAttributesObserver::SetPointSize(PyObject *, PyObject *args)
{
    double val;

    if (!PyArg_ParseTuple(args, "d", &val))
        return NULL;

    _this->pcAtts->SetPointSize(val);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetPointSize: pointsize=%g\n", val);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

PyObject *
PyPcAttributesObserver::SetSkewFactor(PyObject *, PyObject *args)
{
    double val;

    if (!PyArg_ParseTuple(args, "d", &val))
        return NULL;

    _this->pcAtts->SetSkewFactor(val);
    _this->SetUpdate(false);
    _this->pcAtts->Notify();
#ifdef DEBUG
    printf("PyPcAttributesObserver::SetSkewFactor: skew=%f\n", val);
#endif
    Py_INCREF(Py_None);
    return Py_None;
}

///////////////////////////////////////////////////////////////////////////
/// Python wrapper functions (Get)
///////////////////////////////////////////////////////////////////////////

PyObject *
PyPcAttributesObserver::GetCentering(PyObject *, PyObject *)
{
    return Py_BuildValue("i", _this->pcAtts->GetCentering());
}

PyObject *
PyPcAttributesObserver::GetScaling(PyObject *, PyObject *)
{
    return Py_BuildValue("i", _this->pcAtts->GetScaling());
}

PyObject *
PyPcAttributesObserver::GetLegendFlag(PyObject *, PyObject *)
{
    return Py_BuildValue("i", _this->pcAtts->GetLegendFlag());
}

PyObject *
PyPcAttributesObserver::GetLightingFlag(PyObject *, PyObject *)
{
    return Py_BuildValue("i", _this->pcAtts->GetLightingFlag());
}

PyObject *
PyPcAttributesObserver::GetMin(PyObject *, PyObject *)
{
    return Py_BuildValue("d", _this->pcAtts->GetMin());
}

PyObject *
PyPcAttributesObserver::GetMax(PyObject *, PyObject *)
{
    return Py_BuildValue("d", _this->pcAtts->GetMax());
}

PyObject *
PyPcAttributesObserver::GetPointSize(PyObject *, PyObject *)
{
    return Py_BuildValue("d", _this->pcAtts->GetPointSize());
}

PyObject *
PyPcAttributesObserver::GetSkewFactor(PyObject *, PyObject *)
{
    return Py_BuildValue("d", _this->pcAtts->GetSkewFactor());
}
