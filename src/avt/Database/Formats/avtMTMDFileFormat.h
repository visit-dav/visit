// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtMTMDFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MTMD_FILE_FORMAT_H
#define AVT_MTMD_FILE_FORMAT_H

#include <database_exports.h>

#include <avtFileFormat.h>

#include <vector>

#include <void_ref_ptr.h>


class     vtkDataSet;
class     vtkDataArray;

class     avtIOInformation;


// ****************************************************************************
//  Class: avtMTMDFileFormat
//
//  Purpose:
//      This is an implementation of avtFileFormat for file formats that have
//      a Multiple Timesteps and Multiple Domains.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Brad Whitlock, Mon Oct 13 14:17:30 PST 2003
//    Added a method to get the times.
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added method, ActivateTimestep
//
//    Mark C. Miller, Tue Mar 16 14:28:42 PST 2004
//    Added method, PopulateIOInformation
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Moved GetCycles/Times to base class, avtFileFormat
//    Added SetDatabaseMetaData (moved it down from base class)
//    Added time-qualified and non-time-qualified PopulateDatabaseMetaData
//    methods, the later for backward compatibility
//
//    Hank Childs, Sun May  9 18:47:06 CDT 2010
//    Add support for time slice offsets (used when group MT files with .visit
//    files).
//
//    Mark C. Miller, Fri Oct 29 09:58:43 PDT 2010
//    Moved implementation of SetDatabaseMetaData to the .C file.
//
//    Hank Childs, Tue Apr 10 15:12:58 PDT 2012
//    Add method SetReadAllCyclesAndTimes.
//
//    Brad Whitlock, Thu Jun 19 10:50:25 PDT 2014
//    Pass mesh name to PopulateIOInformation.
//
// ****************************************************************************

class DATABASE_API avtMTMDFileFormat : public avtFileFormat
{
  public:
                           avtMTMDFileFormat(const char *);
    virtual               ~avtMTMDFileFormat();

    virtual void          *GetAuxiliaryData(const char *var, int, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetFilename(void) { return filename; };

    virtual vtkDataSet    *GetMesh(int, int, const char *) = 0;
    virtual vtkDataArray  *GetVar(int, int, const char *) = 0;
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

    virtual void           ActivateTimestep(int ts)
                               { avtFileFormat::ActivateTimestep(); };
    using avtFileFormat::ActivateTimestep;
    virtual bool           PopulateIOInformation(int ts, const std::string &meshname,
                                                 avtIOInformation& ioInfo) { return false; }

    virtual void           SetDatabaseMetaData(avtDatabaseMetaData *md, int ts = 0);

    void                   SetTimeSliceOffset(int ts) { timeSliceOffset = ts; };

    void                   SetReadAllCyclesAndTimes(bool b) 
                                             { readAllCyclesAndTimes = b; };
    bool                   GetReadAllCyclesAndTimes(void)
                                             { return readAllCyclesAndTimes; };

  protected:
    char                  *filename;
    int                    timeSliceOffset;
    bool                   readAllCyclesAndTimes;

    // The second of these should really be pure virtual and the first
    // non-existant. However, both are just virtual to maintain 
    // backward compatibility with older MTXX plugins and to allow 
    // MTXX plugins to implement a time-qualified request to populate
    // database metadata.
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *md);
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *md, int);

};


#endif


