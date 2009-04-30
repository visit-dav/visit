/* gprof-helper.c -- preload library to profile pthread-enabled programs
 *
 * Authors: Sam Hocevar <sam at zoy dot org>
 *          Daniel J~nsson <danieljo at fagotten dot org>
 * 
 * Compilation example:
 * gcc -shared -nostdlib -fPIC gprof-helper.c -o gprof-helper.so -lpthread -ldl
 *
 * Usage example:
 * LD_PRELOAD=./gprof-helper.so your_program
 */
/*!
  Note from Rich Cook: 
  http://sam.zoy.org/writings/programming/gprof.html

What is gprof?

gprof is the GNU Profiler, a tool used when tracking which functions are eating CPU in your program. Anyway, you should already be familiar with it if you got interested in this page.

One problem with gprof under certain kernels (such as Linux) is that it doesn~t behave correctly with multithreaded applications. It actually only profiles the main thread, which is quite useless.

Workaround

There is an easy, but surprisingly not very widespread fix for this annoying gprof behaviour. Basically, gprof uses the internal ITIMER_PROF timer which makes the kernel deliver a signal to the application whenever it expires. So we just need to pass this timer data to all spawned threads.

Example

It wouldn~t be too hard to put a call to setitimer in each function spawned by a thread, but I thought it would be more elegant to implement a wrapper for pthread_create.

Daniel Jönsson enhanced my code so that it could be used in a preload library without having to modify the program. It can also be very useful for libraries that spawn threads without warning, such as libSDL. The result code is shown below and can be downloaded (gprof-helper.c):
*/

#define _GNU_SOURCE
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>

static void * wrapper_routine(void *);

/* Original pthread function */
static int (*pthread_create_orig)(pthread_t *__restrict,
                                  __const pthread_attr_t *__restrict,
                                  void *(*)(void *),
                                  void *__restrict) = NULL;

/* Library initialization function */
void _init(void)
{
    pthread_create_orig = dlsym(RTLD_NEXT, "pthread_create");
    fprintf(stderr, "pthreads: using profiling hooks for gprof\n");
    if(pthread_create_orig == NULL)
    {
        char *error = dlerror();
        if(error == NULL)
        {
            error = "pthread_create is NULL";
        }
        fprintf(stderr, "%s", error);
        exit(EXIT_FAILURE);
    }
}

/* Our data structure passed to the wrapper */
typedef struct wrapper_s
{
    void * (*start_routine)(void *);
    void * arg;

    pthread_mutex_t lock;
    pthread_cond_t  wait;

    struct itimerval itimer;

} wrapper_t;

/* The wrapper function in charge for setting the itimer value */
static void * wrapper_routine(void * data)
{
    /* Put user data in thread-local variables */
    void * (*start_routine)(void *) = ((wrapper_t*)data)->start_routine;
    void * arg = ((wrapper_t*)data)->arg;

    /* Set the profile timer value */
    setitimer(ITIMER_PROF, &((wrapper_t*)data)->itimer, NULL);

    /* Tell the calling thread that we don't need its data anymore */
    pthread_mutex_lock(&((wrapper_t*)data)->lock);
    pthread_cond_signal(&((wrapper_t*)data)->wait);
    pthread_mutex_unlock(&((wrapper_t*)data)->lock);

    /* Call the real function */
    return start_routine(arg);
}

/* Our wrapper function for the real pthread_create() */
int pthread_create(pthread_t *__restrict thread,
                   __const pthread_attr_t *__restrict attr,
                   void * (*start_routine)(void *),
                   void *__restrict arg)
{
    wrapper_t wrapper_data;
    int i_return;

    /* Initialize the wrapper structure */
    wrapper_data.start_routine = start_routine;
    wrapper_data.arg = arg;
    getitimer(ITIMER_PROF, &wrapper_data.itimer);
    pthread_cond_init(&wrapper_data.wait, NULL);
    pthread_mutex_init(&wrapper_data.lock, NULL);
    pthread_mutex_lock(&wrapper_data.lock);

    /* The real pthread_create call */
    i_return = pthread_create_orig(thread,
                                   attr,
                                   &wrapper_routine,
                                   &wrapper_data);

    /* If the thread was successfully spawned, wait for the data
     * to be released */
    if(i_return == 0)
    {
        pthread_cond_wait(&wrapper_data.wait, &wrapper_data.lock);
    }

    pthread_mutex_unlock(&wrapper_data.lock);
    pthread_mutex_destroy(&wrapper_data.lock);
    pthread_cond_destroy(&wrapper_data.wait);

    return i_return;
}
