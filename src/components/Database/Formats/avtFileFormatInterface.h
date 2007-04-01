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
//      MCM, Added 23Feb04
//
//      Two classes go hand-in-hand here; the FormatInterface classes and
//      the Format classes. Every FormatInterface HAS A (sometimes more than
//      one) Format object. avtFileFormatInterface is the base class for
//      for FormatInterface objects while avtFileFormat is the base class
//      for Format objects.
//
//      In VisIt, the abstract FormatInterface supports the notion that a mesh
//      is composed of various pieces (called domains) at various timesteps.
//      The general request for data from VisIt through the FormatInterface to a
//      plugin looks something like GetData(int domainNumber, int timeStep).
//      Of course, not all real file formats that Visit reads from actually
//      support either the notions of pieces of mesh or of time. Consequently,
//      there are four basic FormatInterface classes defined by VisIt derived
//      from avtFileFormatInterface. These four derived classes are used to
//      distinguish how the FormatInterface operates on one of many domains
//      and/or timesteps. The four FormatInterfaces are...
//
//         1) single-timestep, single-domain (STSD)
//         2) single-timestep, multiple-domain (STMD)
//         3) multiple-timestep, single-domain (MTSD)
//         4) multiple-timestep, multiple-domain (MTMD)
//
//     For example, an STSD FormatInterface supports the notion of only a single
//     timestep and a single domain. It will only ever recieve requests from VisIt
//     of the form GetData(). Note that the timeStep and domainNumber arguments
//     are not present because the FormatInterface cannot interpret them.
//     Note also that this DOES NOT IMPLY that one cannot handle time-varying
//     and/or multiple domain data through an STSD FormatInterface. Every
//     FormatInterface object HAS A Format object, one or more. An STSD 
//     FormatInterface in fact has many Format objects, one for each timestep
//     and each domain. Each of these Format objects handles one timestep
//     and one domain. The STSDFormatInterface is responsible for selecting
//     which among the many domains and which among the many timesteps for
//     a given operation. On the other hand, an MTMD FormatInterface has
//     a single Format object because all classes derived from an MTMD
//     type of FormatInterface support, natively, there own notions of
//     domains and timesteps.
//
//     Here are some rules of thumb to use in deciding where to implement
//     new functionality.
//
//     1) Don't define new funtions in any of the FormatInterface objects as
//        pure virtual, UNLESS ABSOLUTELY NECESSARY. The effect of introducing
//        pure virtual functions to the FormatInterface is that ALL derived
//        classes (e.g. the database plugins) ARE REQUIRED to implement them.
//        All the plugins would have to be updated.
//
//     2) Don't define new functions in any of the FormatInterface objects
//        as non-virtual because it means the derived classes (e.g. the
//        database plugins) won't be able to override the behavior if needed.
//        If there is functionality needed in the FormatInterface that
//        shall never be allowed to be overridden in the plugins, then it
//        should probably by implemented in the base classes for the Format
//        and FormatInterface.
//
//     3) If the interface to the new functionality is not FormatInterface
//        specific (e.g. it has no formal arguments for time or domain),
//        you can probably get by with implementing it in the base classes,
//        avtFileFormat and avtFileFormatInterface. Of course, it should still
//        be virtual so plugins can override it. Otherwise, you should
//        implement it in BOTH the bases classes and the FormatInterface
//        specific classes.
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
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Mark C. Miller, Mon Feb  9 16:00:10 PST 2004
//    Added method, ActivateTimestep
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added ts argument to ActivateTimestep and made it pure virtual 
//
//    Mark C. Miller, Tue Mar 16 14:09:43 PST 2004
//    Added time step argument to PopulateIOInformation. Made it pure virtual
//
// ****************************************************************************

class DATABASE_API avtFileFormatInterface
{
  public:
    virtual                ~avtFileFormatInterface();

    virtual vtkDataSet     *GetMesh(int, int, const char *) = 0;
    virtual vtkDataArray   *GetVar(int, int, const char *) = 0;
    virtual vtkDataArray   *GetVectorVar(int, int, const char *) = 0;
    virtual void           *GetAuxiliaryData(const char *, int, int, 
                                             const char *, void *, 
                                             DestructorFunction &) = 0;

    virtual const char     *GetFilename(int) = 0;
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *,int=0) = 0;

    virtual void            FreeUpResources(int, int) = 0;
    virtual void            ActivateTimestep(int ts) = 0;
    virtual void            PopulateIOInformation(int ts, avtIOInformation &) = 0;

    bool                    HasInvariantMetaData(void);
    bool                    HasInvariantSIL(void);

    bool                    CanCacheVariable(const char *);

    bool                    CanDoDynamicLoadBalancing(void);
    const char             *GetType(void);
    bool                    HasVarsDefinedOnSubMeshes(void);
    bool                    PerformsMaterialSelection(void);
    void                    RegisterVariableList(const char *,
                                              const std::vector<CharStrRef> &);
    void                    SetCache(avtVariableCache *);
    void                    TurnMaterialSelectionOff(void);
    void                    TurnMaterialSelectionOn(const char *);

  protected:
    virtual int             GetNumberOfFileFormats(void) = 0;
    virtual avtFileFormat  *GetFormat(int) const = 0;
};


#endif


