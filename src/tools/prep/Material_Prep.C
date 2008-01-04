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
//                             Material_Prep.C                               //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif

#include <Material_Prep.h>


//
// Class scoped constants are statics, so they cannot be initialized
// in the class definition.  Initialize them here.
//

int    const  Material_Prep::MATERIAL_NUMBERS_SIZE            = 100;
char * const  Material_Prep::SILO_LOCATION                    = "/";


// ****************************************************************************
//  Method: Material_Prep constructor
// 
//  Note:       Because this is a derived type of Material and because it would
//              like the data members Value to be a derived type of Value,
//              a special constructor for Material is called that indicates 
//              that the Values should not be created.  For reasons why this 
//              can't be done through a virtual function, see the note in the 
//              Material constructor.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

Material_Prep::Material_Prep() : Material(DERIVED_TYPE_CONSTRUCTOR)
{
    setOnceFields             = false;
    nMaterialNumbersAlloc     = 0;

    //
    // This will set the materialNumbers and nMaterialNumbersAlloc fields.
    //
    AllocMaterialNumbers(MATERIAL_NUMBERS_SIZE);

    CreateValues();
}


// ****************************************************************************
//  Method: Material_Prep destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

Material_Prep::~Material_Prep()
{
    //
    // Do not delete aliased Value because the base class also has a reference
    // to them and will delete them.
    //
}


// ****************************************************************************
//  Method: Material_Prep::CreateValues
//
//  Purpose:
//      Creates the derived type of the Value objects appropriate for this
//      class.  That type is Value_Prep.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

void
Material_Prep::CreateValues(void)
{
     aliasedMaterialsValue     = new Value_Prep;
     aliasedMaterialsValue->SetResource(STATE_FILE);
     aliasedMixedVFValue       = new Value_Prep;
     aliasedMixedVFValue->SetResource(STATE_FILE);
     aliasedMixedNextValue     = new Value_Prep;
     aliasedMixedNextValue->SetResource(STATE_FILE);
     aliasedMixedMaterialValue = new Value_Prep;
     aliasedMixedMaterialValue->SetResource(STATE_FILE);

     // 
     // We have created the value object we would like to our Values.  We
     // cannot use the non-aliased Value objects because they are the wrong
     // type.  Alias them here so that we can use the methods for a Value_Prep,
     // but the base methods for Material will still have access to the Value
     // objects and the work that we did.
     //
     materialsValue     = aliasedMaterialsValue;
     mixedVFValue       = aliasedMixedVFValue;
     mixedNextValue     = aliasedMixedNextValue;
     mixedMaterialValue = aliasedMixedMaterialValue;
}


// ****************************************************************************
//  Method:  Material_Prep::SetName
//
//  Purpose:
//      Sets the name of the Material_Prep object and the Value objects it 
//      contains.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
// ****************************************************************************

void
Material_Prep::SetName(char *n)
{
    //
    // Set this materials name.
    //
    name = CXX_strdup(n);
 
    //
    // Set the name of all the Value objects to which it has a reference.
    //
    // For all of these, the Value object will append its name to the end
    // of the string again, so make two copies: one to send to the Value 
    // object and then another that will have the same name as the Value
    // object.
    //
    char   s[LONG_STRING];

    sprintf(s, "%s%s", name, SILO_MATERIALS_NAME);
    aliasedMaterialsValue->SetFieldName(s);
    sprintf(s, "%s%s%s", name, SILO_MATERIALS_NAME, Value::NAME);
    materials = CXX_strdup(s);

    sprintf(s, "%s%s", name, SILO_MIXED_VF_NAME);
    aliasedMixedVFValue->SetFieldName(s);
    sprintf(s, "%s%s%s", name, SILO_MIXED_VF_NAME, Value::NAME);
    mixedVF = CXX_strdup(s);
 
    sprintf(s, "%s%s", name, SILO_MIXED_NEXT_NAME);
    aliasedMixedNextValue->SetFieldName(s);
    sprintf(s, "%s%s%s", name, SILO_MIXED_NEXT_NAME, Value::NAME);
    mixedNext = CXX_strdup(s);

    sprintf(s, "%s%s", name, SILO_MIXED_MATERIAL_NAME);
    aliasedMixedMaterialValue->SetFieldName(s);
    sprintf(s, "%s%s%s", name, SILO_MIXED_MATERIAL_NAME, Value::NAME);
    mixedMaterial = CXX_strdup(s);
}


