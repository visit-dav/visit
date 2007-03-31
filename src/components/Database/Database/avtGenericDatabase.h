// ************************************************************************* //
//                            avtGenericDatabase.h                           //
// ************************************************************************* //

#ifndef AVT_GENERIC_DATABASE_H
#define AVT_GENERIC_DATABASE_H

#include <database_exports.h>

#include <string>
#include <vector>

#include <avtDatasetDatabase.h>
#include <avtMaterial.h>
#include <avtSpecies.h>
#include <avtVariableCache.h>

#include <MIR.h>

struct    avtDatasetCollection;
struct    avtMatSpeciesMetaData;
class     avtFileFormatInterface;
class     avtMixedVariable;
class     avtSILRestrictionTraverser;
class     PickAttributes;
class     PickVarInfo;


// ****************************************************************************
//  Class: avtGenericDatabase
//
//  Purpose:
//      A database that handles a time sequence of data.  It does not care
//      which file format the data comes from because that is handled for it
//      by a file format interface.  The databases specific responsibilities
//      include unwinding SIL restrictions, performing those restrictions,
//      creating a SIL from a table of contents, caching, and combining
//      VTK datasets to make an avtDataset.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Mar 12 15:32:50 PST 2001
//    Added logic to create a SIL from an ordinary table of contents.
//
//    Kathleen Bonnell, Tue Mar 27 08:59:23 PST 2001 
//    Added logic to create a Mesh dataset. 
//
//    Kathleen Bonnell, Mon Apr  9 14:47:12 PDT 2001 
//    changed return type of GetOutput to avtDataTree_p. 
//
//    Kathleen Bonnell, Fri Jun 15 11:34:26 PDT 2001 
//    Added method AddOriginalCellsArray.
//
//    Hank Childs, Fri Aug 17 16:35:12 PDT 2001
//    Inherited from avtDatasetDatabase.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Make GetMaterialList return the material names in an argument.  
//
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001 
//    Added another vector<string> argument to MaterialSelect method. 
//
//    Hank Childs, Tue Oct 23 09:16:53 PDT 2001
//    Added support for reading multiple variables.
//
//    Kathleen Bonnell, Mon Nov 12 11:33:55 PST 2001 
//    Added Query and CreateOriginalZones.
//
//    Jeremy Meredith, Mon Dec 17 14:37:04 PST 2001
//    Added AddSpecies, GetSpeciesDataset, GetSpecies.
//
//    Hank Childs, Wed Dec 19 09:39:15 PST 2001
//    Additional support for species.
//
//    Jeremy Meredith, Thu Mar 14 17:44:19 PST 2002
//    Added support for internal surfaces during material selection.
//
//    Kathleen Bonnell, Tue Mar 26 15:23:11 PST 2002 
//    Added argument to AddOriginalCellsArray.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002  
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Wed Mar 27 15:47:14 PST 2002  
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002 
//    Change AddOriginalCellsArray parameter from 'int &' to 'const int'. 
//
//    Sean Ahern, Tue May 21 11:58:02 PDT 2002
//    Added a virtual method for freeing up resources.
//    
//    Hank Childs, Mon Jun 24 17:35:37 PDT 2002
//    Added support for groups.
//
//    Kathleen Bonnell, Fri Jun 28 09:32:26 PDT 2002
//    Added methods QueryScalars, QueryVectors, QueryMaterial. 
//    
//    Hank Childs, Thu Jul  4 13:21:20 PDT 2002
//    Add better support for mixed variables, as well as secondary variables
//    when reconstructing material interfaces.  Also pushed the struct
//    avtDatasetCollection into its own file.
//
//    Jeremy Meredith, Tue Aug 13 14:48:27 PDT 2002
//    Added extra information to the material selection routines.
//
//    Kathleen Bonnell, Thu Aug 15 17:48:38 PDT 2002 
//    Added an argument to AddGroups, so that collections beneath the
//    group will have the correct titles. 
//
//    Jeremy Meredith, Thu Oct 24 15:37:56 PDT 2002
//    Added smoothing option and clean zones only option for MIR.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Moved Query method to parent class.  Made QueryScalars/Vectors/Material
//    virtual methods. 
//
//    Kathleen Bonnell, Fri Dec  6 16:25:20 PST 2002 
//    Added QueryNodes. 
//    
//    Kathleen Bonnell, Fri Dec 27 14:09:40 PST 2002 
//    Added arguments to QueryNodes. 
//    
//    Kathleen Bonnell, Fri Apr 18 14:11:24 PDT 2003   
//    Added QueryMesh. 
//
//    Brad Whitlock, Wed May 14 09:16:44 PDT 2003
//    I added an optional int argument to SetDatabaseMetaData.
//
//    Jeremy Meredith, Thu Jun 12 09:05:38 PDT 2003
//    Added another flag to MaterialSelect.
//
//    Hank Childs, Wed Jun 18 09:20:16 PDT 2003
//    Added CreateOriginalNodes.
//
//    Kathleen Bonnell, Fri Jun 20 13:52:00 PDT 2003  
//    Added QueryZones, added parameter to other Query methods.
//
//    Hank Childs, Tue Jul 29 15:59:56 PDT 2003
//    Added AssociateBounds and ScaleMesh.
//
//    Kathleen Bonnell, Tue Sep  9 16:51:10 PDT 2003 
//    Changed PickVarInfo argument in QueryMesh to std::string.
//
//    Jeremy Meredith, Mon Sep 15 09:43:26 PDT 2003
//    Added a MIR algorithm flag to MaterialSelect and GetMIR.
//
//    Hank Childs, Mon Sep 22 07:52:34 PDT 2003
//    Added support for tensors.
//
//    Mark C. Miller, 30Sep03, Added support for time varying MD/SIL
//
//    Kathleen Bonnell, Tue Nov 18 14:07:13 PST 2003
//    Add bool and string vector args to QueryNodes, QueryZones, in support
//    of logical zone coords.
//
//    Kathleen Bonnell, Thu Nov 20 15:10:23 PST 2003 
//    Added QuerySpecies. 
//    
//    Kathleen Bonnell, Thu Nov 20 17:47:57 PST 2003 
//    Add 'FindElementForPoint'. 
//
//    Kathleen Bonnell, Wed Dec 17 14:58:31 PST 2003 
//    Updated arguments lists for QueryNodes and QueryZones so that multiple
//    types of coordinates could be retrieved. 
//
// ****************************************************************************

