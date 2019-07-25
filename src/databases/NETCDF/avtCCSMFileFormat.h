// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCCSMFileFormat.h                            //
// ************************************************************************* //

#ifndef AVT_CCSM_FILE_FORMAT_H
#define AVT_CCSM_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>
#include <avtSTSDFileFormat.h>

#include <vector>

class avtFileFormatInterface;
class NETCDFFileObject;
class avtCCSMReader;

// ****************************************************************************
//  Class: avtCCSM_MTSD_FileFormat
//
//  Purpose:
//      Reads in CCSM files with time dimension as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
//  Modifications:
//    Brad Whitlock, Tue Oct 27 12:14:35 PDT 2009
//    I moved the guts to avtCCSMReader.
//
// ****************************************************************************

class avtCCSM_MTSD_FileFormat : public avtMTSDFileFormat
{
public:
   static bool         Identify(NETCDFFileObject *);
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtCCSM_MTSD_FileFormat(const char *);
                       avtCCSM_MTSD_FileFormat(const char *, NETCDFFileObject *);
    virtual           ~avtCCSM_MTSD_FileFormat();

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "CCSM MT"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    avtCCSMReader *reader;
};

// ****************************************************************************
//  Class: avtCCSM_MTSD_FileFormat
//
//  Purpose:
//      Reads in CCSM files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Oct 27 12:15:09 PDT 2009
//
//  Modifications:
//
// ****************************************************************************

class avtCCSM_STSD_FileFormat : public avtSTSDFileFormat
{
public: 
   static bool         Identify(NETCDFFileObject *);
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtCCSM_STSD_FileFormat(const char *);
                       avtCCSM_STSD_FileFormat(const char *, NETCDFFileObject *);
    virtual           ~avtCCSM_STSD_FileFormat();

    virtual int           GetCycle();
    virtual double        GetTime();
    virtual const char   *GetType(void) { return "CCSM ST"; }

    virtual void          ActivateTimestep(void); 
    virtual void          FreeUpResources(void); 

    virtual vtkDataSet   *GetMesh(const char *);
    virtual vtkDataArray *GetVar(const char *);

protected:
    int                   GetCycleFromFilename(const char *f) const;
    void                  PopulateDatabaseMetaData(avtDatabaseMetaData *);

    avtCCSMReader *reader;
};

#endif