// ****************************************************************************
//  Method: Material_Prep::SetNDomains
//
//  Purpose:
//      Sets the nDomains field for this object and the objects it contains.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
// ****************************************************************************

void
Material_Prep::SetNDomains(int nDoms)
{
    // 
    // Set the fields that depend on nDomains.
    //
    nDomains = nDoms;
    nDomainMaterialNumbers  = new int[nDomains];
    domainMaterialNumbers   = new int*[nDomains];
    for (int i = 0 ; i < nDomains ; i++)
    {
        nDomainMaterialNumbers[i] = 0;
        domainMaterialNumbers[i]  = NULL;
    }
    aliasedMaterialsValue->SetNDomains(nDomains);
    aliasedMaterialsValue->SetNVals(1);
    aliasedMixedVFValue->SetNDomains(nDomains);
    aliasedMixedVFValue->SetNVals(1);
    aliasedMixedNextValue->SetNDomains(nDomains);
    aliasedMixedNextValue->SetNVals(1);
    aliasedMixedMaterialValue->SetNDomains(nDomains);
    aliasedMixedMaterialValue->SetNVals(1);
}


// ****************************************************************************
//  Method:  Material_Prep::SetOnceFields
//
//  Purpose:
//      Sets the fields that only need to be set one time.  These include
//      the name and all of the fields that are references to Value objects
//      which depend on the name, and the datatype.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

void
Material_Prep::SetOnceFields(int datatype, char *meshname)
{
    if (setOnceFields == true)
    {
        //
        // This routine is only for fields that need to be set one time.  If
        // it has already been called, then exit.
        //
        return;
    }

    //
    // Set the other data fields that should only be set one time.
    // 
    mesh = CXX_strdup(meshname);
    dataType  = datatype;

    setOnceFields    = true;
}


// ****************************************************************************
//  Method:  Material_Prep::AddMaterial
//
//  Purpose:
//      Populates the Material_Prep object with information from a DBmaterial.
//
//  Arguments:
//      mat       -  A reference to the material for this domain.
//      meshname  -  The name of the mesh for this material.
//      nZones    -  The number of zones for this domain.
//      nDomains  -  The total number of domains.
//      domain    -  The domain number for the current domain.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2000
//
// ****************************************************************************

void
Material_Prep::AddMaterial(DBmaterial *mat, char *meshname, int nZones, 
                           int domain, DBfile *outFile)
{
    //
    // Let SetOnceFields worry about setting the fields that are only set 
    // one time.
    //
    SetOnceFields(mat->datatype, meshname);

    //
    // Set the domain material numbers.
    //
    nDomainMaterialNumbers[domain] = mat->nmat;
    domainMaterialNumbers[domain]  = new int[nDomainMaterialNumbers[domain]];
    for (int i = 0 ; i < nDomainMaterialNumbers[domain] ; i++)
    {
        domainMaterialNumbers[domain][i] = mat->matnos[i];
    }

    //
    // Update running count of how many values are contained by the Value 
    // objects the Material_Prep object parents.
    //
    nValues      += nZones;
    nMixedValues += mat->mixlen;
 
    AddToMaterialList(mat->nmat, mat->matnos);

    //
    // Write out the arrays to the new file.
    //
    void   *val = NULL;

    val = static_cast<void *>( mat->matlist );
    aliasedMaterialsValue->WriteArray(outFile, &val, DB_INT, domain);

    val = static_cast<void *>( mat->mix_vf );
    aliasedMixedVFValue->WriteArray(outFile, &val, DB_FLOAT, domain);

    val = static_cast<void *>( mat->mix_next );
    aliasedMixedNextValue->WriteArray(outFile, &val, DB_INT, domain);

    val = static_cast<void *>( mat->mix_mat );
    aliasedMixedMaterialValue->WriteArray(outFile, &val, DB_INT, domain);
}


