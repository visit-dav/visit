#ifndef AVT_THREAD_POOL_H
#define AVT_THREAD_POOL_H

#if defined(VISIT_THREADS)

#include <pipeline_exports.h>
#include <map>

// Defines thread pool and MUTEX, COND, MUTEX_ID.
#include "ThreadPool.h"

// Convert strings to numbers, used for the Mutex functions.
#include "StringHash.h"


class PIPELINE_API avtExecutionManager
{
  public:
    static avtExecutionManager * GetAvtThreadPool();

    void SetNumberOfThreads(int nt);
 
    // Add some work to the queue.
    void ScheduleWork(void (*workerThreadFunction)(void *), void *cbdata);
 
    // Wait for all work in the queue to be done.
    void FinishWork();
 
    // Use the VisitMutex... macros below for strings.
    // Create named mutex if it does not exist.
    void MutexLock(const MUTEX_ID stringID);
    void MutexUnlock(const MUTEX_ID stringID);
    void MutexDestroy(const MUTEX_ID stringID);
 
  private:
    avtExecutionManager();
    ~avtExecutionManager();

    static void DeleteAvtThreadPool();

    // Not Implemented. Don't want the default copy functions.
    avtExecutionManager( avtExecutionManager const& copy );
    avtExecutionManager& operator=( avtExecutionManager const& copy );

    // Mutex Map functions
    MUTEX *FindMutex( const MUTEX_ID id );
    MUTEX *RemoveMutex( const MUTEX_ID id );

    // Mutex functions
    void MutexInit( MUTEX *lock );
    void MutexDestroy( MUTEX *lock, bool delMutex = true );
    void MutexLock( MUTEX *lock );
    void MutexUnlock( MUTEX *lock );

    // Cond functions
    void CondInit( COND *cond );
    void CondDestroy( COND *cond );
    void CondSignal( COND *cond );
    void CondWait( COND *cond, MUTEX *lock );
 
  private:
    // Global pointer to the execution operator.
    static avtExecutionManager    *_avtExecutionManagerPtr;

    ThreadPool                    *tPool;

    std::map<MUTEX_ID, MUTEX*>     mutexMap;
    MUTEX                          mutexMapLock;

    unsigned int                   numThreads;
};

// With this define you can use avtExecutionManager-> and still use the single thread pool object.
#define avtExecutionManagerPtr avtExecutionManager::GetAvtThreadPool()

// Marco for mutex functions.
#define VisitMutexLock(name)    avtExecutionManagerPtr->MutexLock(StringHash(name).GetHash());
#define VisitMutexUnlock(name)  avtExecutionManagerPtr->MutexUnlock(StringHash(name).GetHash());
#define VisitMutexDestroy(name) avtExecutionManagerPtr->MutexDestroy(StringHash(name).GetHash());

#define VisitSetNumberOfThreads(num) avtExecutionManagerPtr->SetNumberOfThreads(num);

#define avtExecutionManagerScheduleWork(fun, arg) \
    avtExecutionManagerPtr->avtExecutionManagerPtr->ScheduleWork(fun, arg);

#define avtExecutionManagerFinishWork() avtExecutionManagerPtr->FinishWork()

#else

#define VisitMutexLock(name)
#define VisitMutexUnlock(name)
#define VisitMutexDestroy(name)

#define VisitSetNumberOfThreads(num)

#define avtExecutionManagerScheduleWork(fun, arg) fun(arg)
#define avtExecutionManagerFinishWork()

#endif // VISIT_THREADS

#endif // AVT_THREAD_POOL_H

