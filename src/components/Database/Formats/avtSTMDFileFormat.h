// ************************************************************************* //
//                             avtSTMDFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_STMD_FILE_FORMAT_H
#define AVT_STMD_FILE_FORMAT_H

#include <database_exports.h>

#include <avtFileFormat.h>

#include <void_ref_ptr.h>

class     vtkDataSet;
class     vtkDataArray;

class     avtIOInformation;


// ****************************************************************************
//  Class: avtSTMDFileFormat
//
//  Purpose:
//      This is an implementation of avtFileFormat for file formats that have
//      a Single Timestep and Multiple Domains.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Eric Brugger, Fri Dec  7 13:44:05 PST 2001
//    I modified the class to handle the open file cache overflowing by 
//    removing the earliest opened file when the cache overflows.
//
//    Hank Childs, Mon Mar 11 08:48:41 PST 2002
//    Removed SetDatabaseMetaData since it is defined in the base class.
//
//    Kathleen Bonnell, Mon Mar 18 17:22:30 PST 2002 
//    vtkScalars and vtkVectors have  been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Mon Mar 18 17:22:30 PST 2002 
//    vtkScalars and vtkVectors have  been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added method, ActivateTimestep
//
//    Mark C. Miller, Tue Mar 16 14:28:42 PST 2004
//    Added method, PopulateIOInformation
//
// ****************************************************************************

class DATABASE_API avtSTMDFileFormat : public avtFileFormat
{
  public:
                           avtSTMDFileFormat(const char * const *, int);
    virtual               ~avtSTMDFileFormat();

    virtual void          *GetAuxiliaryData(const char *var, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    virtual void           ActivateTimestep(void)
                               { avtFileFormat::ActivateTimestep(); };
    virtual void           PopulateIOInformation(avtIOInformation& ioInfo)
                               { avtFileFormat::PopulateIOInformation(ioInfo); };

    void                   SetTimestep(int ts, int ns) 
                                 { timestep = ts; nTimesteps = ns; };
    virtual int            GetCycle(void);

    virtual const char    *GetFilename(void) { return filenames[0]; };

    virtual vtkDataSet    *GetMesh(int, const char *) = 0;
    virtual vtkDataArray  *GetVar(int, const char *) = 0;
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    char                 **filenames;
    int                    nFiles;
    int                    timestep;
    int                    nTimesteps;
    int                    mostRecentAdded;

    int                    AddFile(const char *);
    virtual void           CloseFile(int) {};
    static const int       MAX_FILES;
};


#endif


