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
// ****************************************************************************

class DATABASE_API avtMTMDFileFormat : public avtFileFormat
{
  public:
                           avtMTMDFileFormat(const char *);
    virtual               ~avtMTMDFileFormat();

    virtual void          *GetAuxiliaryData(const char *var, int, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);
    virtual int            GetNTimesteps(void);

    virtual const char    *GetFilename(void) { return filename; };

    virtual vtkDataSet    *GetMesh(int, int, const char *) = 0;
    virtual vtkDataArray  *GetVar(int, int, const char *) = 0;
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

    virtual void           ActivateTimestep(int ts)
                               { avtFileFormat::ActivateTimestep(); };
    virtual void           PopulateIOInformation(int ts, avtIOInformation& ioInfo)
                               { avtFileFormat::PopulateIOInformation(ioInfo); };

  protected:
    char                  *filename;
};


#endif


