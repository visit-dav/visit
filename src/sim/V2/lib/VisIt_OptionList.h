// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_OPTIONLIST_H
#define VISIT_OPTIONLIST_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_OptionList_alloc(visit_handle *obj);
int VisIt_OptionList_free(visit_handle obj);
int VisIt_OptionList_setValueB(visit_handle h, const char *, int);
int VisIt_OptionList_setValueI(visit_handle h, const char *, int);
int VisIt_OptionList_setValueF(visit_handle h, const char *, float);
int VisIt_OptionList_setValueD(visit_handle h, const char *, double);
int VisIt_OptionList_setValueS(visit_handle h, const char *, const char *);
int VisIt_OptionList_setValueE(visit_handle h, const char *, int);

int VisIt_OptionList_getNumValues(visit_handle h, int *);
int VisIt_OptionList_getName(visit_handle h, int, char **);
int VisIt_OptionList_getType(visit_handle h, int, int *);
int VisIt_OptionList_getIndex(visit_handle h, const char *, int *);
int VisIt_OptionList_getValueI(visit_handle h, const char *, int *);
int VisIt_OptionList_getValueF(visit_handle h, const char *, float *);
int VisIt_OptionList_getValueD(visit_handle h, const char *, double *);
int VisIt_OptionList_getValueS(visit_handle h, const char *, char **);
int VisIt_OptionList_getValueE(visit_handle h, const char *, int *);

#ifdef __cplusplus
}
#endif

#endif
