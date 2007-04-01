// ************************************************************************* //
//                        avtMTSDFileFormatInterface.h                       //
// ************************************************************************* //

#ifndef AVT_MTSD_FILE_FORMAT_INTERFACE_H
#define AVT_MTSD_FILE_FORMAT_INTERFACE_H

#include <database_exports.h>

#include <avtFileFormatInterface.h>
#include <avtMTSDFileFormat.h>


// ****************************************************************************
//  Class: avtMTSDFileFormatInterface
//
//  Purpose:
//      This is an implementation of avtFileFormatInterface for file formats
//      that have Multiple Timesteps and a Single Domain.  It enables such
//      file formats to be treated by the database as if there were multiple
//      domains, but to be written as if there was only one domain.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors havebeen deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Fri Mar 14 17:28:55 PST 2003
//    Removed calls that could be put in the base class.
//
//    Brad Whitlock, Wed May 14 09:24:22 PDT 2003
//    Added an optional timeState argument to SetDatabaseMetaData.
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added method, ActivateTimestep
//
// ****************************************************************************

class DATABASE_API avtMTSDFileFormatInterface : public avtFileFormatInterface
{
  public:
                    avtMTSDFileFormatInterface(avtMTSDFileFormat **, int);
    virtual        ~avtMTSDFileFormatInterface();

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

  protected:
    avtMTSDFileFormat     **domains;
    int                     nDomains;

    virtual int             GetNumberOfFileFormats(void)
                              { return nDomains; };
    virtual avtFileFormat  *GetFormat(int n) const { return domains[n]; };
};


#endif


