// ************************************************************************* //
//                           avtFileFormatInterface.h                        //
// ************************************************************************* //

#ifndef AVT_FILE_FORMAT_INTERFACE_H
#define AVT_FILE_FORMAT_INTERFACE_H

#include <database_exports.h>

#include <vector>

#include <void_ref_ptr.h>
#include <array_ref_ptr.h>

class    vtkDataSet;
class    vtkDataArray;

class    avtDatabaseMetaData;
class    avtFileFormat;
class    avtIOInformation;
class    avtVariableCache;


// ****************************************************************************
//  Class: avtFileFormatInterface
//
//  Purpose:
//      This class goes between a generic database and a file format.  It
//      serves two purposes.  One, it allows the interface of a file format to
//      not be too crowded.  Some function calls are not applicable for certain
//      file formats and this screens those functions out.  This is only
//      worthwhile because it is anticipated that many derived types of file
//      format will be written while only a few file format interfaces will be
//      written.  Two, this enables file formats to be used in ways that the
//      writers of the file format did not intend.  For example, the Silo file
//      format is a single timestep file format, but there are often multiple 
//      Silo files for multiple time steps.  This interface then would worry
//      about making those multiple time steps appear to the database while
//      the Silo files can worry only about one time step at a time.
//      
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 15:49:26 PDT 2001
//    Added GetType.
//
//    Hank Childs, Thu Sep 20 14:07:06 PDT 2001
//    Added SetCache.
//
//    Hank Childs, Fri Oct  5 16:37:03 PDT 2001
//    Added PerformsMaterialSelection and hooks for materials.
//
//    Hank Childs, Thu Oct 25 16:56:18 PDT 2001
//    Added CanDoDynamicLoadBalancing.
//
//    Hank Childs, Wed Jan  9 14:11:28 PST 2002 
//    Added virtual destructor.
//
//    Hank Childs, Tue Mar 12 11:25:42 PST 2002
//    Added GetFilename.
//
//    Kathleen Bonnell, Mon Mar 18 17:15:28 PST 2002 
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Mon Mar 18 17:15:28 PST 2002 
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Fri Mar 14 17:15:00 PST 2003
//    Removed a number of pure virtual functions, since they could be
//    accomplished in the base class.
//
//    Brad Whitlock, Wed May 14 09:22:02 PDT 2003
//    I added an optional int argument to SetDatabaseMetaData.
//
//    Mark C. Miller, Wed Dec 10 10:31:52 PST 2003
//    I added method CanCacheVariable()
//
//    Mark C. Miller, Mon Feb  9 16:00:10 PST 2004
//    Added method, ActivateTimestep
//
// ****************************************************************************

class DATABASE_API avtFileFormatInterface
{
  public:
    virtual                ~avtFileFormatInterface() {;};

    virtual vtkDataSet     *GetMesh(int, int, const char *) = 0;
    virtual vtkDataArray   *GetVar(int, int, const char *) = 0;
    virtual vtkDataArray   *GetVectorVar(int, int, const char *) = 0;
    virtual void           *GetAuxiliaryData(const char *, int, int, 
                                             const char *, void *, 
                                             DestructorFunction &) = 0;

    virtual const char     *GetFilename(int) = 0;
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *,int=0) = 0;

    virtual void            FreeUpResources(int, int) = 0;

    bool                    HasInvariantMetaData(void);
    bool                    HasInvariantSIL(void);

    bool                    CanCacheVariable(const char *);

    bool                    CanDoDynamicLoadBalancing(void);
    const char             *GetType(void);
    bool                    HasVarsDefinedOnSubMeshes(void);
    bool                    PerformsMaterialSelection(void);
    void                    PopulateIOInformation(avtIOInformation &);
    void                    RegisterVariableList(const char *,
                                              const std::vector<CharStrRef> &);
    void                    SetCache(avtVariableCache *);
    void                    TurnMaterialSelectionOff(void);
    void                    TurnMaterialSelectionOn(const char *);
    void                    ActivateTimestep(void);

  protected:
    virtual int             GetNumberOfFileFormats(void) = 0;
    virtual avtFileFormat  *GetFormat(int) const = 0;
};


#endif


