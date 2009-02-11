#include "VisItQueryInterface_V1.h"
#include "SimV2Tracing.h"

/* VisIt Engine Library query function pointers */
typedef struct
{
    int (*v_query_volume)(void *e, double *value, char **err);
} query_callback_t;

static query_callback_t *simv2_queries = NULL;

/*******************************************************************************
*
* Name: LoadVisItQueries
*
* Purpose: Loads the query functions from the VisIt Engine and makes them
*          callable.
*
* Note:    This function should only be called from LoadVisItLibrary.
*
* Author: Brad Whitlock, B Division, Lawrence Livermore National Laboratory
*
* Modifications:
*
*******************************************************************************/
void LoadVisItQueries(void *dl_handle)
{
    if(simv2_queries == NULL)
    {
        simv2_queries = (query_callback_t *)malloc(sizeof(query_callback_t));
        memset(simv2_queries, 0, sizeof(query_callback_t));
    }

#define QUERY_DLSYM(f,t,n)                \
   f = (t)dlsym_function(dl_handle, n); \
   if (!f) \
   { \
       LIBSIM_MESSAGE1("LoadVisItQueries: Could not find function %s", n); \
   }

   QUERY_DLSYM(v_query_volume, int (*)(void*,double*,char**), "query_volume");
}

/*******************************************************************************
 *******************************************************************************
 *******************************************************************************
                             Public API Query Functions
 *******************************************************************************
 *******************************************************************************
 ******************************************************************************/

int VisItQuery_Volume(double *value)
{
    int success = VISIT_QUERY_SUCCESS;
    LIBSIM_API_ENTER(VisItQuery_Volume);

    *value = 0.;
    if(engine != NULL && simv2_queries->query_volume != NULL)
    {
        char *err = NULL;
        success = (*simv2_queries->query_volume)(engine, value, &err);

        if(err != NULL)
        {
            LIBSIM_MESSAGE(err);
            free(err);
        }
    }
    else
    {
        LIBSIM_MESSAGE("The Volume query is not available.");
        success = VISIT_QUERY_FAILURE;
    }
    LIBSIM_API_LEAVE2(VisItQuery_Volume, "volume=%lg, return %g", *value, success);
    return success;   
}
