// ************************************************************************* //
//                              MeshtvPrep.h                                 //
// ************************************************************************* //

#ifndef MESHTV_PREP_H
#define MESHTV_PREP_H

#include <silo.h>

#include <CommandLine.h>
#include <Field_Prep.h>
#include <InputRepository.h>
#include <IntervalTree.h>
#include <Material_Prep.h>
#include <Mesh_Prep.h>
#include <MeshtvPrepLib.h>
#include <NamingConvention.h>
#include <Resample.h>
#include <StructuredTopology_Prep.h>
#include <TimeSequence_Prep.h>
#include <UnstructuredTopology_Prep.h>
#include <Value.h>


// ****************************************************************************
//  Class: MeshtvPrep
//
//  Purpose:
//      A class that contains all of the objects that will be read from and
//      written to files.
//
//  Programmer: Hank Childs
//  Creation:   December 3, 1999
// 
//  Modifications:
//
//    Hank Childs, Mon Jun 12 17:04:27 PDT 2000
//    Added doSiloConversion switch.
//
//    Hank Childs, Mon Jun 26 09:47:08 PDT 2000
//    Allowed for resampling to be done with different resolutions in each
//    dimension.
//
//    Hank Childs, Fri Sep  8 09:59:21 PDT 2000
//    Added calculation of root interval trees.
//    
// ****************************************************************************

class MeshtvPrep
{
  public:
                                 MeshtvPrep();
                                ~MeshtvPrep();

    void                         PreProcess(CommandLine &);
 
  private:
    NamingConvention            *code;
    Field_Prep                  *fields;
    Field_Prep                  *coords;
    int                          statesN;
    int                          fieldsN;
    Mesh_Prep                   *meshes;
    int                          meshesN;
    TimeSequence_Prep            time;
    Material_Prep               *materials;
    int                          materialsN;

    IntervalTree_Prep           *fieldIT;
    int                          fieldITN;
    IntervalTree_Prep           *meshIT;
    int                          meshITN;

    char                        *outPrefix;
    char                       **validVars;
    int                          validVarsN;
    int                          nFiles;
    InputRepository             *repository;
     
    Resample                    *lowres;
    Resample                    *medres;
    int                          lowresN[3];
    int                          medresN[3];

    StructuredTopology_Prep     *structuredTopology;
    int                          structuredTopologyN;
    UnstructuredTopology_Prep   *unstructuredTopology;
    int                          unstructuredTopologyN;

    bool                         doLowRes;
    bool                         doMedRes;
    bool                         doIntervalTrees;
    bool                         doOnionPeel;
    bool                         doSiloConversion;

    void                 ConsolidateObjects(void);
    void                 CleanupState(void);
    void                 CreateCoords(DBtoc *);
    void                 CreateDirectoryStructure(DBfile *);
    void                 CreateFields(DBtoc *);
    void                 CreateMaterials(DBtoc *);
    void                 CreateMeshes(DBtoc *);
    void                 CreateObjects(DBfile *, DBtoc *);
    void                 CreateOutputFiles(int);
    void                 CreateRootIntervalTrees(void);
    void                 CreateTopologies(DBfile *, DBtoc *);
    void                 DeleteMeshes(void);
    void                 DetermineFieldSizes(DBfile *, DBtoc *, char *,char *);
    void                 DetermineGeneratingCode(char **, int);
    void                 DetermineMaterialSizes(DBfile *,DBtoc *,char*,char *);
    void                 DetermineMeshExtents(DBfile *,DBtoc *,char *, char *);
    void                 DetermineMeshSizes(DBfile *, DBtoc *, char *, char *);
    void                 DetermineTopologySizes(DBfile *,DBtoc *,char*,char *);
    int                  FieldIndex(const char *);
    int                  GetFieldIndex(const char *);
    DBfile              *GetOutputFile(RESOURCE_e, int, int);
    void                 GetOutputFilename(RESOURCE_e, int, char *, int);
    void                 InitialPass(DBfile *, DBtoc *, char *, char *);
    void                 Initialize(CommandLine &);
    void                 InitializeState(void);
    void                 IterateDirs(
                             void (MeshtvPrep::*)(DBfile *, DBtoc *, 
                                               char *, char *), 
                             DBfile *, DBtoc *, char *, char *);
    void                 MakeInitialPass(int);
    int                  MaterialIndex(const char *);
    int                  MeshIndex(const char *);
    void                 ObjectReadRootFiles(DBfile *, DBtoc *);
    void                 PopulateRootIntervalTrees(int);
    void                 ProcessPointmesh(DBfile *, DBtoc *,char *,char *,int);
    void                 ProcessQuadmesh(DBfile *, DBtoc *,char *,char *, int);
    void                 ProcessUcdmesh(DBfile *, DBtoc *, char *, char *,int);
    void                 ProcessMeshvar(DBfile *, DBtoc *, char *,char *, int);
    void                 ProcessUcdvar(DBfile *, DBtoc *, char *, char *, int);
    void                 ProcessQuadvar(DBfile *, DBtoc *, char *,char *, int);
    void                 ReadDir(DBfile *, DBtoc *, char *, char *);
    void                 ReadInMeshes(DBfile *, DBtoc *);
    void                 ReadMaterials(DBfile *, DBtoc *, char *, char *);
    void                 ReadMeshes(DBfile *, DBtoc *, char *, char *);
    void                 ReadNormalFiles(int);
    void                 ReadRootFiles(int);
    void                 ReadVars(DBfile *, DBtoc *, char *, char *);
    void                 RelinquishOutputFile(RESOURCE_e, DBfile *);
    void                 SetStatesN(int);
    int                  StructuredTopologyIndex(char *);
    int                  UnstructuredTopologyIndex(char *);
    bool                 ValidName(const char *);
    void                 WrapUpObjects(void);
    void                 WriteFields(DBfile *);
    void                 WriteMaterials(DBfile *);
    void                 WriteMeshes(DBfile *);
    void                 WriteOutTimeInvariantFiles(void);
    void                 WriteOutMeshFiles(void);
    void                 WriteOutStateFiles(void);
    void                 WriteOutVisit(void);
    void                 WriteTopologies(DBfile *);

    // Constants
    static char * const  COORDS_NAME;
    static char * const  LOW_RES_DIR;
    static char * const  MED_RES_DIR;
};


#endif


