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
//                              Field_Prep.h                                 //
// ************************************************************************* //

#ifndef FIELD_PREP_H
#define FIELD_PREP_H

#include <Field.h>
#include <IntervalTree_Prep.h>
#include <Value_Prep.h>


// ****************************************************************************
//  Class: Field_Prep
// 
//  Purpose:
//      A derived class of Field that is meant exclusively for meshtvprep.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
//  Modifications:
//
//      Hank Childs, Mon Jun 12 14:21:56 PDT 2000
//      Added aliasedDimsObject.
//
//      Hank Childs, Tue Jun 13 12:05:17 PDT 2000
//      Added routines to allow for separation of creating Field object and
//      interval tree.
//
// ****************************************************************************

class Field_Prep : public Field
{
  public:
                        Field_Prep();
                       ~Field_Prep();

    char               *GetMeshName() { return mesh; };
    void                SetName(char *, char *);
    void                SetMeshName(char *);
    void                SetNDomains(int);

    void                DetermineUcdMeshSize(int, DBfile *, char *);
    void                DeterminePointMeshSize(int, DBfile *, char *);
    void                DetermineQuadMeshSize(int, DBfile *, char *);
    void                DetermineUcdvarSize(int, DBfile *, char *);
    void                DetermineMeshvarSize(int, DBfile *, char *);
    void                DetermineQuadvarSize(int, DBfile *, char *);

    void                ReadVarAndWriteArrays(DBfile *, DBmeshvar *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBpointmesh *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBquadmesh *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBquadvar *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBucdmesh *, int);
    void                ReadVarAndWriteArrays(DBfile *, DBucdvar *, int);

    void                IntervalTreeReadVar(DBpointmesh *, int);
    void                IntervalTreeReadVar(DBmeshvar *, int);
    void                IntervalTreeReadVar(DBquadmesh *, int);
    void                IntervalTreeReadVar(DBquadvar *, int);
    void                IntervalTreeReadVar(DBucdmesh *, int);
    void                IntervalTreeReadVar(DBucdvar *, int);

    void                Consolidate(void);
    void                WrapUp(void);
    void                WrapUpIntervalTree(void);
    virtual void        Write(DBfile *);
    virtual void        WriteIntervalTree(DBfile *);

    IntervalTree_Prep  *GetIntervalTree(void)  
                            { return aliasedIntervalTreeObject; };

  protected:
    bool                readCentering;
    bool                readDataType;
    bool                readMeshName;
    bool                readUnits;

    Value_Prep         *aliasedValueObject;
    Value_Prep         *aliasedDimsObject;
    Value_Prep         *aliasedMixedValueObject;
    IntervalTree_Prep  *aliasedIntervalTreeObject;

    // Protected methods
    void                ReadVar(char *, int, int);
    virtual void        CreateValues(void);
 
    // Constants
    static int    const   N_DIMS_LIMIT;
    static char * const   SILO_LOCATION;
};


#endif


