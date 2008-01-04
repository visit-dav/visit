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
//                               Material.C                                  //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#include <Material.h>


//
// Class scoped constants are statics, so they cannot be initialized
// in the class definition.  Initialize them here.
//

int    const  Material::SILO_NUM_COMPONENTS              = 13; 
char * const  Material::SILO_TYPE                        = "Material";

char * const  Material::SILO_DATA_TYPE_NAME              = "DataType";
char * const  Material::SILO_DOMAIN_MATERIAL_NUMBERS_NAME       
                                                 = "DomainMaterialNumbers";
char * const  Material::SILO_MATERIAL_NUMBERS_NAME       = "MaterialNumbers";
char * const  Material::SILO_MATERIALS_NAME              = "Materials";
char * const  Material::SILO_MESH_NAME                   = "Mesh";
char * const  Material::SILO_MIXED_VF_NAME               = "MixedVF";
char * const  Material::SILO_MIXED_NEXT_NAME             = "MixedNext";
char * const  Material::SILO_MIXED_MATERIAL_NAME         = "MixedMaterial";
char * const  Material::SILO_N_MATERIAL_NUMBERS_NAME     = "NMaterialNumbers";
char * const  Material::SILO_N_MIXED_VALUES_NAME         = "NMixedValues";
char * const  Material::SILO_N_DOMAINS_NAME              = "NDomains";
char * const  Material::SILO_N_DOMAIN_MATERIAL_NUMBERS_NAME
                                                 = "NDomainMaterialNumbers";
char * const  Material::SILO_N_VALUES_NAME               = "NValues";


// ****************************************************************************
//  Method: Material constructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Material::Material()
{
    RealConstructor();
    CreateValues();
}


// ****************************************************************************
//  Method: Material Constructor
//
//  Purpose:
//      Creates a Material object that is incomplete, because the derived types
//      will fill in the Value objects.  We would like the Value objects to be
//      fully created at the time of instantiation, so subsequent method
//      invocations can be in confidence that the object is fully constructed.
//      Unfortunately, this cannot be accomplished with virtual functions since
//      the base type (this one) cannot resolve whether or not it is the base
//      type, except through passing an explicit argument, like the one in this
//      constructor's signature.  Virtual functions do not work in constructors
//      because they may access data members that do not exist yet.  The
//      virtual function table when this method is called for this object is
//      that of the Field class.  If this is a derived type of Field, the
//      VFT will not be replaced with that of the derived type until the
//      constructor for that derived type is called.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

Material::Material(FOR_DERIVED_TYPES_e)
{
    RealConstructor();

    //
    // Not calling CreateValues so the derived type's constructor can.
    //
}


// ****************************************************************************
//  Method: Material::RealConstructor
//
//  Purpose:
//      Because the Material object needs to be created by derived types, there
//      must be multiple (and similar) constructors.   This is the "real"
//      constructor that they all can call.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

void
Material::RealConstructor(void)
{
    name                      = NULL;
    mesh                      = NULL;
    dataType                  = -1;
    nValues                   = 0;
    nMixedValues              = 0;
    nDomains                  = -1;
    nDomainMaterialNumbers    = NULL;
    domainMaterialNumbers     = NULL;
    materials                 = NULL;
    mixedVF                   = NULL;
    mixedNext                 = NULL;
    mixedMaterial             = NULL;
    nMaterialNumbers          = 0;
    materialNumbers           = NULL;
    nDomainMaterialNumbers1D  = 0;
    domainMaterialNumbers1D   = NULL;

    //
    // Initialize the Value objects to NULL, just in case they don't get 
    // initialized later.
    //
    materialsValue     = NULL;
    mixedVFValue       = NULL;
    mixedNextValue     = NULL;
    mixedMaterialValue = NULL;
}


// ****************************************************************************
//  Method: Material::CreateValues
//
//  Purpose:
//      Creates the Value objects for the Material.
//
//  Note:       The necessity of this method is discussed in the constructor.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

void
Material::CreateValues(void)
{
    materialsValue     = new Value();
    mixedVFValue       = new Value();
    mixedNextValue     = new Value();
    mixedMaterialValue = new Value();
}


