/***********************************************************************
    ThreadPool.cpp
*/
#if defined(VISIT_THREADS)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "ThreadPool.h"
#include <DebugStream.h>
#include <VisItInit.h>

void *ThreadPoolThread( void * );

struct ThreadInfo
{
    ThreadPool *tpool;
    int id;
};

int
GetPthreadIDCallback(void *a)
{
    ThreadPool *tp = (ThreadPool *) a;
    return tp->GetPthreadID();
}

int
ThreadPool::GetPthreadID()
{
    pthread_t my_id = pthread_self();
    if (my_id == mainThread)
        return 0;
    for (int i = 0 ; i < numberThreads ; i++)
    {
        if (threads[i] == my_id)
            return i;
    }
    return -1; // ?? ... and don't issue debug statement, since we would get infinite recursion
}

/***********************************************************************
    return:
        pointer to a new ThreadPool.
*/
ThreadPool * ThreadPool::Create( int numWorkerThreads, int maxQueueSize, int doNotBlockWhenFull )
{
    int        rtn;
    ThreadPool *tpool;

    // Allocate a pool data structure.
    if( (tpool = new ThreadPool) == NULL )
    {
        debug1 << "ThreadPool new failed" << endl;
        return( NULL );
    }

    // Initialize the fields.
    tpool->mainThread         = pthread_self();
    tpool->numberThreads      = numWorkerThreads;
    tpool->maxQueueSize       = maxQueueSize;
    tpool->doNotBlockWhenFull = doNotBlockWhenFull;
    if( (tpool->threads = (pthread_t *)malloc(sizeof(pthread_t) * numWorkerThreads)) == NULL )
    {
        debug1 << "ThreadPool malloc failed" << endl;
        delete( tpool );
        return( NULL );
    }
    tpool->currentQueueSize = 0;
    tpool->queueHead        = NULL;
    tpool->queueTail        = NULL;
    tpool->queueClosed      = 0;
    tpool->shutdown         = 0;
    if( (rtn = pthread_mutex_init(&(tpool->queueLock), NULL)) != 0 )
    {
        debug1 << "pthread_mutex_init " << strerror(rtn) << endl;
        delete( tpool );
        return( NULL );
    }
    if( (rtn = pthread_cond_init(&(tpool->queueNotEmpty), NULL)) != 0 )
    {
        debug1 << "pthread_cond_init queueNotEmpty: " << strerror(rtn) << endl;
        delete( tpool );
        return( NULL );
    }
    if( (rtn = pthread_cond_init(&(tpool->queueNotFull), NULL)) != 0 )
    {
        debug1 << "pthread_cond_init queueNotFull: " << strerror(rtn) << endl;
        delete( tpool );
        return( NULL );
    }
    if( (rtn = pthread_cond_init(&(tpool->queueEmpty), NULL)) != 0 )
    {
        debug1 << "pthread_cond_init queueEmpty: " << strerror(rtn) << endl;
        delete( tpool );
        return( NULL );
    }

    // Create threads.
    for(int i=0; i < numWorkerThreads ;i++)
    {
        ThreadInfo *tI = new ThreadInfo;
        tI->id = i + 1;
        tI->tpool = tpool;
        if( (rtn = pthread_create( &(tpool->threads[i]), NULL, ThreadPoolThread, (void *)tI)) != 0 )
        {
            debug1 << "pthread_create: " << strerror(rtn) << endl;
            delete( tpool );
            return( NULL );
        }
    }

    VisItInit::RegisterThreadIDFunction(GetPthreadIDCallback, tpool);

    return( tpool );
}

