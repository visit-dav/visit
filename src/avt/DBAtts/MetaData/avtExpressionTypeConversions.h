// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
#ifndef AVT_EXPRESSION_TYPE_CONVERSION_H
#define AVT_EXPRESSION_TYPE_CONVERSION_H
#include <avtTypes.h>
#include <Expression.h>
#include <dbatts_exports.h>

DBATTS_API avtVarType           ExprType_To_avtVarType(Expression::ExprType type);
DBATTS_API Expression::ExprType avtVarType_To_ExprType(avtVarType type);

#endif
