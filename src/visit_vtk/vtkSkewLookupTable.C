/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSkewLookupTable.C,v $
  Language:  C++
  Date:      $Date: 2000/12/10 20:08:12 $
  Version:   $Revision: 1.62 $


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include <math.h>
#include "vtkSkewLookupTable.h"
#include "vtkObjectFactory.h"



// *************************************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 15:14:29 PST 2002  
//   Replace 'New' method with Macro to match VTK 4.0 API. 
// *************************************************************************

vtkStandardNewMacro(vtkSkewLookupTable);


// Construct with range=(0,1); 
vtkSkewLookupTable::vtkSkewLookupTable(int sze, int ext):
 vtkLookupTable(sze, ext)
{
   this->SkewFactor = 1.;
}


// Takes the passed value and skews it according to passed min  & max
// and the SkewFactor. 

float vtkSkewLookupTable::SkewTheValue(float val, float min, float max)
{
    if (this->SkewFactor <= 0 || this->SkewFactor == 1.) 
        return val;

    float range = max - min; 
    float rangeInverse = 1. / range;
    float logSkew = log(this->SkewFactor);
    float k = range / (this->SkewFactor -1.);
    float v2 = (val - min) * rangeInverse;
    float temp =   k * (exp(v2 * logSkew) -1.) + min;
    return temp;
}


// Given a scalar value v, return an rgba color value from lookup table.
// val is first skewed accoring to the skewFactor
unsigned char *vtkSkewLookupTable::MapValue(float val)
{
  float temp = SkewTheValue(val, this->TableRange[0], this->TableRange[1]);
  return this->Superclass::MapValue(temp);
}

// *************************************************************************
// Modifications:
//   Kathleen Bonnell, Fri Feb 15 12:50:15 PST 2002
//   Changed implemenation to match changes in vtkLookupTable, especially
//   as regards alpha-blending.  Allowed for all output Formats.
// *************************************************************************
// accelerate the mapping by copying the data in 32-bit chunks instead
// of 8-bit chunks
template<class T>
static void vtkSkewLookupTableMapData(vtkSkewLookupTable *self, 
                                  T *input, unsigned char *output, 
                                  int length, int inIncr, int outFormat)
{
  int i = length;
  unsigned char *cptr;
  float alpha;

  if ( (alpha = self->GetAlpha()) >= 1.0) // no blending reuqired
    {
    if (outFormat == VTK_RGBA)
      {
      while (--i >= 0)
        {
        cptr = self->MapValue((float)(*input));
        *output++ = *cptr++;
        *output++ = *cptr++;
        *output++ = *cptr++;
        *output++ = *cptr++;
        input += inIncr;
        }
      }
    else if (outFormat == VTK_RGB)
      {
      while (--i >= 0)
        {
        cptr = self->MapValue((float)(*input));
        *output++ = *cptr++;
        *output++ = *cptr++;
        *output++ = *cptr++;
        input += inIncr;
        }
      }
    else if (outFormat == VTK_LUMINANCE_ALPHA)
      {
      while (--i >= 0)
        {
        cptr = self->MapValue((float)(*input));
        *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 +
                                                 cptr[2]*0.11 + 0.5);
        *output++ = cptr[3];
        input += inIncr;
        }
      } 
    else // outFormat == VTK_LUMINANCE
      {
      while (--i >= 0)
        {
        cptr = self->MapValue((float)(*input));
        *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 +
                                                 cptr[2]*0.11 + 0.5);
        input += inIncr;
        }
      }
    }  // no blending
  else // blending required
   {
    if (outFormat == VTK_RGBA)
      {
      while (--i >= 0)
        {
        cptr = self->MapValue((float)(*input));
        *output++ = *cptr++;
        *output++ = *cptr++;
        *output++ = *cptr++;
        *output++ = static_cast<unsigned char> ((*cptr)*alpha); cptr++;
        input += inIncr;
        }
      }
    else if (outFormat == VTK_RGB)
      {
      while (--i >= 0)
        {
        cptr = self->MapValue((float)(*input));
        *output++ = *cptr++;
        *output++ = *cptr++;
        *output++ = *cptr++;
        input += inIncr;
        }
      }
    else if (outFormat == VTK_LUMINANCE_ALPHA)
      {
      while (--i >= 0)
        {
        cptr = self->MapValue((float)(*input));
        *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 +
                                                 cptr[2]*0.11 + 0.5);
        *output++ = static_cast<unsigned char>(cptr[3]*alpha);
        input += inIncr;
        }
      } 
    else // outFormat == VTK_LUMINANCE
      {
      while (--i >= 0)
        {
        cptr = self->MapValue((float)(*input));
        *output++ = static_cast<unsigned char>(cptr[0]*0.30 + cptr[1]*0.59 +
                                                 cptr[2]*0.11 + 0.5);
        input += inIncr;
        }
      }
    }  // no blending
}

void 
vtkSkewLookupTable::MapScalarsThroughTable2(
             void *input, unsigned char *output, int inputDataType, 
             int numberOfValues, int inputIncrement, int outputFormat)
{
  switch (inputDataType)
    {
    case VTK_CHAR:
      vtkSkewLookupTableMapData(this, (char *)input, output,
                        numberOfValues, inputIncrement, outputFormat);
      break;
      
    case VTK_UNSIGNED_CHAR:
      vtkSkewLookupTableMapData(this, (unsigned char *)input, output,
                        numberOfValues, inputIncrement, outputFormat);
      break;
      
    case VTK_SHORT:
      vtkSkewLookupTableMapData(this, (short *)input, output,
                        numberOfValues, inputIncrement, outputFormat);
    break;
      
    case VTK_UNSIGNED_SHORT:
      vtkSkewLookupTableMapData(this, (unsigned short *)input, output,
                        numberOfValues, inputIncrement, outputFormat);
      break;
      
    case VTK_INT:
      vtkSkewLookupTableMapData(this, (int *)input, output,
                        numberOfValues, inputIncrement, outputFormat);
      break;
      
    case VTK_UNSIGNED_INT:
      vtkSkewLookupTableMapData(this, (unsigned int *)input, output,
                        numberOfValues, inputIncrement, outputFormat);
      break;
      
    case VTK_LONG:
      vtkSkewLookupTableMapData(this, (long *)input, output,
                        numberOfValues, inputIncrement, outputFormat);
      break;
      
    case VTK_UNSIGNED_LONG:
      vtkSkewLookupTableMapData(this, (unsigned long *)input, output,
                         numberOfValues, inputIncrement, outputFormat);
      break;
      
    case VTK_FLOAT:
      vtkSkewLookupTableMapData(this, (float *)input, output,
                        numberOfValues, inputIncrement, outputFormat);
      break;
      
    case VTK_DOUBLE:
      vtkSkewLookupTableMapData(this, (double *)input, output,
                        numberOfValues, inputIncrement, outputFormat);
      break;
      
    default:
      vtkErrorMacro(<< "MapImageThroughTable: Unknown input ScalarType");
      return;
    }
} 
 
void vtkSkewLookupTable::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Build Time:     " <<this->BuildTime.GetMTime() << "\n";
  os << indent << "SkewFactor:     " << this->SkewFactor << "\n";
}