/***********************************************************************
return:
 0 if everything work
-1 Max queue and did not wait to add to queue
# will be the error code.
*/
int ThreadPool::AddWork( void (*workerThreadFunction)(void *), void *arg )
{
    int             rtn;
    ThreadPoolWork *workp;

    // Allocate work structure.
    if( (workp = (ThreadPoolWork *)malloc(sizeof(ThreadPoolWork))) == NULL )
    {
        debug1 << "AddWork malloc work pool. " << endl;
        return( 5 );
    }
    workp->routine = workerThreadFunction;
    workp->arg     = arg;
    workp->next    = NULL;

    if( (rtn = pthread_mutex_lock(&queueLock)) != 0 )
    {
        debug1 << "AddWork pthread_mutex_lock: " << strerror(rtn) << endl;
        return( 1 );
    }

    // No space and this caller doesn't want to wait.
    if( currentQueueSize == maxQueueSize )
    {
        if( doNotBlockWhenFull )
        {
            free( workp );
            if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
            {
                debug1 << "AddWork pthread_mutex_unlock: " << strerror(rtn) << endl;
                return( 2 );
            }

            // User did not want to wait to add the work.
            return( -1 );
        }
        else
        {
            while( (currentQueueSize == maxQueueSize) && (! (shutdown || queueClosed)) )
            {
                if( (rtn = pthread_cond_wait(&queueNotFull, &queueLock)) != 0 )
                {
                    debug1 << "AddWork pthread_cond_wait: " << strerror(rtn) << endl;
                    return( 3 );
                }
            }
        }
    }

    // the pool is in the process of being destroyed.
    if( shutdown || queueClosed )
    {
        free( workp );
        if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
        {
            debug1 << "AddWork pthread_mutex_unlock: " << strerror(rtn) << endl;
            return( 4 );
        }

        return( -1 );
    }

    if( currentQueueSize == 0 )
    {
        queueTail = queueHead = workp;
    }
    else
    {
        queueTail->next = workp;
        queueTail       = workp;
    }

    currentQueueSize++;

    if( (rtn = pthread_cond_broadcast(&queueNotEmpty)) != 0 )
    {
        debug1 << "AddWork pthread_cond_broadcast: " << strerror(rtn) << endl;
        return( 6 );
    }
    if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
    {
        debug1 << "AddWork pthread_mutex_unlock: " << strerror(rtn) << endl;
        return( 7 );
    }

    return( 0 );
}

/***********************************************************************
return:
0 if everything work
# will be the error code.
*/
int ThreadPool::Destroy( int finish )
{
    int             rtn;
    ThreadPoolWork *cur_nodep;

    if( (rtn = pthread_mutex_lock(&queueLock)) != 0 )
    {
        debug1 << "Destroy pthread_mutex_lock: " << strerror(rtn) << endl;
        return( 1 );
    }

    // Is a shutdown already in progress?
    if( queueClosed || shutdown )
    {
        if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
        {
            debug1 << "Destroy pthread_mutex_unlock: " << strerror(rtn) << endl;
            return( 1 );
        }
        return( 0 );
    }

    queueClosed = 1;

    // If the finish flag is set, wait for workers to drain queue.
    if( finish )
    {
        while( currentQueueSize )
        {
            if( (rtn = pthread_cond_wait(&queueEmpty, &queueLock)) != 0 )
            {
                debug1 << "Destroy pthread_cond_wait: " << strerror(rtn) << endl;
                return( 1 );
            }
        }
    }

    shutdown = 1;

    if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
    {
        debug1 << "Destroy pthread_mutex_unlock: " << strerror(rtn) << endl;
        return( 1 );
    }

    // Wake up any workers so they recheck shutdown flag.
    if( (rtn = pthread_cond_broadcast(&queueNotEmpty)) != 0 )
    {
        debug1 << "Destroy pthread_cond_broadcast: " << strerror(rtn) << endl;
        return( 1 );
    }
    if( (rtn = pthread_cond_broadcast(&queueNotFull)) != 0 )
    {
        debug1 << "Destroy pthread_cond_broadcast queueNotFull: " << strerror(rtn) << endl;
        return( 1 );
    }

    // Wait for workers to exit.
    for(int i=0; i < numberThreads ;i++)
    {
        if( (rtn = pthread_join(threads[i], NULL)) != 0 )
        {
            debug1 << "Destroy pthread_join: " << strerror(rtn) << endl;
            return( 1 );
        }
    }

    // Now free pool structures.
    free( threads );
    while( queueHead != NULL )
    {
        cur_nodep = queueHead->next;
        queueHead = queueHead->next;
        free( cur_nodep );
    }

    if( (rtn = pthread_mutex_destroy(&queueLock)) != 0 )
    {
        debug1 << "Destroy pthread_mutex_destroy: " << strerror(rtn) << endl;
    }
    if( (rtn = pthread_cond_destroy(&queueNotEmpty)) != 0 )
    {
        debug1 << "Destroy pthread_cond_destroy: " << strerror(rtn) << endl;
    }
    if( (rtn = pthread_cond_destroy(&queueNotFull)) != 0 )
    {
        debug1 << "Destroy pthread_cond_destroy: " << strerror(rtn) << endl;
    }
    if( (rtn = pthread_cond_destroy(&queueEmpty)) != 0 )
    {
        debug1 << "Destroy pthread_cond_destroy: " << strerror(rtn) << endl;
    }

    delete this;

    return( 0 );
}

