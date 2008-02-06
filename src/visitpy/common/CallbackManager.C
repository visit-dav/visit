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
#include <CallbackManager.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <ViewerState.h>
#include <AttributeSubject.h>
#include <DebugStream.h>
#include <visitmodulehelpers.h>

// ****************************************************************************
// Class: Threading
//
// Purpose:
//   This class helps us launch threads and manage mutexes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb  1 16:22:43 PST 2008
//
// Modifications:
//   
// ****************************************************************************

class Threading
{
public:
    Threading()
    {
        THREAD_INIT();
        MUTEX_CREATE();
    }

    ~Threading()
    {
        MUTEX_DESTROY();
    }

#if defined(_WIN32)
#define THREAD_RETURN     DWORD WINAPI
#define THREAD_ARGUMENT   LPVOID
    HANDLE                tid;
    CRITICAL_SECTION      mutex;

    void THREAD_INIT()   { tid = INVALID_HANDLE; }
    void MUTEX_CREATE()  { InitializeCriticalSection(&mutex); }
    void MUTEX_DESTROY() { }
    void MUTEX_LOCK()    { EnterCriticalSection(&mutex); }
    void MUTEX_UNLOCK()  { LeaveCriticalSection(&mutex); }

    bool CreateThread(THREAD_RETURN (*thread_cb)(THREAD_ARGUMENT), THREAD_ARGUMENT cbData)
    {
        // Create the thread with the WIN32 API.
        DWORD Id;
        return (tid = CreateThread(0, 0, thread_cb, cbData, 0, &Id) != INVALID_HANDLE_VALUE;
    }
#else
#define THREAD_RETURN     void*
#define THREAD_ARGUMENT   void*
    pthread_t             tid;
    pthread_mutex_t       mutex;
    pthread_attr_t        thread_atts;

    void THREAD_INIT()   { pthread_attr_init(&thread_atts); }
    void MUTEX_CREATE()  { pthread_mutex_init(&mutex, NULL); }
    void MUTEX_DESTROY() { pthread_mutex_destroy(&mutex); }
    void MUTEX_LOCK()    { pthread_mutex_lock(&mutex); }
    void MUTEX_UNLOCK()  { pthread_mutex_unlock(&mutex); }

    bool CreateThread(THREAD_RETURN (*thread_cb)(THREAD_ARGUMENT), THREAD_ARGUMENT cbData)
    {
        // Create the thread using PThreads.
        return pthread_create(&tid, &thread_atts, thread_cb, cbData) != -1;
    }
#endif
};

// ****************************************************************************
// Method: CallbackManager::CallbackManager
//
// Purpose: 
//   Constructor for the CallbackManager class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:51:40 PST 2008
//
// Modifications:
//   
// ****************************************************************************

CallbackManager::CallbackManager() : SimpleObserver(), 
    nameToSubject(), callbacks(), work()
{
    working = false;
    workAllowed = false;
    threading = new Threading;
}

// ****************************************************************************
// Method: CallbackManager::~CallbackManager
//
// Purpose: 
//   Destructor for the CallbackManager class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:51:40 PST 2008
//
// Modifications:
//   
// ****************************************************************************

CallbackManager::~CallbackManager()
{
    // Decrement all of the callback function reference counts.
    for(SubjectCallbackDataMap::iterator it = callbacks.begin();
        it != callbacks.end(); ++it)
    {
        it->first->Detach(this);
        if(it->second.pycb != 0)
            Py_DECREF(it->second.pycb);
    }
    delete threading;
}

// ****************************************************************************
// Method: CallbackManager::WorkAllowed
//
// Purpose: 
//   Tells the CallbackManager that thread work is allowed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:52:12 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
CallbackManager::WorkAllowed()
{
    workAllowed = true;
}

// ****************************************************************************
// Method: CallbackManager::SubjectRemoved
//
// Purpose: 
//   This method is called when a subject needs to be removed from this object.
//
// Arguments:
//   subj : The subject to remove from observation.
//
// Returns:    
//
// Note:       This method is not currently called because the callback manager
//             is deleted before ViewerState.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:52:45 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
CallbackManager::SubjectRemoved(Subject *subj)
{
    threading->MUTEX_LOCK();
    SubjectCallbackDataMap::iterator it = callbacks.find(subj);
    if(it != callbacks.end())
    {
        subj->Detach(this);
        if(it->second.pycb != 0)
            Py_DECREF(it->second.pycb);
        callbacks.erase(it);
    }
    threading->MUTEX_UNLOCK();
}

// ****************************************************************************
// Method: CallbackManager::Update
//
// Purpose: 
//   This method is called by thread 2 when one of the observed state objects
//   updates. This method adds a work item to the thread work queue, if
//   there is a user-defined Python handler installed that can consume the
//   work item. Finally, we start the work thread if we don't already have one.
//
// Arguments:
//   subj : The subject that is updating.
//
// Returns:    
//
// Note:       Called from thread 2. Starts thread 3 to do the work.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:53:53 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
CallbackManager::Update(Subject *subj)
{
    // If callback work is not yet allowed then return.
    if(!workAllowed)
        return;

    // Push some work onto the queue if there is a handler for the
    // subject that updated.
    threading->MUTEX_LOCK();
    SubjectCallbackDataMap::iterator it = callbacks.find(subj);
    if(it != callbacks.end())
    {
        if(it->second.handler != 0)
        {
            // Execute a callback to see if the work should be added to the 
            // work queue. This lets us not add work unless we know that we
            // will process it later in Python.
            bool addwork = true;
            if(it->second.addwork != 0)
                addwork = (*it->second.addwork)(subj, it->second.addwork_data);

            // Create a work item that identifies the callback to be
            // performed as well as the data that we should pass to
            // the callback that does that work.
            if(addwork)
            {
                WorkItem w;
                w.key = (AttributeSubject *)subj;
                w.data = ((AttributeSubject *)subj)->NewInstance(true);
                work.push_back(w);
            }
        }
    }
    bool hasWork = work.size() > 0;
    threading->MUTEX_UNLOCK();

    if(hasWork)
        StartWork();
}

// ****************************************************************************
// Method: work_callback
//
// Purpose: 
//   This is the main function for thread 3 -- the worker thread.
//
// Arguments:
//   data : A pointer to a void* array that contains the "this" pointer for the
//          callback manager.
//
// Returns:    NULL
//
// Note:       This function executes on thread 3.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:56:16 PST 2008
//
// Modifications:
//   
// ****************************************************************************

static THREAD_RETURN
work_callback(THREAD_ARGUMENT data)
{
    void **cbData = (void **)data;
    CallbackManager *obj = (CallbackManager *)cbData[0];
    delete [] cbData;
    obj->Work();
    return NULL;
}

// ****************************************************************************
// Method: CallbackManager::StartWork
//
// Purpose: 
//   This method starts the worker thread if we don't already have one.
//
// Note:       Called from thread 2.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:57:34 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
CallbackManager::StartWork()
{
    if(working)
        return;

    // Create callback data.
    void **cbData = new void*[1];
    cbData[0] = (void*)this;

    if(!threading->CreateThread(work_callback, (THREAD_ARGUMENT)cbData))
    {
        delete [] cbData;
        cerr << "VisIt: Error - Could not create work thread." << endl;
    }
}

// ****************************************************************************
// Method: CallbackManager::Work
//
// Purpose: 
//   This method does the job of processing work items in the queue and 
//   dispatching to the associated user-defined Python handler functions.
//
// Note:       Called from thread 3
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 11:58:43 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
CallbackManager::Work()
{
    working = true;
    bool keepWorking = working;

    // Lock the Python interpreter
    PyThreadState *threadState = VisItLockPythonInterpreter();

    do
    {
        WorkItem      w;
        CallbackData  cb;
        Subject      *key = 0;
        // Get an item of work.
        threading->MUTEX_LOCK();
        if(work.size() > 0)
        {
            w = work.front();
            work.pop_front();
            key = (Subject *)w.key;
            cb = callbacks[key];
        }
        else
            keepWorking = false;
        threading->MUTEX_UNLOCK();

        // Call the function that handles the state object.
        if(keepWorking)
        {
            if(cb.handler != 0)
            {
                CallbackHandlerData cbData;
                cbData.pycb = cb.pycb;
                cbData.data = w.data;
                cbData.userdata = cb.handler_data;
                (*cb.handler)(key, (void *)&cbData);
            }

            // When the work was added to the work queue, we created
            // a copy of the data. Delete that now that we've processed it.
            delete w.data;
        }
    } while(keepWorking);

    // Unlock the Python intrepter.
    VisItUnlockPythonInterpreter(threadState);

    // Indicate that we're no longer working and the thread will soon exit.
    threading->MUTEX_LOCK();
    working = false;
    threading->MUTEX_UNLOCK();
}

// ****************************************************************************
// Method: CallbackManager::RegisterHandler
//
// Purpose: 
//   Registers a C++ callback function for a state object.
//
// Arguments:
//   subj         : The subject for which we're installing a callback function.
//   cbName       : The name of the callback function that we're installing. This
//                  value is used later in GetCallbackNames.
//   handler      : The callback function to be called when the subject updates.
//   handler_data : Data to be passed to the handler function.
//   addwork      : The callback function to be called when determining whether
//                  the update merits adding to the work queue.
//   addwork_data : Data to be passed to the addwork callback function.
//
// Returns:    
//
// Note:       Called from thread 1.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 12:00:47 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void 
CallbackManager::RegisterHandler(Subject *subj, const std::string &cbName, 
    ObserverCallback *handler, void *handler_data,
    AddWorkCallback  *addwork, void *addwork_data)
{
    threading->MUTEX_LOCK();

    // Attach the subject. It will only happen once, even if we call this
    // routine multiple times.
    subj->Attach(this);

    CallbackData entry;   
    entry.name = cbName;
    entry.handler = handler;
    entry.handler_data = handler_data;
    entry.addwork = addwork;
    entry.addwork_data = addwork_data;
    entry.pycb = 0;
    callbacks[subj] = entry;

    nameToSubject[cbName] = subj;

    debug5 << "Adding callback handler for " << cbName.c_str() << endl;

    threading->MUTEX_UNLOCK();
}

// ****************************************************************************
// Method: CallbackManager::RegisterCallback
//
// Purpose: 
//   Registers a user-defined Python callback for a subject.
//
// Arguments:
//   subj : The subject
//   pycb : The user-defined Python callback to be executed when the subject
//          updates.
//
// Returns:    True on success; False otherwise.
//
// Note:       Called from thread 1.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 12:05:37 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
CallbackManager::RegisterCallback(Subject *subj, PyObject *pycb)
{
    bool retval = false;
    threading->MUTEX_LOCK();

    SubjectCallbackDataMap::iterator it = callbacks.find(subj);
    if(it != callbacks.end())
    {
        if(PyCallable_Check(pycb))
        {
            if(it->second.pycb != 0)
                Py_DECREF(it->second.pycb);

            Py_INCREF(pycb);
            callbacks[subj].pycb = pycb;
            retval = true;
        }
        else
            cerr << "RegisterCallback called with uncallable object" << endl;
    }
    else
        cerr << "RegisterCallback called with invalid subject." << endl;

    threading->MUTEX_UNLOCK();
    return retval;
}

// ****************************************************************************
// Method: CallbackManager::RegisterCallback
//
// Purpose: 
//   Registers a user-defined Python callback for a subject.
//
// Arguments:
//   name : The name of the subject.
//   pycb : The user-defined Python callback to be executed when the subject
//          updates.
//
// Returns:    True on success; False otherwise.
//
// Note:       Called from thread 1.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 12:05:37 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
CallbackManager::RegisterCallback(const std::string &name, PyObject *pycb)
{
    bool retval = false;
    StringSubjectMap::iterator it = nameToSubject.find(name);
    if(it != nameToSubject.end())
    {
        retval = RegisterCallback(it->second, pycb);
    }
    else
        cerr << "Could not register callback " << name.c_str() << endl;
    return retval;
}

// ****************************************************************************
// Method: CallbackManager::GetCallbackNames
//
// Purpose: 
//   Gets the names of the supported callback objects.
//
// Arguments:
//   names : The return vector of names.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 12:07:15 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
CallbackManager::GetCallbackNames(stringVector &names) const
{
    for(SubjectCallbackDataMap::const_iterator it = callbacks.begin();
        it != callbacks.end(); ++it)
    {
        if(it->second.handler != 0 && it->second.name.size() > 0)
            names.push_back(it->second.name);
    }
}

// ****************************************************************************
// Method: CallbackManager::GetCallback
//
// Purpose: 
//   Returns the callback for a specific subject.
//
// Arguments:
//   subj : The subject whose callback we want returned.
//
// Returns:    The user-defined Python callback for the subject -- or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb  5 12:07:52 PST 2008
//
// Modifications:
//   
// ****************************************************************************

PyObject *
CallbackManager::GetCallback(Subject *subj) const
{
    PyObject *retval = 0;
    SubjectCallbackDataMap::const_iterator it = callbacks.find(subj);
    if(it != callbacks.end())
        retval = it->second.pycb;
    return retval;
}
