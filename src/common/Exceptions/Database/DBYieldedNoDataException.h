// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        DBYieldedNoDataException.h                         //
// ************************************************************************* //

#ifndef DB_YIELDED_NO_DATA_EXCEPTION_H
#define DB_YIELDED_NO_DATA_EXCEPTION_H

#include <avtexception_exports.h>

#include <DatabaseException.h>

#include <string>


// ****************************************************************************
//  Class: DBYieldedNoDataException
//
//  Purpose: Exception thrown (typically internally by VisIt's format objects)
//      when database metadata is found to be empty after call to PopulateMd.
//
//  Programmer: Mark C. Miller
//  Creation:   28Oct10
//
// ****************************************************************************

class AVTEXCEPTION_API DBYieldedNoDataException : public DatabaseException
{
  public:
                          DBYieldedNoDataException(const std::string &filename);
                          DBYieldedNoDataException(const std::string &filename,
                                                   const std::string &plugin);
                          DBYieldedNoDataException(const std::string &filename,
                                                   const std::string &plugin,
                                                   const std::string &msg);
    virtual              ~DBYieldedNoDataException() VISIT_THROW_NOTHING {;};
};


#endif
