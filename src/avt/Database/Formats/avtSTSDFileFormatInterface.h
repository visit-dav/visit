// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtSTSDFileFormatInterface.h                       //
// ************************************************************************* //

#ifndef AVT_STSD_FILE_FORMAT_INTERFACE_H
#define AVT_STSD_FILE_FORMAT_INTERFACE_H

#include <database_exports.h>

#include <avtFileFormatInterface.h>
#include <avtSTSDFileFormat.h>

class avtIOInformation;

// ****************************************************************************
//  Class: avtSTSDFileFormatInterface
//
//  Purpose:
//      This is an implementation of avtFileFormatInterface for file formats
//      that have a Single Timestep and a Single Domain.  It enables such
//      file formats to be treated by the database as if there were multiple
//      timesteps and multiple domains, but to be written as if there was only
//      one timestep and one domain.
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 15:49:26 PDT 2001
//    Added GetType.
//
//    Hank Childs, Mon Oct  8 08:49:52 PDT 2001
//    Added hooks for file formats to do their own material selection.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Brad Whitlock, Wed May 14 09:24:22 PDT 2003
//    Added an optional timeState argument to SetDatabaseMetaData.
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added method, ActivateTimestep
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool arg, forceReadAllCyclesTimes, to SetDatabasemetaData
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added method SetCycleTimeInDatabaseMetaData
//
//    Hank Childs, Tue Dec 20 14:15:33 CST 2011
//    Add method CreateCacheNameIncludingSelections.
//
//    Brad Whitlock, Thu Jun 19 10:50:25 PDT 2014
//    Pass mesh name to PopulateIOInformation.
//
//    Alister Maguire, Tue Sep 24 11:15:10 MST 2019
//    Added GetCycles and GetTimes. 
//
// ****************************************************************************

class DATABASE_API avtSTSDFileFormatInterface : public avtFileFormatInterface
{
  public:
                    avtSTSDFileFormatInterface(avtSTSDFileFormat ***, int,int);
    virtual        ~avtSTSDFileFormatInterface();

    virtual vtkDataSet     *GetMesh(int, int, const char *);
    virtual vtkDataArray   *GetVar(int, int, const char *);
    virtual vtkDataArray   *GetVectorVar(int, int, const char *);
    virtual void           *GetAuxiliaryData(const char *var, int, int,
                                             const char *type, void *args, 
                                             DestructorFunction &);
    virtual std::string     CreateCacheNameIncludingSelections(std::string,
                                                               int, int);

    virtual void            GetCycles(int, intVector &);
    virtual void            GetTimes(int, doubleVector &);

    virtual const char     *GetFilename(int);
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *md,
                                int timeState = 0,
                                bool forceReadAllCyclesTimes = false);
    virtual void            SetCycleTimeInDatabaseMetaData(
                                avtDatabaseMetaData *md, int ts);
    virtual void            FreeUpResources(int, int);

    virtual void            ActivateTimestep(int ts);

    virtual bool            PopulateIOInformation(int ts, const std::string &meshname,
                                                  avtIOInformation& ioInfo);

  protected:
    avtSTSDFileFormat    ***timesteps;
    int                     nTimesteps;
    int                     nBlocks;

    virtual int             GetNumberOfFileFormats(void)
                              { return nTimesteps*nBlocks; };
    virtual avtFileFormat  *GetFormat(int n) const
                              { int block = n % nBlocks;
                                int ts    = n / nBlocks;
                                return timesteps[ts][block]; };
};


#endif


