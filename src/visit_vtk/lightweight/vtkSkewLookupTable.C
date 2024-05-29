// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <math.h>
#include "vtkSkewLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkSkew.h"

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

// Given a scalar value v, return an rgba color value from lookup table.
// val is first skewed accoring to the skewFactor
const unsigned char *vtkSkewLookupTable::MapValue(double val)
{
  float temp = vtkSkewValue((float)val, (float)this->TableRange[0], (float)this->TableRange[1],
                            this->SkewFactor);
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

  const unsigned char *cptr;
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

