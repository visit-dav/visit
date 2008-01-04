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
//                                 Field.h                                   //
// ************************************************************************* //

#ifndef FIELD_H
#define FIELD_H
#include <siloobj_exports.h>

#include <visitstream.h>
#include <silo.h>

#include <Value.h>
#include <SiloObjLib.h>
#include <IntervalTree.h>


// ****************************************************************************
//  Class: Field
// 
//  Purpose:
//      Keeps information about a field.
//
//  Data Members:
//      mesh              The name of the mesh associated with this field.
//      intervalTree      The name of the interval tree associated with this
//                        field.
//      centering         The centering of the variable.
//      dataType          The data type of the field.
//      mixedMaterial     The name of the material object associated with this
//                        field.
//      units             The units associated with the variable.
//      nValues           The total number of values in the field.
//      nMixedValues      The number of mixed values in the field.
//      nDomains          The number of domains.
//      values            The name of the value object assocated with this
//                        field.
//      mixedValues       The name of the value object associated with the
//                        mixed values.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
//  Modifications:
//
//      Hank Childs,   Mon Jun 12 14:08:17 PDT 2000
//      Added fields dims, dimsObject, and SILO_DIMS_NAME.
//
// ****************************************************************************

class SILOOBJ_API Field
{
  public:
                      Field();
                      Field(FOR_DERIVED_TYPES_e);
    virtual          ~Field();

    char             *GetName(void)       { return name; };
    int               GetCentering(void)  { return centering; };

    void              Read(DBobject *);
    virtual void      Write(DBfile *);
    virtual void      WriteIntervalTree(DBfile *);

    void              PrintSelf(ostream &);

  protected:
    char             *mesh;
    char             *intervalTree;
    int               centering;
    int               dataType;
    char             *mixedMaterial;
    char             *units;
    int               nValues;
    int               nMixedValues;
    int               nDomains;
    char             *dims;
    char             *values;
    char             *mixedValues;

    char             *name;

    Value            *dimsObject;
    Value            *valueObject;
    Value            *mixedValueObject;
    IntervalTree     *intervalTreeObject;

    // Protected methods
    void              CreateValues(void);
    void              RealConstructor(void);
     
    // Constants
  public:
    static char * const   SILO_TYPE;
  protected:
    static char * const   DIMS_NAME;
    static char * const   MIXED_VALUES_NAME;
    static int    const   SILO_NUM_COMPONENTS;
    static char * const   SILO_CENTERING_NAME;
    static char * const   SILO_DATA_TYPE_NAME;
    static char * const   SILO_DIMS_NAME;
    static char * const   SILO_INTERVAL_TREE_NAME;
    static char * const   SILO_MESH_NAME;
    static char * const   SILO_MIXED_MATERIAL_NAME;
    static char * const   SILO_MIXED_VALUES_NAME;
    static char * const   SILO_N_DOMAINS_NAME;
    static char * const   SILO_N_MIXED_VALUES_NAME;
    static char * const   SILO_N_VALUES_NAME;
    static char * const   SILO_UNITS_NAME;
    static char * const   SILO_VALUES_NAME;
};


#endif