//////////////////////////////////////////////////////////////////////////
// Wait for all threads to exit.
//////////////////////////////////////////////////////////////////////////
int ThreadPool::Join()
{
    int rtn;

    // Wait for workers to exit.
    for(int i=0; i < numberThreads ;i++)
    {
        if( (rtn = pthread_join(threads[i], NULL)) != 0 )
        {
            debug1 << "Join pthread_join: " << strerror(rtn) << endl;
            return( 1 );
        }
    }
    numberThreads = 0;
    return( 0 );
}

int ThreadPool::JoinNoExit()
{
    int rtn;

    if( (rtn = pthread_mutex_lock(&queueLock)) != 0 )
    {
        debug1 << "JoinNoExit pthread_mutex_lock: " << strerror(rtn) << endl;
        return( 1 );
    }

    while( currentQueueSize || threadWorking.to_ulong() || threadWorking2.to_ulong() || threadWorking3.to_ulong() || threadWorking4.to_ulong())
    {
        if( (rtn = pthread_cond_wait(&queueEmpty, &queueLock)) != 0 )
        {
            debug1 << "JoinNoExit pthread_cond_wait: " << strerror(rtn) << endl;
            return( 1 );
        }
    }

    if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
    {
        debug1 << "JoinNoExit pthread_mutex_unlock: " << strerror(rtn) << endl;
        return( 1 );
    }
    return( 0 );
}

#if defined(__APPLE__)

#define TURN_OFF_THREAD_SET_AFFINITY 1

#endif

