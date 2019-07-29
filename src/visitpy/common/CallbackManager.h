// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CALLBACK_MANAGER_H
#define CALLBACK_MANAGER_H
#include <Python.h>
#include <map>
#include <vector>
#include <string>
#include <deque>
#include <vectortypes.h>

#include <SimpleObserver.h>
#include <ObserverToCallback.h>

class AttributeSubject;
class Threading;
class ViewerProxy;

typedef bool (AddWorkCallback)(Subject *, void *data);

// ****************************************************************************
// Class: CallbackManager
//
// Purpose:
//   This class manages a queue of callback requests caused by state objects
//   updating. The callback requests are executed on another thread and their
//   state objects are wrapped in our Python bindings and then dispatched to
//   user-specified Python functions.
//
// Notes:      Init done on thread 1. Callback requests added on thread 2.
//             Callbacks actually executed on thread 3.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb  1 16:18:28 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Feb  6 10:27:51 PST 2008
//   Added support for callback data.
//
//   Hank Childs, Mon Jan 18 21:38:53 PST 2010
//   Added method IsWorking on behalf of Tila Ochatta, Huy Vo, and Claudio
//   Silva.  (For VisTrails.)
//
// ****************************************************************************

class CallbackManager : public SimpleObserver
{
    struct CallbackData
    {
        std::string       name;
        ObserverCallback *handler;
        void             *handler_data;
        AddWorkCallback  *addwork;
        void             *addwork_data;
        PyObject         *pycb;
        PyObject         *pycb_data;
    };

    struct WorkItem
    {
        // The subject in ViewerState for whom we do the callback.
        AttributeSubject *key;
        // An instance of the subject that has the actual object contents that
        // we had at the time the callback was received.
        AttributeSubject *data;
    };

    typedef std::map<std::string, Subject *> StringSubjectMap;
    typedef std::map<Subject *, CallbackData> SubjectCallbackDataMap;

public:
    struct CallbackHandlerData
    {
        // The Python object that will handle the callback
        PyObject         *pycb;
        PyObject         *pycb_data;
        // An instance of the subject that has the actual object contents that
        // we had at the time the callback was received.
        AttributeSubject *data;
        // User-defined data.
        void             *userdata;
        // Pointer to the viewer proxy
        ViewerProxy      *viewer;
    };

    CallbackManager(ViewerProxy *);
    ~CallbackManager();
    virtual void SubjectRemoved(Subject *);
    virtual void Update(Subject *);

    void RegisterHandler(Subject *, const std::string &, 
                         ObserverCallback *handler, void *handler_data,
                         AddWorkCallback  *addwork, void *addwork_data);
    bool RegisterCallback(const std::string &, PyObject *, PyObject *);
    void GetCallbackNames(stringVector &names) const;
    PyObject *GetCallback(Subject *) const;

    void WorkAllowed();
    void Work();
    bool IsWorking();

private:
    bool RegisterCallback(Subject *, PyObject *, PyObject *);
    void StartWork();

    ViewerProxy           *viewer;
    StringSubjectMap       nameToSubject;
    SubjectCallbackDataMap callbacks;
    std::deque<WorkItem>   work;
    bool                   working;
    bool                   workAllowed;
    Threading             *threading;
};

#endif
