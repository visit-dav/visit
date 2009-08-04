/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

// NOTE - This code incomplete and requires underlined portions
// to be replaced with code to read values from your file format.

#include <avtMaterial.h>

// STMD version of GetAuxiliaryData.
void *
avtXXXXFileFormat::GetAuxiliaryData(const char *var,
    int domain, const char *type, void *,
    DestructorFunction &df)
{
    void *retval = 0;

    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        int dims[3] = {1,1,1}, ndims = 1;
        // Structured mesh case
        ndims = MESH DIMENSION, 2 OR 3;
        dims[0] = NUMBER OF ZONES IN X DIMENSION;
        dims[1] = NUMBER OF ZONES IN Y DIMENSION;
        dims[2] = NUMBER OF ZONES IN Z DIMENSION, OR 1 IF 2D;

        // Unstructured mesh case
        dims[0] = NUMBER OF ZONES IN THE MESH
        ndims = 1;        

        // Read the number of materials from the file. This
        // must have already been read from the file when
        // PopulateDatabaseMetaData was called.
        int nmats = NUMBER OF MATERIALS;

        // The matnos array contains the list of numbers that
        // are associated with particular materials. For example,
        // matnos[0] is the number that will be associated with
        // the first material and any time it is seen in the
        // matlist array, that number should be taken to mean
        // material 1. The numbers in the matnos array must
        // all be greater than or equal to 1.
        int *matnos = new int[nmats];
        READ nmats INTEGER VALUES INTO THE matnos ARRAY.

        // Read the material names from your file format or
        // make up names for the materials. Use the same
        // approach as when you created material names in
        // the PopulateDatabaseMetaData method.
        char **names = new char *[nmats];
        READ MATERIAL NAMES FROM YOUR FILE FORMAT UNTIL EACH
        ELEMENT OF THE names ARRAY POINTS TO ITS OWN STRING.

        // Read the matlist array, which tells what the material
        // is for each zone in the mesh.
        int nzones = dims[0] * dims[1] * dims[2];
        int *matlist = new int[nzones];
        READ nzones INTEGERS INTO THE matlist array.

        // Optionally create mix_mat, mix_next, mix_zone, mix_vf
        // arrays and read their contents from the file format.

        // Use the information to create an avtMaterial object.
        avtMaterial *mat = new avtMaterial(
            nmats,
            matnos,
            names,
            ndims,
            dims,
            0,
            matlist,
            0, // length of mix arrays
            0, // mix_mat array
            0, // mix_next array
            0, // mix_zone array
            0  // mix_vf array
            );

        // Clean up.
        delete [] matlist;
        delete [] matnos;
        for(int i = 0; i < nmats; ++i)
            delete [] names[i];
        delete [] names;

        // Set the return values.
        retval = (void *)mat;
        df = avtMaterial::Destruct;
    }

    return retval;
}
