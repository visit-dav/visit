#include <MRUCache.h>
#include <visitstream.h>
#include <string>
#include <stdio.h>

using std::string;

#define CACHE_SIZE 20

typedef struct {
   int a;
   float b;
   char c;
   double d;
} foo_t;

void DeleteAFoo(void *p)
{
    foo_t *fp = (foo_t*) p;
    free(fp);
}

int main()
{
    int i;

    // make caches for each of the kinds of delete
    MRUCache<string, int, MRUCache_DontDelete, CACHE_SIZE> dontCache;
    MRUCache<string, char*, MRUCache_Free, CACHE_SIZE> freeCache;
    MRUCache<string, foo_t*, MRUCache_CallbackDelete, CACHE_SIZE> callbackCache(DeleteAFoo);
    MRUCache<string, int*, MRUCache_Delete, CACHE_SIZE> delCache;
    MRUCache<string, float*, MRUCache_ArrayDelete, CACHE_SIZE> adelCache;


    // load caches and see how things behave
    for (i = 0; i < 2*CACHE_SIZE; i++)
    {
        cerr << "Iteration " << i << endl;
        char tmp[256];
        sprintf(tmp, "item %02d", i);
        string tmpStr = string(tmp);

        // create entries in each kind of cache
        dontCache[tmpStr] = i;

        freeCache[tmpStr] = strdup(tmp);

        foo_t *fp = (foo_t*) malloc(sizeof(foo_t));
        fp->a = i;
        fp->b = (float) i / 2*CACHE_SIZE;
        fp->c = '\0';
        fp->d = (double) i / 2*CACHE_SIZE;
        callbackCache[tmpStr] = fp;

        int *ip = new int;
        *ip = i;
        delCache[tmpStr] = ip;

        float *fltp = new float[10];
        for (int j = 0; j < 10; j++)
            fltp[j] = (float) j;
        adelCache[tmpStr] = fltp;
    }

}
