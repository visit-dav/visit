// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <NoEngineException.h>

NoEngineException::NoEngineException() : VisItException()
{
}

NoEngineException::~NoEngineException() VISIT_THROW_NOTHING
{
}
