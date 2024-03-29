// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

    void THREAD_INIT()   { tid = INVALID_HANDLE_VALUE; }
    void MUTEX_CREATE()  { InitializeCriticalSection(&mutex); }
    void MUTEX_DESTROY() { }
    void MUTEX_LOCK()    { EnterCriticalSection(&mutex); }
    void MUTEX_UNLOCK()  { LeaveCriticalSection(&mutex); }

    bool CreateThread(THREAD_RETURN thread_cb (THREAD_ARGUMENT), THREAD_ARGUMENT cbData)
    {
        // Create the thread with the WIN32 API.
        DWORD Id;
        tid = ::CreateThread(0, 0, thread_cb, cbData, 0, &Id);
        return (tid != INVALID_HANDLE_VALUE);
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

CallbackManager::CallbackManager(ViewerProxy *p) : SimpleObserver(), 
    viewer(p), nameToSubject(), callbacks(), work()
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
//   Brad Whitlock, Wed Feb  6 10:28:58 PST 2008
//   Added support for callback data.
//
//   Brad Whitlock, Thu Aug 19 15:52:59 PDT 2010
//   Be careful about deleting a class instance from here.
//
//   Cyrus Harrison, Fri Mar 20 16:01:38 PDT 2020
//   Python 2 and 3 support.
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
        if(it->second.pycb_data != 0)
        {
#ifdef Py_REFCNT
            // Hack! I found that decrementing the refcount of a class 
            //       instance from here causes a crash if it's the last
            //       reference and it will cause the object to be deleted.
            //       Decrementing the refcount on other object types is fine,
            //       even if it causes the object to get deleted. Other objects
            //       work okay so our reference counting seems good.
            bool lastInstance = Py_REFCNT(it->second.pycb_data) == 1 &&
            // python 2 ==> 3 use PyType_Check instead of PyInstance_Check
                                          PyType_Check(it->second.pycb_data);
            if(!lastInstance)
#endif
                Py_DECREF(it->second.pycb_data);
        }
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
//  Method: CallbackManager::IsWorking
//
//  Purpose:
//     Method to describe whether the CLI is still working.  
//     (Used for VisTrails)
//
//  Programmer: Tila Ochatta
//  Creation:   May 26, 2009
//
// ****************************************************************************

bool
CallbackManager::IsWorking()
{
  return this->working;
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
//   Brad Whitlock, Wed Feb  6 10:29:21 PST 2008
//   Added support for callback data.
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
        if(it->second.pycb_data != 0)
            Py_DECREF(it->second.pycb_data);
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
//   Kathleen Bonnell, Thu Mar  6 09:04:09 PST 2008
//   Add windows-specifics.
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

#ifndef _WIN32
    if(!threading->CreateThread(work_callback, (THREAD_ARGUMENT)cbData))
#else
    if(!threading->CreateThread(&work_callback, (THREAD_ARGUMENT)cbData))
#endif
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
//   Brad Whitlock, Wed Feb  6 10:30:04 PST 2008
//   Added support for callback data.
//
//   Brad Whitlock, Tue Jun 24 14:20:53 PDT 2008
//   Pass the viewer proxy pointer to the callback.
//
//   Cyrus Harrison, Fri Feb 19 13:25:57 PST 2021
//   Update to use new VisItLockPythonInterpreter signature. 
//
// ****************************************************************************

void
CallbackManager::Work()
{
    working = true;
    bool keepWorking = working;

    // Lock the Python interpreter
    VISIT_PY_THREAD_LOCK_STATE threadState = VisItLockPythonInterpreter();

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
                cbData.pycb_data = cb.pycb_data;
                cbData.data = w.data;
                cbData.userdata = cb.handler_data;
                cbData.viewer = viewer;
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
//   Brad Whitlock, Wed Feb  6 10:31:09 PST 2008
//   Added support for callback data.
//
// ****************************************************************************

void 
CallbackManager::RegisterHandler(Subject *subj, const std::string &cbName,
    ObserverCallback *handler, void *handler_data,
    AddWorkCallback  *addwork, void *addwork_data)
{

    threading->MUTEX_LOCK();

    /// if name already exists then skip
    /// TODO: Hari (Hack): For PySide both cli and viewer register, this 
    /// structure does not correctly handle this registration properly.
    /// I am temporarily making the first one cbName the only attachment.
    /// Note: I don't believe this changes the traditional execution of VisIt.
    if(nameToSubject.count(cbName) > 0)
    {
        threading->MUTEX_UNLOCK();
        return;
    }
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
    entry.pycb_data = 0;
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
//   Brad Whitlock, Wed Feb  6 10:31:27 PST 2008
//   Added support for callback data.
//
// ****************************************************************************

bool
CallbackManager::RegisterCallback(Subject *subj, PyObject *pycb, PyObject *pycb_data)
{
    bool retval = false;
    threading->MUTEX_LOCK();

    SubjectCallbackDataMap::iterator it = callbacks.find(subj);
    if(it != callbacks.end())
    {
        if(it->second.pycb != 0)
            Py_DECREF(it->second.pycb);
        if(it->second.pycb_data != 0)
            Py_DECREF(it->second.pycb_data);

        if(pycb != 0)
            Py_INCREF(pycb);
        if(pycb_data != 0)
            Py_INCREF(pycb_data);

        callbacks[subj].pycb = pycb;
        callbacks[subj].pycb_data = pycb_data;
        retval = true;
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
//   Brad Whitlock, Wed Feb  6 10:32:46 PST 2008
//   Added support for callback data.
//
// ****************************************************************************

bool
CallbackManager::RegisterCallback(const std::string &name, PyObject *pycb, 
    PyObject *pycb_data)
{
    bool retval = false;
    StringSubjectMap::iterator it = nameToSubject.find(name);
    if(it != nameToSubject.end())
    {
        retval = RegisterCallback(it->second, pycb, pycb_data);
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
