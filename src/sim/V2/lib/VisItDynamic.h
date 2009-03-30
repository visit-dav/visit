#ifndef VISIT_DYNAMIC_H
#define VISIT_DYNAMIC_H
#include "SimV2Tracing.h"

#define VISIT_DYNAMIC_EXECUTE(FUNC, FUNCTYPE, FUNCVAR, EXEC_CB) \
    int retval = VISIT_ERROR; \
    const char *fname = "VisIt_"#FUNC;\
    const char *fname2 = "visit_"#FUNC;\
    LIBSIM_API_ENTER(VisIt_##FUNC);\
    {\
        FUNCVAR = (FUNCTYPE)visit_get_runtime_function(fname2);\
        if(cb != NULL)\
        { \
            retval = EXEC_CB;\
        } \
    }\
    LIBSIM_API_LEAVE(VisIt_##FUNC); \
    return retval;

void *visit_get_runtime_function(const char *name);

/* F77_ID - attempt a uniform naming of FORTRAN 77 functions which
 *        - gets around loader naming conventions
 *        -
 *        - F77_ID(foo_, foo, FOO)(x, y, z)
 */
#ifndef F77_ID
#ifdef _INT_MAX_46              /* UNICOS */
#define F77_ID(x_, x, X)  X
#endif
#ifdef _H_LIMITS                /* AIX */
#define F77_ID(x_, x, X)  x
#endif
#ifdef _INCLUDE_HPUX_SOURCE     /* HPUX */
#define F77_ID(x_, x, X)  x
#endif
#ifndef F77_ID
#define F77_ID(x_, x, X)  x_
#endif
#endif

#endif
