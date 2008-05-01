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

#include <MRUCache.h>
#include <visitstream.h>
#include <string>
#include <stdio.h>

#include <cstring>
#include <cstdlib>

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
    vector<string> keysToRemove;

    cerr << "Iterating to find and remove all keys beginning with \"" << fullName.c_str() << "\"" << endl; 
    for (it = delCache.begin(); it != delCache.end(); it++)
    {
        if (fullName.compare(0, n, it->first, 0, n) == 0)
            keysToRemove.push_back(it->first);
    }
    for (size_t j = 0; j < keysToRemove.size(); j++)
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
