// ************************************************************************* //
//                        avtMTMDFileFormatInterface.h                       //
// ************************************************************************* //

#ifndef AVT_MTMD_FILE_FORMAT_INTERFACE_H
#define AVT_MTMD_FILE_FORMAT_INTERFACE_H

#include <database_exports.h>

#include <avtFileFormatInterface.h>
#include <avtMTMDFileFormat.h>

class avtIOInformation;

// ****************************************************************************
//  Class: avtMTMDFileFormatInterface
//
//  Purpose:
//      This is an implementation of avtFileFormatInterface for file formats
//      that have Multiple Timesteps and a Multiple Domain.  This is the basic
//      abstraction used by the generic database.  However, this class is
//      needed to enable those classes that don't use the basic abstraction
//      provided by the generic database to also fit in.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Brad Whitlock, Wed May 14 09:24:22 PDT 2003
//    Added an optional timeState argument to SetDatabaseMetaData.
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added ActivateTimestep method
//
// ****************************************************************************

class DATABASE_API avtMTMDFileFormatInterface : public avtFileFormatInterface
{
  public:
                    avtMTMDFileFormatInterface(avtMTMDFileFormat *);
    virtual        ~avtMTMDFileFormatInterface();

    virtual vtkDataSet     *GetMesh(int, int, const char *);
    virtual vtkDataArray   *GetVar(int, int, const char *);
    virtual vtkDataArray   *GetVectorVar(int, int, const char *);
    virtual void           *GetAuxiliaryData(const char *var, int, int,
                                             const char *type, void *args,
                                             DestructorFunction &);

    virtual const char     *GetFilename(int);
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *,int=0);
    virtual void            FreeUpResources(int, int);

    virtual void            ActivateTimestep(int ts);

    virtual void            PopulateIOInformation(int ts, avtIOInformation& ioInfo);

  protected:
    avtMTMDFileFormat      *format;

    virtual int             GetNumberOfFileFormats(void)
                              { return 1; };
    virtual avtFileFormat  *GetFormat(int n) const { return format; };
};


#endif


