/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#ifndef CALLBACK_MANAGER_H
#define CALLBACK_MANAGER_H
#include <map>
#include <vector>
#include <string>
#include <deque>
#include <vectortypes.h>

#include <SimpleObserver.h>
#include <ObserverToCallback.h>
#include <Python.h>

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
