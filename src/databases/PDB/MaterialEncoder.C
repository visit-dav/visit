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

#include <MaterialEncoder.h>
#include <avtMaterial.h>

//
// Template function for memory reallocation.
//

template <class T>
T *remake(T *ptr, int oldsize, int size)
{
    T *retval = new T[size];
    T *iptr = retval;
    for(int i = 0; i < oldsize; ++i)
        *iptr++ = ptr[i];
    delete [] ptr;
    return retval;
}

// ****************************************************************************
// Method: MaterialEncoder::MaterialEncoder
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 6 11:33:52 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MaterialEncoder::MaterialEncoder() : matNames()
{
    have_mixed = false;
    mix_zone = NULL;
    mix_mat = NULL;
    mix_vf = NULL;
    mix_next = NULL;
    matlist = NULL;

    // initialize private members.
    _array_size = 0;
    _array_index = 1;
    _array_growth = 5000;
}

// ****************************************************************************
// Method: MaterialEncoder::~MaterialEncoder
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 6 11:34:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MaterialEncoder::~MaterialEncoder()
{
   delete [] matlist;
   if (have_mixed)
   {
       delete [] mix_zone;   
       delete [] mix_mat;    
       delete [] mix_vf;      
       delete [] mix_next;    
   }
}

// ****************************************************************************
// Method: MaterialEncoder::AddMaterial
//
// Purpose: 
//   Adds a material to the list of materials.
//
// Arguments:
//   mat : The name of a material.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 6 11:34:20 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
MaterialEncoder::AddMaterial(const std::string &mat)
{
    matNames.push_back(mat);
}

// ****************************************************************************
// Method: MaterialEncoder::AddClean
//
// Purpose: 
//   Adds a clean zone for the specified zone.
//
// Arguments:
//   zoneId : The zone number that we're calling clean.
//   matNumber : The material number we're putting into the zone.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 6 11:34:44 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
MaterialEncoder::AddClean(int zoneId, int matNumber)
{
    matlist[zoneId] = matNumber;
}

// ****************************************************************************
// Method: MaterialEncoder::AddMixed
//
// Purpose: 
//   Adds a mixed zone for the specified zone.
//
// Arguments:
//   zoneId   : The zone number that we're calling mixed.
//   matNumbers : The material numbers present in the zone.
//   matVf      : The material volume fractions in the zone.
//   nMats      : The number of materials in the zone.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 6 11:37:37 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Dec 7 16:16:19 PST 2004
//   Converted matVf from double to float.
//
// ****************************************************************************

void
MaterialEncoder::AddMixed(int zoneId, const int *matNumbers,
    const float *matVf, int nMats)
{
    int i;

    // Grow the arrays if they will not fit nMats materials.
    Resize(nMats);

    // Record the mixed zone as a negative offset into the mix arrays.
    matlist[zoneId] = -_array_index;

    // Update the mix arrays.
    for(i = 0; i < nMats; ++i)
    {
        int index = _array_index - 1;

        mix_zone[index] = zoneId;
        mix_mat[index]  = matNumbers[i];
        mix_vf[index]   = matVf[i];

        if(i < nMats - 1)
            mix_next[index] = index + 2;
        else
            mix_next[index] = 0;

        ++(_array_index);
    }

    // indicate that we have mixed materials.
    have_mixed = true;
}

// ****************************************************************************
// Method: MaterialEncoder::AllocClean
//
// Purpose: 
//   Allocates clean zones.
//
// Arguments:
//   nZones : The number of clean zones to allocate.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 6 11:38:58 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
MaterialEncoder::AllocClean(int nZones)
{
    matlist = new int[nZones];
}

// ****************************************************************************
// Method: MaterialEncoder::GetMixedSize
//
// Purpose: 
//   Returns the size of the mixed arrays.
//
// Returns:    The size of the mixed arrays.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 6 11:39:25 PDT 2003
//
// Modifications:
//   Eric Brugger, Tue Mar 16 16:33:08 PST 2004
//   Modified it to return the correct size, which is one less than the
//   next index to insert an entry.
//   
// ****************************************************************************

int
MaterialEncoder::GetMixedSize() const
{
    return _array_index - 1;
}

// ****************************************************************************
// Method: MaterialEncoder::CreateMaterial
//
// Purpose: 
//   Creates an avtMaterial from the material information in the object.
//
// Arguments:
//   dims : The dimensions of the mesh associated with the material.
//   ndims : The number of dimensions.
//
// Returns:    An avtMaterial object that contains the material arrays.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 6 11:39:57 PDT 2003
//
// Modifications:
//   Brad Whitlock, Wed Sep 8 14:52:13 PST 2004
//   I added support for material names back in.
//
// ****************************************************************************

avtMaterial *
MaterialEncoder::CreateMaterial(const int *dims, int ndims) const
{
    int *matnos = new int[matNames.size()];
    char **names = new char *[matNames.size()];
    for(int i = 0; i < matNames.size(); ++i)
    {
        matnos[i] = i + 1;
        names[i] = (char *)matNames[i].c_str();
    }

    avtMaterial *retval;
    if(have_mixed)
    {
        retval = new avtMaterial(
            matNames.size(),
            matnos,
            names,
            ndims,
            dims,
            0,
            matlist,
            GetMixedSize(),
            mix_mat,
            mix_next,
            mix_zone,
            mix_vf
            );
    }
    else
    {
        retval = new avtMaterial(
            matNames.size(),
            matnos,
            names,
            ndims,
            dims,
            0,
            matlist,
            0,
            0,
            0,
            0,
            0
            );
    }

    delete [] matnos;
    delete [] names;

    return retval;
}

// ****************************************************************************
// Method: MaterialEncoder::Resize
//
// Purpose: 
//   Resizes the mixed material arrays so they can hold more information.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 6 11:41:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
MaterialEncoder::Resize(int nMats)
{ 
    if(_array_index + nMats >= _array_size)
    {
        int new_size = _array_size + _array_growth;

        if(_array_size == 0)
        {
            // Reallocate arrays in large increments.
            mix_zone = new int[new_size];
            mix_mat  = new int[new_size];
            mix_vf   = new float[new_size];
            mix_next = new int[new_size];
        }
        else
        {
            // Reallocate arrays in large increments.
            mix_zone = remake(mix_zone, _array_size, new_size);
            mix_mat  = remake(mix_mat, _array_size,  new_size);
            mix_vf   = remake(mix_vf, _array_size, new_size);
            mix_next = remake(mix_next, _array_size, new_size);
        }

        _array_size = new_size;
    }
}
