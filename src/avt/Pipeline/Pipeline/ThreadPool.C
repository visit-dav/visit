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

void *ThreadPoolThread( void * );

struct ThreadInfo
{
    ThreadPool *tpool;
    int id;
};

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
        fprintf( stderr, "ThreadPool new failed" );
        return( NULL );
    }

    // Initialize the fields.
    tpool->numberThreads      = numWorkerThreads;
    tpool->maxQueueSize       = maxQueueSize;
    tpool->doNotBlockWhenFull = doNotBlockWhenFull;
    if( (tpool->threads = (pthread_t *)malloc(sizeof(pthread_t) * numWorkerThreads)) == NULL )
    {
        fprintf( stderr, "ThreadPool malloc failed" );
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
        fprintf( stderr, "pthread_mutex_init %s", strerror(rtn) );
        delete( tpool );
        return( NULL );
    }
    if( (rtn = pthread_cond_init(&(tpool->queueNotEmpty), NULL)) != 0 )
    {
        fprintf( stderr, "pthread_cond_init %s", strerror(rtn) );
        delete( tpool );
        return( NULL );
    }
    if( (rtn = pthread_cond_init(&(tpool->queueNotFull), NULL)) != 0 )
    {
        fprintf( stderr, "pthread_cond_init %s", strerror(rtn) );
        delete( tpool );
        return( NULL );
    }
    if( (rtn = pthread_cond_init(&(tpool->queueEmpty), NULL)) != 0 )
    {
        fprintf( stderr, "pthread_cond_init %s", strerror(rtn) );
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
            fprintf( stderr, "pthread_create %s", strerror(rtn) );
            delete( tpool );
            return( NULL );
        }
    }

    return( tpool );
}

