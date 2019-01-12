#if defined(VISIT_THREADS)

#include <stdlib.h>
#include <DebugStream.h>
#include <VisItInit.h>
#include "avtExecutionManager.h"


// Singleton for the thread pool. Use macro avtExecutionManager to access it.
avtExecutionManager *avtExecutionManager::_avtExecutionManagerPtr = NULL;

avtExecutionManager * avtExecutionManager::GetAvtThreadPool()
{
    if( _avtExecutionManagerPtr == NULL )
    {
        atexit( avtExecutionManager::DeleteAvtThreadPool );

        _avtExecutionManagerPtr = new avtExecutionManager();
    }
    return( _avtExecutionManagerPtr );
}

void avtExecutionManager::DeleteAvtThreadPool()
{
    if( _avtExecutionManagerPtr )
    {
        delete _avtExecutionManagerPtr;
        _avtExecutionManagerPtr = NULL;
    }
}

avtExecutionManager::avtExecutionManager()
{
    tPool = NULL;
    MutexInit( &mutexMapLock );
    numThreads = 0;
}

avtExecutionManager::~avtExecutionManager()
{
    MutexDestroy( &mutexMapLock, false );

    // Destroy any mutexes in the list.
    std::map<MUTEX_ID, MUTEX *>::iterator it;
    for(it=mutexMap.begin(); it != mutexMap.end(); ++it)
    {
        MutexDestroy( it->second );
    }
}

void avtExecutionManager::SetNumberOfThreads( int nt )
{
    if( nt == 0 )
    {
        numThreads = 4;
    }
    else
    {
        numThreads = nt;
    }

    tPool = ThreadPool::Create( numThreads, (numThreads * 100), false );
    if( tPool == NULL )
    {
        // Failed to create threads, we need to exit.
        // TODO: I think we need to throw an error.
    }
    else
    {
        VisItInit::SetNumberOfThreads(numThreads);
    }
}
 
void avtExecutionManager::ScheduleWork( void (*workerThreadFunction)(void *), void *cbdata )
{
    if (tPool == NULL)
    {
        workerThreadFunction(cbdata);
    }
    else
    {
        tPool->AddWork( workerThreadFunction, cbdata );
    }
}
 
void avtExecutionManager::FinishWork()
{
    if (tPool == NULL)
        return;

    tPool->JoinNoExit();
}
 
MUTEX * avtExecutionManager::FindMutex( const MUTEX_ID id )
{
    if (tPool == NULL)
        return NULL;

    std::map<MUTEX_ID, MUTEX *>::iterator it;
    MUTEX *lock;

    MutexLock( &mutexMapLock );

    it = mutexMap.find( id );
    if( it == mutexMap.end() )
    {
        // Not found, create it.
        lock = new MUTEX;
        MutexInit( lock );

        mutexMap.insert( std::pair<MUTEX_ID, MUTEX *>(id, lock) );
    }
    else
    {
        lock = it->second;
    }

    MutexUnlock( &mutexMapLock );

    return( lock );
}

MUTEX * avtExecutionManager::RemoveMutex( const MUTEX_ID id )
{
    if (tPool == NULL)
        return NULL;

    std::map<MUTEX_ID, MUTEX *>::iterator it;
    MUTEX *lock;

    MutexLock( &mutexMapLock );

    it = mutexMap.find( id );
    if( it != mutexMap.end() )
    {
        // Remove from list.
        lock = it->second;
        mutexMap.erase( it );
    }

    MutexUnlock( &mutexMapLock );

    return( lock );
}

void avtExecutionManager::MutexLock( const MUTEX_ID stringID )
{
    if (tPool == NULL)
        return;

    MutexLock( FindMutex(stringID) );
}

void avtExecutionManager::MutexUnlock( const MUTEX_ID stringID )
{
    if (tPool == NULL)
        return;

    MutexUnlock( FindMutex(stringID) );
}

void avtExecutionManager::MutexDestroy( const MUTEX_ID stringID )
{
    if (tPool == NULL)
        return;

    MutexDestroy( RemoveMutex(stringID) );
}
 
void avtExecutionManager::MutexInit( MUTEX *lock )
{
    if (tPool == NULL)
        return;

    int rtn;
    if( (rtn = pthread_mutex_init(lock, NULL)) )
    {
        debug1 << "pthread_mutex_init: " << strerror(rtn);
    }
}

void avtExecutionManager::MutexDestroy( MUTEX *lock, bool delMutex )
{
    if (tPool == NULL)
        return;

    int rtn;
    if( (rtn = pthread_mutex_destroy(lock)) )
    {
        debug1 << "pthread_mutex_destroy: " << strerror(rtn);
    }
    if( delMutex )
        delete lock;
}

void avtExecutionManager::MutexLock( MUTEX *lock )
{
    if (tPool == NULL)
        return;

    int rtn;
    if( (rtn = pthread_mutex_lock(lock)) )
    {
        debug1 << "pthread_mutex_lock: " << strerror(rtn);
    }
}

void avtExecutionManager::MutexUnlock( MUTEX *lock )
{
    if (tPool == NULL)
        return;

    int rtn;
    if( (rtn = pthread_mutex_unlock(lock)) )
    {
        debug1 << "pthread_mutex_unlock: " << strerror(rtn);
    }
}

void avtExecutionManager::CondInit( COND *cond )
{
    if (tPool == NULL)
        return;

    int rtn;
    if( (rtn = pthread_cond_init(cond, NULL)) )
    {
        debug1 << "pthread_cond_init: " << strerror(rtn);
    }
}

void avtExecutionManager::CondDestroy( COND *cond )
{
    if (tPool == NULL)
        return;

    int rtn;
    if( (rtn = pthread_cond_destroy(cond)) )
    {
        debug1 << "pthread_cond_destroy: " << strerror(rtn);
    }
}

void avtExecutionManager::CondSignal( COND *cond )
{
    if (tPool == NULL)
        return;

    int rtn;
    if( (rtn = pthread_cond_signal(cond)) )
    {
        debug1 << "pthread_cond_signal: " << strerror(rtn);
    }
}

void avtExecutionManager::CondWait( COND *cond, MUTEX *lock )
{
    if (tPool == NULL)
        return;

    int rtn;
    if( (rtn = pthread_cond_wait(cond, lock)) )
    {
        debug1 << "pthread_cond_wait: " << strerror(rtn);
    }
}

#endif // VISIT_THREADS