// ****************************************************************************
//  Method: Material destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Material::~Material()
{
    if (name != NULL)
    {
        delete [] name;
    }
    if (mesh != NULL)
    {
        delete [] mesh;
    }
    if (materialNumbers != NULL)
    {
        delete [] materialNumbers;
    }
    if (nDomainMaterialNumbers != NULL)
    {
        delete [] nDomainMaterialNumbers;
    }
    if (domainMaterialNumbers != NULL)
    {
        for (int i = 0 ; i < nDomains ; i++)
        {
            if (domainMaterialNumbers[i] != NULL)
            {
                delete [] domainMaterialNumbers[i];
            }
        }
        delete [] domainMaterialNumbers;
    }
    if (domainMaterialNumbers1D != NULL)
    {
        delete [] domainMaterialNumbers1D;
    }
    if (materials != NULL)
    {
        delete [] materials;
    }
    if (mixedVF != NULL)
    {
        delete [] mixedVF;
    }
    if (mixedNext != NULL)
    {
        delete [] mixedNext;
    }
    if (mixedMaterial != NULL)
    {
        delete [] mixedMaterial;
    }
    if (materialsValue != NULL)
    {
        delete materialsValue;
    }
    if (mixedVFValue != NULL)
    {
        delete mixedVFValue;
    }
    if (mixedNextValue != NULL)
    {
        delete mixedNextValue;
    }
    if (mixedMaterialValue != NULL)
    {
        delete mixedMaterialValue;
    }
}


// ****************************************************************************
//  Method: Material::Write
//
//  Purpose:
//      Writes out a Material object (and the Values it contains)
//      to a SILO file.
//
//  Note:       The type field for the DBobject, siloObj, must have its memory
//              allocated through a malloc, so that when it is freed, we do
//              not get a free memory mismatch from purify.
//
//  Programmer: Hank Childs
//  Creation:   January 10, 2000
//
// ****************************************************************************