class DATABASE_API avtGenericDatabase : public avtDatasetDatabase
{
  public:
                               avtGenericDatabase(avtFileFormatInterface *);
    virtual                   ~avtGenericDatabase();

    virtual void               GetAuxiliaryData(avtDataSpecification_p,
                                                VoidRefList &,
                                                const char *type, void *args);
    virtual avtDataTree_p      GetOutput(avtDataSpecification_p,
                                         avtSourceFromDatabase *);

    virtual void               FreeUpResources(void);
    virtual bool               CanDoDynamicLoadBalancing(void);
    virtual int                NumStagesForFetch(avtDataSpecification_p);

    virtual const char        *GetFilename(int);

    virtual bool               HasInvariantMetaData(void) const;
    virtual bool               HasInvariantSIL(void) const;

    virtual bool               FindElementForPoint(const char *, const int, 
                                    const int, const char *, float[3], int &);

  protected:
    avtFileFormatInterface    *Interface;
    avtVariableCache           cache;

    int                        lastTimestep;

    virtual void               PopulateSIL(avtSIL *, int=0);
    virtual void               PopulateIOInformation(avtIOInformation &);
    virtual void               SetDatabaseMetaData(avtDatabaseMetaData *,int=0);

    vtkDataSet                *GetDataset(const char *, int, int, const char *,
                                          const std::vector<CharStrRef> &,
                                          avtSourceFromDatabase *);
    vtkDataSet                *GetScalarVarDataset(const char *, int, int,
                                                   const char *);
    vtkDataSet                *GetMeshDataset(const char *, int, int,
                                              const char *);
    vtkDataSet                *GetVectorVarDataset(const char *, int, int,
                                                   const char *);
    vtkDataSet                *GetSymmetricTensorVarDataset(const char *, int,
                                                            int, const char *);
    vtkDataSet                *GetTensorVarDataset(const char *, int, int,
                                                   const char *);
    vtkDataSet                *GetMaterialDataset(const char *, int, int,
                                                  const char *);
    vtkDataSet                *GetSpeciesDataset(const char *, int, int,
                                                 const char *);
    vtkDataArray              *GetScalarVariable(const char *, int, int,
                                                 const char *);
    vtkDataArray              *GetVectorVariable(const char *, int, int,
                                                 const char *);
    vtkDataArray              *GetSymmetricTensorVariable(const char *,int,int,
                                                          const char *);
    vtkDataArray              *GetTensorVariable(const char *, int, int,
                                                 const char *);
    vtkDataArray              *GetSpeciesVariable(const char *, int, int,
                                                  const char *, int);
    void                       AddSecondaryVariables(vtkDataSet *, int, int,
                                                     const char *,
                                              const std::vector<CharStrRef> &);
    vtkDataSet                *GetMesh(const char *, int, int, const char *);

