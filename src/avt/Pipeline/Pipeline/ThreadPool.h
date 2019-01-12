/***********************************************************************
ThreadPool.h

        Thread pool class.
*/
#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#if defined(VISIT_THREADS)

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <bitset>


// Define these types for the avtExecutionManager.
typedef pthread_mutex_t MUTEX;
typedef pthread_cond_t  COND;
typedef unsigned int MUTEX_ID;


class ThreadPool;

class ThreadPoolWorker
{
    public:
        virtual void ThreadWork( void *arg, int threadID ) = 0;
        virtual void ThreadDoneWithQueue( int threadID ) {}

        void *       GetWorkItem( ThreadPool *tpool );
};


class ThreadPool
{
    public:
        static ThreadPool * Create( int numWorkerThreads, int maxQueueSize, int doNotBlockWhenFull = false );

        int AddWork( void (*workerThreadFunction)(void *), void *arg );
        //int AddWork( ThreadPoolWorker *worker, void *data, unsigned long num, size_t size );

        int Destroy( int waitToFinish = true );

        // Join and work threads will exit.
        int Join();

        // Join and worker thread will not exit.
        int JoinNoExit();

        int GetNumThreads() const
        {
            return( numberThreads );
        }

        int  GetPthreadID();

    private:
        ThreadPool() {}
        ThreadPool( const ThreadPool &obj ) {}
        ~ThreadPool() {}
        ThreadPool& operator=( const ThreadPool& obj ) { return( *this ); }

        // Work structure.
        typedef struct ThreadPoolWorkStruct
        {
            void (*routine)(void *);              // Work function.
            void                        *arg;     // Arguments to work function.
            struct ThreadPoolWorkStruct *next;    // Next work structure.
        } ThreadPoolWork;

        // characteristics.
        int                 numberThreads;
        int                 maxQueueSize;
        int                 doNotBlockWhenFull;

        // state.
        pthread_t           *threads;
        pthread_t           mainThread;
        int                 currentQueueSize;
        ThreadPoolWork      *queueHead;
        ThreadPoolWork      *queueTail;
        int                 queueClosed;
        int                 shutdown;
        // A bit array of threads that are currently working.
        // Used with the queueEmpty condition to know if all work is done.
// TODO: make a bitset class that can handle any number of threads
        std::bitset<64>     threadWorking;
        std::bitset<64>     threadWorking2;
        std::bitset<64>     threadWorking3;
        std::bitset<64>     threadWorking4;

        // synchronization.
        pthread_mutex_t     queueLock;
        pthread_cond_t      queueNotEmpty;
        pthread_cond_t      queueNotFull;
        pthread_cond_t      queueEmpty;

        friend void *ThreadPoolThread( void * );
        friend class ThreadPoolWorker;
};

inline void PthreadMutexLock( pthread_mutex_t &lock )
{
    int rtn;
    if( (rtn = pthread_mutex_lock(&lock)) )
    {
        fprintf( stderr, "pthread_mutex_lock %s", strerror(rtn) );
    }
}

inline void PthreadMutexUnlock( pthread_mutex_t &lock )
{
    int rtn;
    if( (rtn = pthread_mutex_unlock(&lock)) )
    {
        fprintf( stderr, "pthread_mutex_unlock %s", strerror(rtn) );
    }
}

inline void PthreadMutexInit( pthread_mutex_t &lock )
{
    int rtn;
    if( (rtn = pthread_mutex_init(&lock, NULL)) )
    {
        fprintf( stderr, "pthread_mutex_init %s", strerror(rtn) );
    }
}

inline void PthreadMutexDestroy( pthread_mutex_t &lock )
{
    int rtn;
    if( (rtn = pthread_mutex_destroy(&lock)) )
    {
        fprintf( stderr, "pthread_mutex_destroy %s", strerror(rtn) );
    }
}

inline void PthreadCondInit( pthread_cond_t &cond )
{
    int rtn;
    if( (rtn = pthread_cond_init(&cond, NULL)) != 0 )
    {
        fprintf( stderr, "pthread_cond_init %s", strerror(rtn) );
    }
}

inline void PthreadCondDestroy( pthread_cond_t &cond )
{
    int rtn;
    if( (rtn = pthread_cond_destroy(&cond)) != 0 )
    {
        fprintf( stderr, "pthread_cond_destroy %s", strerror(rtn) );
    }
}

inline void PthreadCondSignal( pthread_cond_t &cond )
{
    int rtn;
    if( (rtn = pthread_cond_signal(&cond)) != 0 )
    {
        fprintf( stderr, "pthread_cond_signal %s", strerror(rtn) );
    }
}

inline void PthreadCondWait( pthread_cond_t &cond, pthread_mutex_t &lock )
{
    int rtn;
    if( (rtn = pthread_cond_wait(&cond, &lock)) != 0 )
    {
        fprintf( stderr, "pthread_cond_wait %s", strerror(rtn) );
    }
}

#endif // VISIT_THREADS
#endif // _THREAD_POOL_H_

