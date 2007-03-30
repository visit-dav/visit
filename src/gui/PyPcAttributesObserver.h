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
