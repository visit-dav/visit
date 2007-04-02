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

#ifndef PY_PCATTRIBUTES_OBSERVER_H
#define PY_PCATTRIBUTES_OBSERVER_H
#include <gui_exports.h>
#include <Python.h>
#include <Observer.h>

class Interpreter;
class PcAttributes;

// *******************************************************************
// Class: PyPcAttributesObserver
//
// Purpose:
//   This class has two purposes. The first is to add some commands
//   to the Python interpreter so it knows how to manipulate a
//   PcAttributes state object. The second purpose is to watch a
//   PcAttributes state object and log Python commands equivalent
//   to the state changes it makes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 3 16:41:41 PST 2000
//
// Modifications:
//   
// *******************************************************************

class GUI_API PyPcAttributesObserver : public Observer
{
public:
    PyPcAttributesObserver(PcAttributes *attr, Interpreter *_int = 0);
    ~PyPcAttributesObserver();
    void Update(Subject *TheChangedSubject);
private:
    static PyObject *SetCentering(PyObject *, PyObject *);
    static PyObject *SetScaling(PyObject *, PyObject *);
    static PyObject *SetLegendFlag(PyObject *, PyObject *);
    static PyObject *SetLightingFlag(PyObject *, PyObject *);
    static PyObject *SetMin(PyObject *, PyObject *);
    static PyObject *SetMinFlag(PyObject *, PyObject *);
    static PyObject *SetMax(PyObject *, PyObject *);
    static PyObject *SetMaxFlag(PyObject *, PyObject *);
    static PyObject *SetPointSize(PyObject *, PyObject *);
    static PyObject *SetSkewFactor(PyObject *, PyObject *);
    static PyObject *GetCentering(PyObject *, PyObject *);
    static PyObject *GetScaling(PyObject *, PyObject *);
    static PyObject *GetLegendFlag(PyObject *, PyObject *);
    static PyObject *GetLightingFlag(PyObject *, PyObject *);
    static PyObject *GetMin(PyObject *, PyObject *);
    static PyObject *GetMax(PyObject *, PyObject *);
    static PyObject *GetPointSize(PyObject *, PyObject *);
    static PyObject *GetSkewFactor(PyObject *, PyObject *);
private:
    Interpreter                   *interpreter;
    PcAttributes                  *pcAtts;
    static PyPcAttributesObserver *_this;
};

#endif
