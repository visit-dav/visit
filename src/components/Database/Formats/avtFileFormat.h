// ************************************************************************* //
//                               avtFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_FILE_FORMAT_H
#define AVT_FILE_FORMAT_H
#include <database_exports.h>

// For NULL
#include <stdlib.h>

#include <string>
#include <vector>

#include <array_ref_ptr.h>

#include <avtTypes.h>


class     avtDatabaseMetaData;
class     avtIOInformation;
class     avtVariableCache;


// ****************************************************************************
//  Class:  avtFileFormat
//
//  Purpose:
//      This defines an interfaces that all file formats must conform to.
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep 20 14:10:18 PDT 2001
//    Added SetCache.
//
//    Hank Childs, Mon Oct  8 08:50:45 PDT 2001
//    Added hooks for material selection.
//
//    Hank Childs, Thu Mar 21 13:43:14 PST 2002
//    Added mechanisms for closing files for file descriptor management.
//
// ****************************************************************************

class DATABASE_API avtFileFormat
{
    friend void           FileFormatCloseFileCallback(void *, int);

  public:
                          avtFileFormat();
    virtual              ~avtFileFormat();

    virtual void          FreeUpResources(void);
    void                  SetDatabaseMetaData(avtDatabaseMetaData *);
    void                  RegisterDatabaseMetaData(avtDatabaseMetaData *);
    virtual void          PopulateIOInformation(avtIOInformation &);
    void                  SetCache(avtVariableCache *);

    virtual const char   *GetType(void) = 0;
    virtual const char   *GetFilename(void) = 0;

    virtual bool          PerformsMaterialSelection(void) { return false; };
    virtual bool          HasVarsDefinedOnSubMeshes(void) { return false; };
    virtual void          TurnMaterialSelectionOff(void);
    virtual void          TurnMaterialSelectionOn(const char *);

    bool                  CanDoDynamicLoadBalancing(void)
                              { return canDoDynamicLoadBalancing; };

    virtual void          RegisterVariableList(const char *,
                                          const std::vector<CharStrRef> &) {;};

  protected:
    avtVariableCache     *cache;
    avtDatabaseMetaData  *metadata;
    bool                  doMaterialSelection;
    bool                  canDoDynamicLoadBalancing;
    bool                  closingFile;
    char                 *materialName;
    std::vector<int>      fileIndicesForDescriptorManager;

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *) = 0;

    void       AddMeshToMetaData(avtDatabaseMetaData *, std::string,
                                 avtMeshType, const float * = NULL, int = 1,
                                 int = 0, int = 3, int = 3);
    void       AddScalarVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering,
                                      const float * = NULL);
    void       AddVectorVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering, int = 3,
                                      const float * = NULL);
    void       AddMaterialToMetaData(avtDatabaseMetaData *, std::string,
                                     std::string, int,std::vector<std::string>);
    void       AddSpeciesToMetaData(avtDatabaseMetaData *, std::string,
                                    std::string, std::string, int,
                                    std::vector<int>,
                                    std::vector<std::vector<std::string> >);

    int           GuessCycle(const char *);

    void          RegisterFile(int);
    void          UnregisterFile(int);
    void          UsedFile(int);
    virtual void  CloseFile(int);
    void          CloseFileDescriptor(int);
};


#endif


