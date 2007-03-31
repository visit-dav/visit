// ************************************************************************* //
//                             avtSTSDFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_STSD_FILE_FORMAT_H
#define AVT_STSD_FILE_FORMAT_H

#include <database_exports.h>

#include <avtFileFormat.h>

#include <void_ref_ptr.h>

class     vtkDataSet;
class     vtkDataArray;


// ****************************************************************************
//  Class: avtSTSDFileFormat
//
//  Purpose:
//      This is an implementation of avtFileFormat for file formats that have
//      a Single Timestep and a Single Domain.
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Mar 11 08:48:41 PST 2002
//    Removed SetDatabaseMetaData since it is defined in the base class.
//
//    Kathleen Bonnell, Mon Mar 18 17:22:30 PST 2002  
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Mon Mar 18 17:22:30 PST 2002  
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Brad Whitlock, Mon Oct 13 14:42:42 PST 2003
//    Added a method to return the time. The default implementation just
//    returns zero but it allows derived types to perform other actions
//    to return the right time. I also added method to indicate whether the
//    returned cycle/time can be believed.
//
// ****************************************************************************

class DATABASE_API avtSTSDFileFormat : public avtFileFormat
{
  public:
                           avtSTSDFileFormat(const char *);
    virtual               ~avtSTSDFileFormat();

    void                   SetTimestep(int ts) { timestep = ts; };
    void                   SetDomain(int dom) { domain = dom; };
    virtual void          *GetAuxiliaryData(const char *var, const char *type,
                                            void *args, DestructorFunction &);

    virtual bool           ReturnsValidCycle() const { return false; }
    virtual int            GetCycle(void);
    virtual bool           ReturnsValidTime() const { return false; }
    virtual double         GetTime(void);
    virtual const char    *GetFilename(void) { return filename; };

    virtual vtkDataSet    *GetMesh(const char *) = 0;
    virtual vtkDataArray  *GetVar(const char *) = 0;
    virtual vtkDataArray  *GetVectorVar(const char *);

  protected:
    char                  *filename;
    int                    domain;
    int                    timestep;
};


#endif