    void                       AddOriginalCellsArray(vtkDataSet *, const int);
    void                       AddOriginalNodesArray(vtkDataSet *, const int);

    bool                       PrepareMaterialSelect(int, bool,
                                                  avtSILRestrictionTraverser &, 
                                                  std::vector<std::string> &);
    avtDataTree_p              MaterialSelect(vtkDataSet *, avtMaterial *, 
                                              std::vector<avtMixedVariable *>,
                                              int, const char *, int,
                                              std::vector<std::string> &,
                                              std::vector<std::string> &,
                                              bool, bool, bool, bool, bool,
                                              bool, int, bool, bool&, bool&,
                                              bool);
    void_ref_ptr               GetMIR(int, const char *, int, vtkDataSet*,
                                      avtMaterial *, int, bool, bool, bool,
                                      int, bool, bool&, bool&, bool);
    avtMaterial               *GetMaterial(int, const char *, int);
    avtSpecies                *GetSpecies(int, const char *, int);
    void                       GetMaterialIndices(const char *,
                                                  std::vector<std::string> &,
                                                  std::vector<int> &);

    void                       ReadDataset(avtDatasetCollection &, 
                                  std::vector<int> &, avtDataSpecification_p &,
                                  avtSourceFromDatabase *);
    bool                       CommunicateGhosts(avtDatasetCollection &,
                                  std::vector<int> &, avtDataSpecification_p &,
                                  avtSourceFromDatabase *);
    bool                       ApplyGhostForDomainNesting(avtDatasetCollection &,
                                  std::vector<int> &, std::vector<int> &,
                                  avtDataSpecification_p &);
    void                       MaterialSelect(avtDatasetCollection &,
                                 std::vector<int> &, avtDataSpecification_p &,
                                 avtSourceFromDatabase *, bool);
    void                       SpeciesSelect(avtDatasetCollection &,
                                 std::vector<int> &, std::vector<bool> &,
                                 avtDataSpecification_p &,
                                 avtSourceFromDatabase *);
    void                       CreateOriginalZones(avtDatasetCollection &,
                                                   std::vector<int> &, 
                                                   avtSourceFromDatabase *);
    void                       CreateOriginalNodes(avtDatasetCollection &,
                                                   std::vector<int> &, 
                                                   avtSourceFromDatabase *);
    void                       CreateStructuredIndices(avtDatasetCollection &,
                                                       avtSourceFromDatabase*);
    void                       UpdateInternalState(int);

    virtual bool               QueryScalars(const std::string &, const int, 
                                            const int , const int ,
                                            const std::vector<int> &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryVectors(const std::string &, const int, 
                                            const int, const int,
                                            const std::vector<int> &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryTensors(const std::string &, const int, 
                                            const int, const int,
                                            const std::vector<int> &, 
                                            PickVarInfo &, const bool);
    virtual bool               QuerySymmetricTensors(const std::string &,
                                            const int, const int, const int,
                                            const std::vector<int> &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryMaterial(const std::string &, const int, 
                                            const int , const int,
                                            const std::vector<int> &, 
                                            PickVarInfo &, const bool);
    virtual bool               QuerySpecies(const std::string &, const int, 
                                            const int , const int ,
                                            const std::vector<int> &, 
                                            PickVarInfo &, const bool);
    virtual bool               QueryNodes(const std::string &, const int, 
                                          const int, const int, 
                                          std::vector<int> &, float [3], 
                                          const int, const bool, const bool,
                                          const bool, std::vector<std::string> &,
                                          std::vector<std::string> &,
                                          std::vector<std::string> &,
                                          const bool, const bool,
                                          std::vector<std::string> &,
                                          std::vector<std::string> &);

    virtual bool               QueryMesh(const std::string &, const int, const int, 
                                         std::string &);

    virtual bool               QueryZones(const std::string&, const int, int &,
                                          const int, std::vector<int> &, 
                                          float [3], const int, const bool, 
                                          const bool, const bool, 
                                          std::vector<std::string> &,
                                          std::vector<std::string> &,
                                          std::vector<std::string> &,
                                          const bool, const bool, 
                                          std::vector<std::string> &,
                                          std::vector<std::string> &); 

    void                       AssociateBounds(vtkDataSet *);
    void                       ScaleMesh(vtkDataSet *);
};


#endif


