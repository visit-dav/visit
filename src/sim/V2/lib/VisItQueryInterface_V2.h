// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_QUERY_INTERFACE_V1_H
#define VISIT_QUERY_INTERFACE_V1_H

/*****************************************************************************
 *  File:  VisItQueryInterface_V1.h
 *
 *  Purpose:
 *    Query functions available through libsim.
 *
 *  Programmer:  Brad Whitlock
 *  Creation:    Thu Jul 31 09:24:11 PDT 2008
 *
 *  Modifications:
 *
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define VISIT_QUERY_SUCCESS 1
#define VISIT_QUERY_FAILURE 0

int VisItQuery_Volume(double *value);

#ifdef __cplusplus
}
#endif

#endif
