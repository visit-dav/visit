// ************************************************************************* //
//                             InputRepository.h                             //
// ************************************************************************* //

#ifndef INPUT_REPOSITORY_H
#define INPUT_REPOSITORY_H

#include <silo.h>


// ****************************************************************************
//  Class: InputRepository
//
//  Purpose:
//      A class that MeshtvPrep can use to store some data in.  It is used to 
//      get some information that is important to several objects being
//      read in by MeshtvPrep and then to share that data with them later.
//      It is not part of the MeshtvPrep class so that the class will not 
//      become too cumbersome.
//
//  Programmer: Hank Childs
//  Creation:   December 8, 1999
//
//  Modifications:
// 
//      Hank Childs, Tue Apr  4 14:26:22 PDT 2000
//      Removed field meshesID and methods GetMeshID and GetMeshName in favor
//      of safer silo calls.
//
// ****************************************************************************

class InputRepository
{
  public:
                  InputRepository();
                 ~InputRepository();

    bool          ContainsDomain(char *filename, char *dirname);

    void          AddMesh(DBquadmesh *, char *);
    void          AddMesh(DBpointmesh *, char *);
    void          AddMesh(DBucdmesh *, char *);
    void          DeleteMeshes();
    DBpointmesh  *GetPointmesh(char *);
    DBquadmesh   *GetQuadmesh(char *);
    DBucdmesh    *GetUcdmesh(char *);
    void          SetNPointmesh(int);
    void          SetNQuadmesh(int);
    void          SetNUcdmesh(int);

    int           GetMeshNZones(char *);

    int           GetVarNDomains(char *);
    int           GetMaterialNDomains(char *);
    int           GetMeshNDomains(char *);

    int           GetMeshDomain(char *, char *, char *);
    int           GetVarDomain(char *, char *, char *);
    int           GetMaterialDomain(char *, char *, char *);

    int           GetNVars(void)       { return nVars; };
    int           GetNMeshes(void)     { return nMeshes; };
    int           GetNMaterials(void)  { return nMaterials; };
    char         *GetVar(int i)      
                      { return (i<0 || i>=nVars ? NULL : varsName[i]); };
    char         *GetMesh(int i)      
                      { return (i<0 || i>=nMeshes ? NULL : meshesName[i]); };
    char         *GetMaterial(int i)      
                      { return (i<0 || i>=nMaterials ? NULL 
                                                     : materialsName[i]); };

    void          ReadRoot(DBfile *, DBtoc *);

  private:
    int           nMeshes;
    int          *meshNDomains;
    char        **meshesName;
    char       ***meshDomainListing;

    int           nVars;
    int          *varNDomains;
    char        **varsName;
    char       ***varDomainListing;

    int           nMaterials;
    int          *materialNDomains;
    char        **materialsName;
    char       ***materialDomainListing;

    bool          readInMesh;

    DBpointmesh **ptmeshes;
    DBquadmesh  **qmeshes;
    DBucdmesh   **ucdmeshes;
    char        **ptmeshnames;
    char        **qmeshnames;
    char        **ucdmeshnames;
    int           ucdmeshesN;
    int           qmeshesN;
    int           ptmeshesN;

    // Private Methods
    void          AddMeshEntry(int, int, char *, char **);
    void          AddVarEntry(int, int, char *, char **);
    void          AddMaterialEntry(int, int, char *, char **);
    
    bool          ContainsDomainForEntry(char *, char *, int, int *, char ***);
    void          GetDomainRange(int, int *, int *);

    void          ReadMeshesFromRoot(DBfile *, DBtoc *);
    void          ReadVarsFromRoot(DBfile *, DBtoc *);
    void          ReadMaterialsFromRoot(DBfile *, DBtoc *);
};


#endif