// ****************************************************************************
//  Method: Material_Prep::AddToMaterialList
//
//  Purpose:
//      Iterates through a list of materials and adds the materials not on
//      our master list to the master list.
//
//  Programmer: Hank Childs
//  Creation:   January 10, 2000
//
// ****************************************************************************

void
Material_Prep::AddToMaterialList(int nmat, int *matnos)
{
    int  i, j;

    //
    // Go through the current list of materials and see if there are any
    // new materials to add to the list.
    //
    for (i = 0 ; i < nmat; i++)
    {
        bool   haveMaterial = false;
        for (j = 0 ; j < nMaterialNumbers ; j++)
        {
            if (materialNumbers[j] == matnos[i])
            {
                //
                // We already have this material in the list.
                //
                haveMaterial = true;
                break;
            }

        }     // End 'for' over all the materials in the Material_Prep object.
        if (haveMaterial == false)
        {
            if (nMaterialNumbers == nMaterialNumbersAlloc)
            {
                // 
                // We don't have any room left in the materialNumbers array
                // to hold this material, so allocate a bigger array.
                //
                AllocMaterialNumbers(2*nMaterialNumbersAlloc);
            }
            materialNumbers[nMaterialNumbers] = matnos[i];
            nMaterialNumbers++;
        }

    }     // End 'for' over materials for the current domain.

    //
    // Since we have added some new materials to the end of the list, the list
    // is probably not ordered.  Sort it.  Use a slow bubble sort since this
    // is a very short list.
    //
    for (i = 0 ; i < nMaterialNumbers ; i++)
    {
        for (j = i+1 ; j < nMaterialNumbers ; j++)
        {
            if (materialNumbers[i] > materialNumbers[j])
            {
                int temp           = materialNumbers[i];
                materialNumbers[i] = materialNumbers[j];
                materialNumbers[j] = temp;
            }
        }
    }
}


// ****************************************************************************
//  Method: Material_Prep::AllocMaterialNumbers
//
//  Purpose:
//      Gets memory for the material numbers field, in case the number of 
//      materials is larger than intended.
//
//  Programmer: Hank Childs
//  Creation:   January 10, 2000
//
// ****************************************************************************

void
Material_Prep::AllocMaterialNumbers(int size)
{
    //
    // Save a copy of the old fields so that we can copy over their 
    // information.
    //
    int  *oldMaterialNumbers       = materialNumbers;
    int   oldNMaterialNumbersAlloc = nMaterialNumbersAlloc;

    //
    // Create the new fields.
    //
    materialNumbers         = new int[size];
    nMaterialNumbersAlloc   = size;

    //
    // Copy over the old information.  Note that this is ok even if 
    // oldMaterialNumbers == NULL.
    //
    int   i;
    for (i = 0 ; i < oldNMaterialNumbersAlloc ; i++)
    {
        materialNumbers[i] = oldMaterialNumbers[i];
    }
    for (i = oldNMaterialNumbersAlloc ; i < nMaterialNumbersAlloc ; i++)
    {
        materialNumbers[i] = -1;
    }
   
    //
    // Clean up memory.
    //
    if (oldMaterialNumbers != NULL)
    {
        delete [] oldMaterialNumbers;
    }
}


// ****************************************************************************
//  Method: Material_Prep::DetermineSize
//
//  Purpose:
//      Figures out how large the material is in this domain.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2000
//
// ****************************************************************************

