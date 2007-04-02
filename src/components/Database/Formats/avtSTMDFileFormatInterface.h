// ************************************************************************* //
//                        avtSTMDFileFormatInterface.h                       //
// ************************************************************************* //

#ifndef AVT_STMD_FILE_FORMAT_INTERFACE_H
#define AVT_STMD_FILE_FORMAT_INTERFACE_H

#include <database_exports.h>

#include <avtFileFormatInterface.h>
#include <avtSTMDFileFormat.h>

#include <void_ref_ptr.h>

class avtIOInformation;

// ****************************************************************************
//  Class: avtSTMDFileFormatInterface
//
//  Purpose:
//      This is an implementation of avtFileFormatInterface for file formats
//      that have a Single Timestep and Multiple Domains.  It enables such
//      file formats to be treated by the database as if there were multiple
//      timesteps, but to be written as if there was only one timestep.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 15:49:26 PDT 2001
//    Added GetType.
//
//    Hank Childs, Mon Oct  8 10:49:52 PDT 2001
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
//    Hank Childs, Fri Mar 14 18:05:55 PST 2003
//    Moved many routines that were appropriate for the base class to the
//    base class.
//
//    Brad Whitlock, Wed May 14 09:24:22 PDT 2003
//    Added an optional timeState argument to SetDatabaseMetaData.
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added method, ActivateTimestep
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool arg, forceReadAllCyclesAndTimes, to SetDatabaseMetaData
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added method SetCycleTimeInDatabaseMetaData
// ****************************************************************************

class DATABASE_API avtSTMDFileFormatInterface : public avtFileFormatInterface
{
  public:
                    avtSTMDFileFormatInterface(avtSTMDFileFormat **, int);
    virtual        ~avtSTMDFileFormatInterface();

    virtual vtkDataSet     *GetMesh(int, int, const char *);
    virtual vtkDataArray   *GetVar(int, int, const char *);
    virtual vtkDataArray   *GetVectorVar(int, int, const char *);
    virtual void           *GetAuxiliaryData(const char *var, int, int,
                                             const char *type, void *args,
                                             DestructorFunction &);

    virtual const char     *GetFilename(int);
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *md,
                                int timeState = 0,
                                bool forceReadAllCyclesTimes = false);
    virtual void            SetCycleTimeInDatabaseMetaData(
                                avtDatabaseMetaData *md, int ts);
    virtual void            FreeUpResources(int, int);

    virtual void            ActivateTimestep(int ts);

    virtual void            PopulateIOInformation(int ts, avtIOInformation& ioInfo);

  protected:
    avtSTMDFileFormat     **timesteps;
    int                     nTimesteps;

    virtual int             GetNumberOfFileFormats(void)
                              { return nTimesteps; };
    virtual avtFileFormat  *GetFormat(int n) const { return timesteps[n]; };
};


#endif


