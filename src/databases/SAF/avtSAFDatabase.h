/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               avtSAFDatabase.h                            //
// ************************************************************************* //

#ifndef AVT_SAF_DATABASE_H
#define AVT_SAF_DATABASE_H
#include <database_exports.h>

#include <avtDatasetDatabase.h>
#include <vtkSystemIncludes.h>

class vtkUnstructuredGrid;

//
// The extern "C" around saf.h shouldn't be necessary but SAF doesn't have
// enough of them in their header files so this is done as a work around.
//
extern "C"
{
#include <saf.h>
}

struct SILNode
{
    SAF_Set            setSet;
    char              *setName;
    int                setId;
    bool               setSelfStored;
    int                setCellType;           // VTK cell type
    SAF_Rel            setTopoRelation;
    int                nCats;
    char             **catNames;
    SILCategoryRole   *catRoles;
    int               *catNSubsets;
    struct SILNode  ***catSubsets;
};

// ****************************************************************************
//  Class: avtSAFDatabase
//
//  Purpose:
//      A stand-in for the avtSAFDatabase class used by VisIt.
//
//  Programmer: Jim Reus
//  Creation:   16Mar2001
//
//  Modifications:
//
//    Hank Childs, Fri Aug 17 16:35:12 PDT 2001
//    Inherited from avtDatasetDatabase.
//
//    Eric Brugger, Thu Mar  7 08:12:09 PST 2002
//    I added the Query method.
//
//    Eric Brugger, Thu Sep  5 11:24:47 PDT 2002
//    Completely rewritten.
//
// ****************************************************************************

class DATABASE_API avtSAFDatabase : public avtDatasetDatabase
{
  public:
                              avtSAFDatabase(const char*);
    virtual                  ~avtSAFDatabase();

    virtual avtDataTree_p     GetOutput(avtDataSpecification_p,
                                        avtSourceFromDatabase *);

    virtual void              GetAuxiliaryData(avtDataSpecification_p,
                                               VoidRefList &,
                                               const char*,
                                               void*);

    virtual void              Query(avtDataSpecification_p,
                                    PickAttributes *);

  protected:

    virtual void              SetDatabaseMetaData(avtDatabaseMetaData*);
    virtual void              PopulateSIL(avtSIL*);

  private:
                              avtSAFDatabase();
                              avtSAFDatabase(const avtSAFDatabase&);

    int                       AddSet(avtSIL*, struct SILNode*, bool);
    void                      GetDataSets(vtkDataSet **, avtSILRestriction_p,
                                          int, SAF_Field, SAF_Field);
    int                       GetFieldNComponents(SAF_Field);
    vtkUnstructuredGrid      *GetUnstructuredGrid(int, SAF_Field, SAF_Field);
    void                      PopulateTopologySets(SILNode *);
    struct SILNode           *ReadSet(SAF_Set, int, SAF_Cat*);
    void                      ReadSetInformation();

    SAF_Db                    db;
    DSL_Boolean_t             initialized;
    DSL_Boolean_t             opened;

    SAF_Cat                   nodeCat;
    SAF_Cat                   edgeCat;
    SAF_Cat                   faceCat;
    SAF_Cat                   elemCat;

    int                       nSuites;
    SAF_Suite                *suites;
    char                    **suiteNames;
    SAF_Field                *suiteCoords;
    size_t                   *suiteCoordsCount;
    int                     **suiteCoordsValues;
    SAF_StateFld             *stateFields;

    int                       nStates;
    SAF_StateTmpl             stateTemplate;
    int                       nFields;
    SAF_Field               **fields;
    char                    **fieldNames;
    int                      *fieldNComponents;
    DSL_Boolean_t            *fieldSelfStored;
    char                    **fieldMeshNames;
    avtCentering             *fieldCentering;
    int                       coords;

    int                       nTopSets;
    SAF_Set                  *topSets;

    struct SILNode           *SIL;
    int                       SILId;

    int                       nTopologySets;
    SILNode                 **topologySets;

    int                       iblock;
};

#endif

