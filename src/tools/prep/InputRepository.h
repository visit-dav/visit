/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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


