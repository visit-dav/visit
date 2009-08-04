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

#include <vtkFloatArray.h>
#include <InvalidVariableException.h>

vtkDataArray *
avtXXXFileFormat::GetVectorVar(const char *varname)
{
    int nvals, ncomps = 3;

    // Read the number of values contained in the array
    // specified by varname.
    nvals = NUMBER OF VALUES IN ARRAY NAMED BY varname;
    ncomps = NUMBER OF VECTOR COMPONENTS IN ARRAY NAMED BY varname;

    // Read component 1 from the file.
    float *comp1 = new float[nvals];
    READ nvals FLOAT VALUES INTO comp1

    // Read component 2 from the file.
    float *comp2 = new float[nvals];
    READ nvals FLOAT VALUES INTO comp2

    // Read component 3 from the file.
    float *comp3 = 0;
    if(ncomps > 2)
    {
        comp3 = new float[nvals];
        READ nvals FLOAT VALUES INTO comp3
    }

    // Allocate the return vtkFloatArray object. Note that
    // you can use vtkFloatArray, vtkDoubleArray,
    // vtkUnsignedCharArray, vtkIntArray, etc.
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfComponents(3);
    arr->SetNumberOfTuples(nvals);
    float *data = (float *)arr->GetVoidPointer(0);
    float *c1 = comp1;
    float *c2 = comp2;
    float *c3 = comp3;
    if(ncomps == 3)
    {
        for(int i = 0; i < nvals; ++i)
        {
            *data++ = *c1++;
            *data++ = *c2++;
            *data++ = *c3++;
        }
    }
    else if(ncomps == 2)
    {
        for(int i = 0; i < nvals; ++i)
        {
            *data++ = *c1++;
            *data++ = *c2++;
            *data++ = 0.;
        }
    }
    else
    {
        delete [] comp1;
        delete [] comp2;
        delete [] comp3;
        arr->Delete();
        EXCEPTION1(InvalidVariableException, varname);
    }

    // Delete temporary arrays.
    delete [] comp1;
    delete [] comp2;
    delete [] comp3;

    return arr;
}