/***********************************************************************
return:
 0 if everything work
-1 Max queue and did not wait to add to queue
# will be the error code.
*/
//int ThreadPool::AddWork( ThreadPoolWorker *worker, void *arg )
//int ThreadPool::AddWork( ThreadPoolWorker *worker, Data *data )
int ThreadPool::AddWork( void (*workerThreadFunction)(void *), void *arg )
{
    int             rtn;
    ThreadPoolWork *workp;

    if( (rtn = pthread_mutex_lock(&queueLock)) != 0 )
    {
        fprintf( stderr, "pthread_mutex_lock %s", strerror(rtn) );
        return( 1 );
    }

    // No space and this caller doesn't want to wait.
    if( currentQueueSize == maxQueueSize )
    {
        if( doNotBlockWhenFull )
        {
            if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
            {
                fprintf( stderr, "pthread_mutex_unlock %s", strerror(rtn) );
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
                    fprintf( stderr, "pthread_cond_wait %s", strerror(rtn) );
                    return( 3 );
                }
            }
        }
    }

    // the pool is in the process of being destroyed.
    if( shutdown || queueClosed )
    {
        if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
        {
            fprintf( stderr, "pthread_mutex_unlock %s", strerror(rtn) );
            return( 4 );
        }

        return( -1 );
    }

    // Allocate work structure.
    if( (workp = (ThreadPoolWork *)malloc(sizeof(ThreadPoolWork))) == NULL )
    {
        fprintf( stderr, "ThreadPoolWork Malloc failed" );
        return( 5 );
    }
    workp->routine = workerThreadFunction;
    workp->arg     = arg;
    workp->next    = NULL;

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
        fprintf( stderr, "pthread_cond_signal %s", strerror(rtn) );
        return( 6 );
    }
    if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
    {
        fprintf( stderr, "pthread_mutex_unlock %s", strerror(rtn) );
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
        fprintf( stderr, "pthread_mutex_lock %s", strerror(rtn) );
        return( 1 );
    }

    // Is a shutdown already in progress?
    if( queueClosed || shutdown )
    {
        if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
        {
            fprintf( stderr, "pthread_mutex_unlock %s", strerror(rtn) );
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
                fprintf( stderr, "pthread_cond_wait %s", strerror(rtn) );
                return( 1 );
            }
        }
    }

    shutdown = 1;

    if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
    {
        fprintf( stderr, "pthread_mutex_unlock %s", strerror(rtn) );
        return( 1 );
    }

    // Wake up any workers so they recheck shutdown flag.
    if( (rtn = pthread_cond_broadcast(&queueNotEmpty)) != 0 )
    {
        fprintf( stderr, "pthread_cond_broadcast %s", strerror(rtn) );
        return( 1 );
    }
    if( (rtn = pthread_cond_broadcast(&queueNotFull)) != 0 )
    {
        fprintf( stderr, "pthread_cond_broadcast %s", strerror(rtn) );
        return( 1 );
    }

    // Wait for workers to exit.
    for(int i=0; i < numberThreads ;i++)
    {
        if( (rtn = pthread_join(threads[i], NULL)) != 0 )
        {
            fprintf( stderr, "pthread_join %s", strerror(rtn) );
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
        fprintf( stderr, "pthread_mutex_destroy %s", strerror(rtn) );
    }
    /*
    if( (rtn = pthread_mutex_destroy(&totalTimeLock)) != 0 )
    {
        fprintf( stderr, "pthread_mutex_destroy %s", strerror(rtn) );
    }
    */
    if( (rtn = pthread_cond_destroy(&queueNotEmpty)) != 0 )
    {
        fprintf( stderr, "pthread_cond_destroy %s", strerror(rtn) );
    }
    if( (rtn = pthread_cond_destroy(&queueNotFull)) != 0 )
    {
        fprintf( stderr, "pthread_cond_destroy %s", strerror(rtn) );
    }
    if( (rtn = pthread_cond_destroy(&queueEmpty)) != 0 )
    {
        fprintf( stderr, "pthread_cond_destroy %s", strerror(rtn) );
    }

//fprintf( stderr, "Thread wait time: %e\n", GetTime() );

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
            fprintf( stderr, "pthread_join %s", strerror(rtn) );
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
        fprintf( stderr, "pthread_mutex_lock %s", strerror(rtn) );
        return( 1 );
    }

    while( currentQueueSize || threadWorking.to_ulong() )
    {
        if( (rtn = pthread_cond_wait(&queueEmpty, &queueLock)) != 0 )
        {
            fprintf( stderr, "pthread_cond_wait %s", strerror(rtn) );
            return( 1 );
        }
    }

    if( (rtn = pthread_mutex_unlock(&queueLock)) != 0 )
    {
        fprintf( stderr, "pthread_mutex_unlock %s", strerror(rtn) );
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
    CPU_SET( (tI->id - 1), &cpuset );
    if( (rtn = pthread_setaffinity_np( pthread_self(), sizeof(cpu_set_t), &cpuset )) != 0 )
    {
        fprintf( stderr, "pthread_setaffinity_np %s", strerror(rtn) );
    }
#endif

    while( true )
    {
        // Check queue for work.
        if( (rtn = pthread_mutex_lock(&(tpool->queueLock))) != 0 )
        {
            fprintf( stderr, "pthread_mutex_lock %s", strerror(rtn) );
            return( NULL );
        }

        // If no work, wait for new work.
        while( (tpool->currentQueueSize == 0) && (! tpool->shutdown) )
        {
            //printf( "worker %d: I'm sleeping again\n", pthread_self() );

            // Check if we should send the empty queue message.
            tpool->threadWorking[tI->id] = 0;
            if( tpool->threadWorking.to_ulong() == 0 )
            {
                if( (rtn = pthread_cond_signal(&(tpool->queueEmpty))) != 0 )
                {
                    fprintf( stderr, "pthread_cond_signal %s", strerror(rtn) );
                    return( NULL );
                }
            }

            if( (rtn = pthread_cond_wait(&(tpool->queueNotEmpty), &(tpool->queueLock))) != 0 )
            {
                fprintf( stderr, "pthread_cond_wait %s", strerror(rtn) );
                return( NULL );
            }
        }

        //fprintf( stderr, "worker %d: I'm awake\n", pthread_self() );

        // Has a shutdown started while I was sleeping?
        if( tpool->shutdown )
        {
            if( (rtn = pthread_mutex_unlock(&(tpool->queueLock))) != 0 )
            {
                fprintf( stderr, "pthread_mutex_unlock %s", strerror(rtn) );
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
        tpool->threadWorking[tI->id] = 1;

        //fprintf( stderr, "worker %d: dequeing item %d\n", pthread_self(), my_workp->next );

        // Handle waiting add_work threads.
        if( (!tpool->doNotBlockWhenFull) && (tpool->currentQueueSize == (tpool->maxQueueSize - 1)) )
        {
            if ((rtn = pthread_cond_broadcast(&(tpool->queueNotFull))) != 0)
            {
                fprintf( stderr, "pthread_cond_broadcast %s", strerror(rtn) );
                return( NULL );
            }
        }

        if( (rtn = pthread_mutex_unlock(&(tpool->queueLock))) != 0 )
        {
            fprintf( stderr, "pthread_mutex_unlock %s", strerror(rtn) );
            return( NULL );
        }


        // Do this work item.
        my_workp->routine( my_workp->arg );

        delete my_workp->arg;
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

