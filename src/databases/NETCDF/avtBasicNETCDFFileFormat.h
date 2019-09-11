// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef BASIC_NETCDF_FILE_FORMAT_H
#define BASIC_NETCDF_FILE_FORMAT_H
#include <avtSTSDFileFormat.h>
#include <avtMTSDFileFormat.h>
#include <vectortypes.h>
#include <map>

class NETCDFFileObject;
class avtFileFormatInterface;
class avtBasicNETCDFReader;

// ****************************************************************************
//  Class: avtBasic_MTSD_NETCDFFileFormat
//
//  Purpose:
//      Reads in NETCDF files with time dimension as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jul 11 11:28:20 PDT 2007
//
//  Modifications:
//
// ****************************************************************************

class avtBasic_MTSD_NETCDFFileFormat : public avtMTSDFileFormat
{
public:
   static bool         Identify(NETCDFFileObject *);
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtBasic_MTSD_NETCDFFileFormat(const char *);
                       avtBasic_MTSD_NETCDFFileFormat(const char *, NETCDFFileObject *);
    virtual           ~avtBasic_MTSD_NETCDFFileFormat();

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "Basic MT NETCDF"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    avtBasicNETCDFReader *reader;
};

// ****************************************************************************
//  Class: avtBasic_STSD_NETCDFFileFormat
//
//  Purpose:
//      Reads in NETCDF files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 10 15:21:14 PST 2005
//
//  Modifications:
//    Brad Whitlock, Wed Apr 26 17:39:32 PST 2006
//    Added a new flag.
//
//    Brad Whitlock, Tue May 16 14:00:05 PST 2006
//    Added override of GetCycleFromFilename.
//
//    Mark C. Miller, Tue Aug 15 15:28:11 PDT 2006
//    Added args to ReturnValidDimensions and procNum/Count data members
//    to support on-the-fly domain decomp
//
//    Brad Whitlock, Thu Oct 29 16:14:17 PDT 2009
//    I moved the guts to avtBasicNETCDFReader.
//
//    Eric Brugger, Fri Nov 13 16:31:57 PST 2009
//    I added GetCycle.
//
// ****************************************************************************

class avtBasic_STSD_NETCDFFileFormat : public avtSTSDFileFormat
{
public: 
   static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f, 
                       const char *const *list, int nList, int nBlock);

                       avtBasic_STSD_NETCDFFileFormat(const char *filename, 
                                                NETCDFFileObject *);
                       avtBasic_STSD_NETCDFFileFormat(const char *filename);
    virtual           ~avtBasic_STSD_NETCDFFileFormat();

    virtual int            GetCycle();
    virtual double         GetTime();

    virtual const char    *GetType(void) { return "Basic ST NETCDF"; }
    virtual void           ActivateTimestep(void); 
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);

protected:
    virtual int            GetCycleFromFilename(const char *f) const;
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
private:
    avtBasicNETCDFReader *reader;
};

#endif
