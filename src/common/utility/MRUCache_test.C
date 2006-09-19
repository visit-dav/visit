#include <MRUCache.h>
#include <visitstream.h>
#include <string>
#include <stdio.h>

#define CACHE_SIZE 20

using std::string;

// define some types via a typedef
typedef MRUCache<string, char*, MRUCache_Free, CACHE_SIZE> FreeCache;
typedef MRUCache<string, int*, MRUCache_Delete, CACHE_SIZE> DelCache;

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
    FreeCache freeCache;
    MRUCache<string, foo_t*, MRUCache_CallbackDelete, CACHE_SIZE> callbackCache(DeleteAFoo);
    DelCache delCache;
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

    DelCache::iterator it;
    const string fullName = "item 2";
    const int n = fullName.size();
    int j = 0;
    vector<string> keysToRemove;

    cerr << "Iterating to find and remove all keys beginning with \"" << fullName.c_str() << "\"" << endl; 
    for (it = delCache.begin(); it != delCache.end(); it++)
    {
        if (fullName.compare(0, n, it->first, 0, n) == 0)
            keysToRemove.push_back(it->first);
    }
    for (j = 0; j < keysToRemove.size(); j++)
    {
        dontCache.remove(keysToRemove[j]);
        freeCache.remove(keysToRemove[j]);
        callbackCache.remove(keysToRemove[j]);
        delCache.remove(keysToRemove[j]);
        adelCache.remove(keysToRemove[j]);
        cerr << "Removed item with key \"" << keysToRemove[j].c_str() << "\"" << endl;
    }
    cerr << "Iterating to see what remains in cache" << endl;
    for (it = delCache.begin(); it != delCache.end(); it++)
        cerr << "Item with key \"" << it->first.c_str() << "\" still in cache" << endl;
}
