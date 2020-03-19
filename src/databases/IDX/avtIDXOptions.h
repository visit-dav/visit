// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIDXOptions.h                             //
// ************************************************************************* //

#ifndef AVT_IDX_OPTIONS_H
#define AVT_IDX_OPTIONS_H

class DBOptionsAttributes;

#include <string>

DBOptionsAttributes *GetIDXReadOptions(void);
DBOptionsAttributes *GetIDXWriteOptions(void);


#endif