void
Material::Write(DBfile *dbfile)
{
    //
    // Create an object to be written into the SILO file.
    //
    DBobject   *siloObj = DBMakeObject(name, DB_USERDEF, SILO_NUM_COMPONENTS);
    if (siloObj == NULL)
    {
        cerr << "Unable to create a SILO object for the material." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // We must make a copy of the string, because the "destructor" deletes this
    // field and we can't have it delete statics.
    //
    siloObj->type = C_strdup(SILO_TYPE);

    //
    // Add data members to the siloObj.
    //
    char   *validEntry = NULL;
    char   *nullString = "";
    char    absName[LONG_STRING];

    validEntry = (mesh == NULL ? nullString : mesh);
    DBAddStrComponent(siloObj, SILO_MESH_NAME, validEntry);

    DBAddIntComponent(siloObj, SILO_DATA_TYPE_NAME, dataType);
    
    DBAddIntComponent(siloObj, SILO_N_MATERIAL_NUMBERS_NAME, nMaterialNumbers);

    sprintf(absName, "%s%s", name, SILO_MATERIAL_NUMBERS_NAME);
    DBWrite(dbfile, absName, materialNumbers, &nMaterialNumbers, 1, DB_INT);
    DBAddVarComponent(siloObj, SILO_MATERIAL_NUMBERS_NAME, absName);

    DBAddIntComponent(siloObj, SILO_N_VALUES_NAME, nValues);
   
    DBAddIntComponent(siloObj, SILO_N_MIXED_VALUES_NAME, nMixedValues);

    DBAddIntComponent(siloObj, SILO_N_DOMAINS_NAME, nDomains); 

    sprintf(absName, "%s%s", name, SILO_N_DOMAIN_MATERIAL_NUMBERS_NAME);
    DBWrite(dbfile, absName, nDomainMaterialNumbers, &nDomains, 1, DB_INT);
    DBAddVarComponent(siloObj, SILO_N_DOMAIN_MATERIAL_NUMBERS_NAME, absName);

    //
    // domainMaterialNumbers1D is sometimes calculated for us by derived types
    // to avoid issues about calculating in parallel.  Check to see if it has
    // already been calculated.
    //
    if (domainMaterialNumbers1D == NULL)
    {
        //
        // We need to calculate the 1D array.  Use the SILO Obj Lib routine.
        //
        Construct1DArray(nDomains, nDomainMaterialNumbers, 
                         domainMaterialNumbers, &domainMaterialNumbers1D, 
                         &nDomainMaterialNumbers1D);
    }
    sprintf(absName, "%s%s", name, SILO_DOMAIN_MATERIAL_NUMBERS_NAME);
    int   ndims = 1;
    DBWrite(dbfile, absName, domainMaterialNumbers1D, 
            &nDomainMaterialNumbers1D, ndims, DB_INT);
    DBAddVarComponent(siloObj, SILO_DOMAIN_MATERIAL_NUMBERS_NAME, absName);

    validEntry = (materials == NULL ? nullString : materials);
    DBAddStrComponent(siloObj, SILO_MATERIALS_NAME, validEntry);

    validEntry = (mixedVF == NULL ? nullString : mixedVF);
    DBAddStrComponent(siloObj, SILO_MIXED_VF_NAME, validEntry);

    validEntry = (mixedNext == NULL ? nullString : mixedNext);
    DBAddStrComponent(siloObj, SILO_MIXED_NEXT_NAME, validEntry);

    validEntry = (mixedMaterial == NULL ? nullString : mixedMaterial);
    DBAddStrComponent(siloObj, SILO_MIXED_MATERIAL_NAME, validEntry);

    DBWriteObject(dbfile, siloObj, 0);
    DBFreeObject(siloObj);

    //
    // Write out the Value objects contained by this object.
    //
    materialsValue->Write(dbfile);
    mixedVFValue->Write(dbfile);
    mixedNextValue->Write(dbfile);
    mixedMaterialValue->Write(dbfile);
}


// ****************************************************************************
//  Method: Material::Read
//
//  Purpose:  
//      Reads in a Material object from a DBobject.
//
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void
Material::Read(DBobject *siloObj, DBfile *dbfile)
{
    int  i, j;
    if (strcmp(siloObj->type, SILO_TYPE) != 0)
    {
        //
        // A mistake was made in trying to create this object.
        //
        cerr << "Cannot create a material object from a " << siloObj->type
             << " object." << endl;
        exit(EXIT_FAILURE);
    }

    //
    // Copy the name.
    //
    name = CXX_strdup(siloObj->name);

    //
    // Populate the object by going through the components.
    //
    char  *materialNumbersName        = NULL;
    char  *nDomainMaterialNumbersName = NULL;
    char  *domainMaterialNumbersName  = NULL;
    for (i = 0 ; i < siloObj->ncomponents ; i++)
    {
        if (strcmp(siloObj->comp_names[i], SILO_DATA_TYPE_NAME) == 0)
        {
            dataType = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], 
                        SILO_DOMAIN_MATERIAL_NUMBERS_NAME) == 0)
        {
            domainMaterialNumbersName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MATERIAL_NUMBERS_NAME)==0)
        {
            materialNumbersName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MATERIALS_NAME) == 0)
        {
            materials = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MESH_NAME) == 0)
        {
            mesh = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MIXED_VF_NAME) == 0)
        {
            mixedVF = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MIXED_NEXT_NAME) == 0)
        {
            mixedNext = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_MIXED_MATERIAL_NAME) == 0)
        {
            mixedMaterial = ParsePDBNameString(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_MATERIAL_NUMBERS_NAME) 
                 == 0)
        {
            nMaterialNumbers = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_MIXED_VALUES_NAME) == 0)
        {
            nMixedValues = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_DOMAINS_NAME) == 0)
        {
            nDomains = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else if (strcmp(siloObj->comp_names[i], 
                        SILO_N_DOMAIN_MATERIAL_NUMBERS_NAME) == 0)
        {
            nDomainMaterialNumbersName = siloObj->pdb_names[i];
        }
        else if (strcmp(siloObj->comp_names[i], SILO_N_VALUES_NAME) == 0)
        {
            nValues = ParsePDBNameInt(siloObj->pdb_names[i]);
        }
        else
        {
            cerr << "Invalid component " << siloObj->comp_names[i] 
                 << " for Material." << endl;
            exit(EXIT_FAILURE);
        }
    }   // End 'for' over all the components in the DBobject

    //
    // Read in the material numbers array.  Allocate our own memory so we don't
    // run into problems with purify and C-alloc/C++-dealloc.
    //
    if (nMaterialNumbers <= 0)
    {
        cerr << "Didn't read in nMaterialNumbers; can't create array." << endl;
        exit(EXIT_FAILURE);
    }
    if (materialNumbersName == NULL)
    {
        cerr << "Didn't find material numbers name, can't read array." << endl;
        exit(EXIT_FAILURE);
    }
    materialNumbers = new int[nMaterialNumbers];
    DBReadVar(dbfile, materialNumbersName, materialNumbers);

    //
    // Read in the domain material numbers stuff.  First read in the 1D
    // array and then calculate the 2D array.  Note that the 1D array is
    // stored in the SILO file and uses the name for the 2D array.
    //
    if (nDomains <= 0)
    {
        cerr << "Didn't read in nDomains; can't create array." << endl;
        exit(EXIT_FAILURE);
    }
    if (nDomainMaterialNumbersName == NULL)
    {
        cerr << "Didn't find the n-domain material numbers domain." << endl;
        exit(EXIT_FAILURE);
    }
    if (domainMaterialNumbersName == NULL)
    {
        cerr << "Didn't find the domain material numbers domain." << endl;
        exit(EXIT_FAILURE);
    }
    nDomainMaterialNumbers1D 
                          = DBGetVarLength(dbfile, domainMaterialNumbersName);
    domainMaterialNumbers1D = new int[nDomainMaterialNumbers1D];
    DBReadVar(dbfile, domainMaterialNumbersName, domainMaterialNumbers1D);

    nDomainMaterialNumbers = new int[nDomains];
    DBReadVar(dbfile, nDomainMaterialNumbersName, nDomainMaterialNumbers);

    int  count = 0;
    domainMaterialNumbers = new int*[nDomains];
    for (i = 0 ; i < nDomains ; i++)
    {
        domainMaterialNumbers[i] = new int[nDomainMaterialNumbers[i]];
        for (j = 0 ; j < nDomainMaterialNumbers[i] ; j++)
        {
            domainMaterialNumbers[i][j] = domainMaterialNumbers1D[count];
            count++;
        }
   }
}


