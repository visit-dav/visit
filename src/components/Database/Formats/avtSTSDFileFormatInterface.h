// ************************************************************************* //
//                        avtSTSDFileFormatInterface.h                       //
// ************************************************************************* //

#ifndef AVT_STSD_FILE_FORMAT_INTERFACE_H
#define AVT_STSD_FILE_FORMAT_INTERFACE_H

#include <database_exports.h>

#include <avtFileFormatInterface.h>
#include <avtSTSDFileFormat.h>


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

    virtual const char     *GetFilename(int);
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *,int=0);
    virtual void            FreeUpResources(int, int);

  protected:
    avtSTSDFileFormat    ***timesteps;
    int                     nTimesteps;
    int                     nBlocks;

    virtual int             GetNumberOfFileFormats(void)
                              { return nTimesteps*nBlocks; };
    virtual avtFileFormat  *GetFormat(int n) 
                              { int block = n % nBlocks;
                                int ts    = n / nBlocks;
                                return timesteps[ts][block]; };
};


#endif