/***********************************************************************
  The ThreadPool will create threads that will use this function to
process the work queue.
*/
void *ThreadPoolThread( void *arg )
{
    ThreadInfo *tI = (ThreadInfo *)arg;
    ThreadPool *tpool = tI->tpool;
    int rtn;
    ThreadPool::ThreadPoolWork  *my_workp;

#ifndef TURN_OFF_THREAD_SET_AFFINITY
    cpu_set_t cpuset;
    CPU_ZERO( &cpuset );
#ifdef __KNC__
    int numCores = sysconf( _SC_NPROCESSORS_ONLN );
    CPU_SET(((tI->id-1)*4)%numCores + ((tI->id-1)*4)/numCores + 1, &cpuset);
#else
    CPU_SET( (tI->id - 1), &cpuset );
#endif

    if( (rtn = pthread_setaffinity_np( pthread_self(), sizeof(cpu_set_t), &cpuset )) != 0 )
    {
        debug1 << "ThreadPoolThread pthread_setaffinity_np: " << strerror(rtn) << endl;
    }
#endif

    while( true )
    {
        // Check queue for work.
        if( (rtn = pthread_mutex_lock(&(tpool->queueLock))) != 0 )
        {
            debug1 << "ThreadPoolThread pthread_mutex_lock: " << strerror(rtn) << endl;
            return( NULL );
        }

        // If no work, wait for new work.
        while( (tpool->currentQueueSize == 0) && (! tpool->shutdown) )
        {
            // Check if we should send the empty queue message.
           if( tI->id < 64) 
               tpool->threadWorking[tI->id] = 0;
           else if( tI->id <128) 
               tpool->threadWorking2[tI->id-64] = 0;
           else if( tI->id < 192) 
               tpool->threadWorking3[tI->id-128] = 0;
           else if( tI->id < 256) 
               tpool->threadWorking4[tI->id-192] = 0;
            if( tpool->threadWorking.to_ulong() == 0 && tpool->threadWorking2.to_ulong() == 0 && tpool->threadWorking3.to_ulong() == 0 && tpool->threadWorking3.to_ulong() == 0 )
            {
                if( (rtn = pthread_cond_signal(&(tpool->queueEmpty))) != 0 )
                {
                    debug1 << "ThreadPoolThread pthread_cond_signal: " << strerror(rtn) << endl;
                    return( NULL );
                }
            }

            if( (rtn = pthread_cond_wait(&(tpool->queueNotEmpty), &(tpool->queueLock))) != 0 )
            {
                debug1 << "ThreadPoolThread pthread_cond_wait: " << strerror(rtn) << endl;
                return( NULL );
            }
        }

        // Has a shutdown started while I was sleeping?
        if( tpool->shutdown )
        {
            if( (rtn = pthread_mutex_unlock(&(tpool->queueLock))) != 0 )
            {
                debug1 << "ThreadPoolThread pthread_mutex_unlock: " << strerror(rtn) << endl;
                return( NULL );
            }

            delete tI;
            return( NULL );
        }

        // Get to work, dequeue the next item.
        my_workp = tpool->queueHead;
        --tpool->currentQueueSize;
        if( tpool->currentQueueSize == 0 )
            tpool->queueHead = tpool->queueTail = NULL;
        else
            tpool->queueHead = my_workp->next;

        // Set that this thread is working.
        if( tI->id < 64) 
            tpool->threadWorking[tI->id] = 1;
        else if( tI->id <128) 
            tpool->threadWorking2[tI->id-64] = 1;
        else if( tI->id < 192) 
            tpool->threadWorking3[tI->id-128] = 1;        
        else if( tI->id < 256) 
            tpool->threadWorking4[tI->id-192] = 1;

        // Handle waiting add_work threads.
        if( (!tpool->doNotBlockWhenFull) && (tpool->currentQueueSize == (tpool->maxQueueSize - 1)) )
        {
            if ((rtn = pthread_cond_broadcast(&(tpool->queueNotFull))) != 0)
            {
                debug1 << "ThreadPoolThread pthread_cond_broadcast: " << strerror(rtn) << endl;
                return( NULL );
            }
        }

        if( (rtn = pthread_mutex_unlock(&(tpool->queueLock))) != 0 )
        {
            debug1 << "ThreadPoolThread pthread_mutex_unlock: " << strerror(rtn) << endl;
            return( NULL );
        }


        // Do this work item.
        my_workp->routine( my_workp->arg );

        // The "routine" function above will delete the arg.
        //delete my_workp->arg;
        free( my_workp );
    }

    return( NULL );
}

void *ThreadPoolWorker::GetWorkItem( ThreadPool *tpool )
{
    ThreadPool::ThreadPoolWork  *my_workp = NULL;

    pthread_mutex_lock( &(tpool->queueLock) );
    if( tpool->currentQueueSize )
    {
        // Get to work, dequeue the next item.
        my_workp = tpool->queueHead;
        tpool->currentQueueSize--;
        if( tpool->currentQueueSize == 0 )
            tpool->queueHead = tpool->queueTail = NULL;
        else
            tpool->queueHead = my_workp->next;
    }
    pthread_mutex_unlock( &(tpool->queueLock) );

    void *rnt;
    if( my_workp )
    {
        rnt = my_workp->arg;
        free( my_workp );
    }
    else
    {
        rnt = NULL;
    }

    return( rnt );
}

#endif // VISIT_THREADS

