#ifndef VISIT_DYNAMIC_H
#define VISIT_DYNAMIC_H
#include "SimV2Tracing.h"

#ifdef VISIT_STATIC
    /* Statically linked case */
#define VISIT_DYNAMIC_EXECUTE(FUNC, FR, FA, INVOKEARGS) \
    int retval = VISIT_ERROR; \
    extern FR sim2_##FUNC FA; \
    LIBSIM_API_ENTER(VisIt_##FUNC);\
    {\
        retval = simv2_##FUNC INVOKEARGS; \
        if(retval == VISIT_ERROR) \
        { \
            LIBSIM_MESSAGE("simv2_" #FUNC " returned VISIT_ERROR"); \
        } \
        else \
        { \
            LIBSIM_MESSAGE("simv2_" #FUNC " returned VISIT_OKAY"); \
        } \
    }\
    LIBSIM_API_LEAVE(VisIt_##FUNC); \
    return retval;

#else
    /* Dynamic runtime case */
#define VISIT_DYNAMIC_EXECUTE(FUNC, FR, FA, INVOKEARGS) \
    int retval = VISIT_ERROR; \
    LIBSIM_API_ENTER(VisIt_##FUNC);\
    {\
        FR (*cb) FA = (FR (*) FA)visit_get_runtime_function("simv2_"#FUNC);\
        if(cb != NULL)\
        { \
            retval = (*cb) INVOKEARGS;\
            if(retval == VISIT_ERROR) \
            { \
                LIBSIM_MESSAGE("simv2_" #FUNC " returned VISIT_ERROR"); \
            } \
            else \
            { \
                LIBSIM_MESSAGE("simv2_" #FUNC " returned VISIT_OKAY"); \
            } \
        } \
    }\
    LIBSIM_API_LEAVE(VisIt_##FUNC); \
    return retval;

void *visit_get_runtime_function(const char *name);
#endif

#endif