// ****************************************************************************
//  Method: Material::PrintSelf
//
//  Purpose:
//      Prints out a Material.  For debugging purposes.
//
//  Programmer: Hank Childs
//  Creation:   January 31, 2000
//
// ****************************************************************************

void
Material::PrintSelf(ostream &out)
{
    out << "Material (" << name << ")" << endl;
    
    out << "\tMesh         = " << (mesh == NULL ? "(nil)" : mesh) << endl;
    out << "\tdata type    = " << dataType << endl;
    out << "\tnValues      = " << nValues << endl;
    out << "\tnMixedValues = " << nMixedValues << endl;
    out << "\tmaterial     = " << (materials == NULL ? "(nil)" : materials) 
                               << endl;
    out << "\tmixedVF      = " << (mixedVF == NULL ? "(nil)" : mixedVF) 
                               << endl;
    out << "\tmixedNext    = " << (mixedNext == NULL ? "(nil)" : mixedNext) 
                               << endl;
    out << "\tmixedMat     = " << (mixedMaterial == NULL ? "(nil)" 
                                   : mixedMaterial) << endl;

    out << "\tnMaterial#s  = " << nMaterialNumbers << endl;
    out << "\tmaterial#s   = {" ;
    if (materialNumbers == NULL)
    {
        out << "(nil)";
    }
    else
    {
        for (int i = 0 ; i < nMaterialNumbers ; i++)
        {
            out << materialNumbers[i] << (i%20 == 19 ? "\n\t\t\t" : ", ");
        }
    }
    out << "}" << endl;

    out << "\tnDomains     = " << nDomains << endl;
    out << "\tDomainMat#s  = {";
    if (nDomainMaterialNumbers == NULL || domainMaterialNumbers == NULL)
    {
        out << "(nil)";
    }
    else
    {
        out << endl;
        for (int i = 0 ; i < nDomains ; i++)
        {
            out << "\t\t\t(" << nDomainMaterialNumbers[i] << ": ";
            for (int j = 0 ; j < nDomainMaterialNumbers[i] ; j++)
            {
                out << domainMaterialNumbers[i][j] << ", ";
            }
            out << ")" << endl;
        }
    }
    out << "\t}" << endl;
}