void
Material_Prep::DetermineSize(int domain, DBfile *dbfile, char *matName)
{
    void  *rv;

    //
    // Determine the number of zones.
    //
    rv = DBGetComponent(dbfile, matName, "ndims");
    int  *ndims = static_cast< int * >(rv);
    rv = DBGetComponent(dbfile, matName, "dims");
    int  *dims  = static_cast< int * >(rv);
    int   nZones = 1;
    for (int i = 0 ; i < *ndims ; i++)
    {
        nZones *= dims[i];
    }
    free(dims);
    free(ndims);

    //
    // We know that the rank of this Value is 1, so don't bother constructing
    // an array to hold the value of nZones across all dimensions and just
    // send in nZones.
    //
    aliasedMaterialsValue->AddDomain(&nZones, domain);

    rv = DBGetComponent(dbfile, matName, "mixlen");
    int  *mixlen = static_cast< int * >(rv);
    aliasedMixedVFValue->AddDomain(mixlen, domain);
    aliasedMixedNextValue->AddDomain(mixlen, domain);
    aliasedMixedMaterialValue->AddDomain(mixlen, domain);

    free(mixlen);
}


// ****************************************************************************
//  Method: Material_Prep::Consolidate
//
//  Purpose:
//      The initial pass has been completed, so any additional pre-processing
//      can be done here before the 'real' preprocessing begins is earnest.
//
//  Programmer: Hank Childs
//  Creation:   January 26, 2000
//
// ****************************************************************************

void
Material_Prep::Consolidate(void)
{
    aliasedMaterialsValue->Consolidate();
    aliasedMixedVFValue->Consolidate();
    aliasedMixedNextValue->Consolidate();
    aliasedMixedMaterialValue->Consolidate();
}


// ****************************************************************************
//  Method: Material_Prep::WrapUp
//
//  Purpose:
//      All of the information has been read in.  This is the place to do
//      calculations that can only be done when all the information has been
//      collected.
//      Specifically, this routine calls WrapUp for the object it contains and
//      condenses its 2D array domainMaterial_PrepNumbers to a 1D array so that
//      it can be written to a SILO file.
//
//  Programmer: Hank Childs
//  Creation:   January 10, 2000
//
// ****************************************************************************

void
Material_Prep::WrapUp(void)
{
    aliasedMaterialsValue->WrapUp();
    aliasedMixedVFValue->WrapUp();
    aliasedMixedNextValue->WrapUp();
    aliasedMixedMaterialValue->WrapUp();

#ifdef PARALLEL
    //
    // Get the nDomainMaterialNumbers from the other processors.
    //
    int   i;
    int  *domainCopy = new int[nDomains];
    MPI_Allreduce(nDomainMaterialNumbers, domainCopy, nDomains, MPI_INT,
                  MPI_SUM, MPI_COMM_WORLD);
    for (i = 0 ; i < nDomains ; i++)
    {
        nDomainMaterialNumbers[i] = domainCopy[i];
    }
    delete [] domainCopy;
#endif

    //
    // This function handles any NULL arrays in domainMaterialNumbers and 
    // puts in 0's, so we can do a MPI_SUM if we are operating in parallel
    // later.
    //
    Construct1DArray(nDomains, nDomainMaterialNumbers, domainMaterialNumbers,
                     &domainMaterialNumbers1D, &nDomainMaterialNumbers1D);
    
#ifdef PARALLEL
    //
    // Now each processor has a 1D array of the domains it knows about.  Do
    // a MPI_Reduce and put the array on processor 0.
    //
    int   *out = new int[nDomainMaterialNumbers1D];
    MPI_Reduce(domainMaterialNumbers1D, out, nDomainMaterialNumbers1D,
                  MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    for (i = 0 ; i < nDomainMaterialNumbers1D ; i++)
    {
        domainMaterialNumbers1D[i] = out[i];
    }
    delete [] out;
#endif

}


// ****************************************************************************
//  Method: Material_Prep::Write
//
//  Purpose:
//      Writes out a Material_Prep object by calling the method from the base
//      type.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2000
//
// ****************************************************************************

void
Material_Prep::Write(DBfile *dbfile)
{
#ifdef PARALLEL
    extern int my_rank;
    if (my_rank != 0)
    {
        return;
    }
#endif

    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to "
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }

    Material::Write(dbfile);
}


