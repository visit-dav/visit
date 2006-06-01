/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                                Material.h                                 //
// ************************************************************************* //

#ifndef MATERIAL_H
#define MATERIAL_H
#include <siloobj_exports.h>

#include <visitstream.h>

#include <silo.h>

#include <SiloObjLib.h>
#include <Value.h>


// ****************************************************************************
//  Class : Material
//
//  Purpose:
//      Contains information about materials.
//
//  Data Members:
//      name                       The name of this material.
//      mesh                       The name of the mesh associated with this
//                                 material field.
//      dataType                   The data type of the volume fractions.
//      nMaterialNumbers           The number of material numbers. 
//      materialNumbers            The material numbers.  This is an array of
//                                 size nMaterialNumbers.
//      nValues                    The total number of values in the material
//                                 field.
//      nMixedValues               The total number of mixed values in the
//                                 material field.
//      nDomains                   The total number of domains.
//      nDomainMaterialNumbers     The number of material numbers for each
//                                 domain.  This is an array of size nDomains.
//      domainMaterialNumbers      The material numbers for each domain.  This
//                                 is an array of size nDomains and the ith
//                                 entry contain an array of size 
//                                 nDomainMaterialNumbers[i].
//      materials                  The name of the value object associated 
//                                 with the material numbers.
//      mixedVF                    The name of the value object with the mixed
//                                 material volume fractions.
//      mixedNext                  The name of the value object with the mixed
//                                 material next indices.
//      mixedMaterial              The name of the value object with the mixed
//                                 material numbers.
//      nDomainMaterials#s1D       The size of the domainMaterialNumbers if
//                                 it was compressed to be a 1D array.
//      domainMaterial#s1D         An array that contains the domainMat#s 
//                                 compressed to a 1D array.  This is 
//                                 important for writing to SILO files.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

class SILOOBJ_API Material
{
  public:
                       Material();
                       Material(FOR_DERIVED_TYPES_e);
    virtual           ~Material();
    
    char              *GetName(void) { return name; };

    void               PrintSelf(ostream &);
    void               Read(DBobject *, DBfile *);
    virtual void       Write(DBfile *);

    int                GetNMaterialNumbers(void) { return nMaterialNumbers; };
    const int         *GetMaterialNumbers(void)  { return materialNumbers;  };
    int                GetNDomains(void)         { return nDomains; };
    const char        *GetMesh(void)             { return mesh; };

  protected:
    char              *name;

    char              *mesh;
    int                dataType;
    int                nMaterialNumbers;
    int               *materialNumbers;
    int                nValues;
    int                nMixedValues;
    int                nDomains;
    int               *nDomainMaterialNumbers;
    int              **domainMaterialNumbers;
    char              *materials;
    char              *mixedVF;
    char              *mixedNext;
    char              *mixedMaterial;
    
    Value             *materialsValue;
    Value             *mixedVFValue;
    Value             *mixedNextValue;
    Value             *mixedMaterialValue;
 
    int                nDomainMaterialNumbers1D;
    int               *domainMaterialNumbers1D;

    // Protected methods
    virtual void       CreateValues(void);
    void               RealConstructor(void);

    // Class-scoped constants
  public:
    static char * const   SILO_TYPE;
  protected:
    static int    const   SILO_NUM_COMPONENTS;
    static char * const   SILO_DATA_TYPE_NAME;
    static char * const   SILO_DOMAIN_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_MATERIALS_NAME;
    static char * const   SILO_MESH_NAME;
    static char * const   SILO_MIXED_VF_NAME;
    static char * const   SILO_MIXED_NEXT_NAME;
    static char * const   SILO_MIXED_MATERIAL_NAME;
    static char * const   SILO_N_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_N_MIXED_VALUES_NAME;
    static char * const   SILO_N_DOMAINS_NAME;
    static char * const   SILO_N_DOMAIN_MATERIAL_NUMBERS_NAME;
    static char * const   SILO_N_VALUES_NAME;
};


#endif


