/* =======================================================================================
   Copyright 2014-2015 Texas Advanced Computing Center, The University of Texas at Austin
   All rights reserved.

   Licensed under the BSD 3-Clause License, (the "License"); you may not use this file
   except in compliance with the License.
   A copy of the License is included with this software in the file LICENSE.
   If your copy does not contain the License, you may obtain a copy of the License at:

       http://opensource.org/licenses/BSD-3-Clause

   Unless required by applicable law or agreed to in writing, software distributed under
   the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
   KIND, either express or implied.
   See the License for the specific language governing permissions and limitations under
   limitations under the License.

   pvOSPRay is derived from VTK/ParaView Los Alamos National Laboratory Modules (PVLANL)
   Copyright (c) 2007, Los Alamos National Security, LLC
   ======================================================================================= */

#include "ospray/ospray.h"
#include "ospray/common/OSPCommon.h"

#include "vtkOSPRay.h"
#include "vtkOSPRayManager.h"
#include "vtkOSPRayRenderer.h"
#include "vtkOSPRayTexture.h"

#include "vtkHomogeneousTransform.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkTransform.h"




#include <math.h>


vtkStandardNewMacro(vtkOSPRayTexture);

//----------------------------------------------------------------------------
// Initializes an instance, generates a unique index.
vtkOSPRayTexture::vtkOSPRayTexture()
: OSPRayTexture(NULL)
{
  this->OSPRayManager = NULL;
  this->OSPRayTexture = NULL;
}

//----------------------------------------------------------------------------
vtkOSPRayTexture::~vtkOSPRayTexture()
{
  if (this->OSPRayManager)
  {
    this->DeleteOSPRayTexture();

    this->OSPRayManager->Delete();
  }
}

//-----------------------------------------------------------------------------
void vtkOSPRayTexture::DeleteOSPRayTexture()
{
  if (!this->OSPRayTexture)
  {
    return;
  }

  this->OSPRayTexture = NULL;

}

//-----------------------------------------------------------------------------
// Release the graphics resources used by this texture.
void vtkOSPRayTexture::ReleaseGraphicsResources(vtkWindow *win)
{
  this->Superclass::ReleaseGraphicsResources( win );
  if (!this->OSPRayManager)
  {
    return;
  }

  this->DeleteOSPRayTexture();
}

//----------------------------------------------------------------------------
void vtkOSPRayTexture::Load(vtkRenderer *ren, bool nearest)
{
  vtkImageData *input = this->GetInput();

  vtkOSPRayRenderer* renderer =
  vtkOSPRayRenderer::SafeDownCast(ren);
  if (!renderer)
  {
    return;
  }
  if (!this->OSPRayManager)
  {
    this->OSPRayManager = renderer->GetOSPRayManager();
    this->OSPRayManager->Register(this);
  }

  if (this->GetMTime() > this->LoadTime.GetMTime() ||
    input->GetMTime()> this->LoadTime.GetMTime() ||
      (this->GetLookupTable() && this->GetLookupTable()->GetMTime() > this->LoadTime.GetMTime())
      )
      {
        int bytesPerPixel=4;
        int size[3];
        vtkDataArray *scalars;
        unsigned char *dataPtr;
        int xsize, ysize;

    // Get the scalars the user choose to color with.
        scalars = this->GetInputArrayToProcess(0, input);
    // make sure scalars are non null
        if (!scalars)
        {
          vtkErrorMacro(<< "No scalar values found for texture input!");
          return;
        }

    // get some info
        input->GetDimensions(size);

        if (input->GetNumberOfCells() == scalars->GetNumberOfTuples())
        {
      // we are using cell scalars. Adjust image size for cells.
          for (int kk = 0; kk < 3; kk++)
          {
            if (size[kk] > 1)
            {
              size[kk]--;
            }
          }
        }

        bytesPerPixel = scalars->GetNumberOfComponents();

    // make sure using unsigned char data of color scalars type
        if (this->MapColorScalarsThroughLookupTable ||
          scalars->GetDataType() != VTK_UNSIGNED_CHAR)
        {
          dataPtr = this->MapScalarsToColors(scalars);
          bytesPerPixel = 4;
        }
        else
        {
          dataPtr = static_cast<vtkUnsignedCharArray *> (scalars)->GetPointer(0);
        }

    // we only support 2d texture maps right now
    // so one of the three sizes must be 1, but it
    // could be any of them, so lets find it
        if (size[0] == 1)
        {
          xsize = size[1];
          ysize = size[2];
        }
        else
        {
          xsize = size[0];
          if (size[1] == 1)
          {
            ysize = size[2];
          }
          else
          {
            ysize = size[1];
            if (size[2] != 1)
            {
              vtkErrorMacro(<< "3D texture maps currently are not supported!");
              return;
            }
          }
        }
        pixels.resize(xsize*ysize*3);  //TODO: Carson: memory leak
        for (int i=0; i < xsize*ysize; i++)
        {
            unsigned char *color = &dataPtr[(i)*bytesPerPixel];
            pixels[i*3+0] = color[0];
            pixels[i*3+1] = color[1];
            pixels[i*3+2] = color[2];
        }

        OSPDataType type = OSP_VOID_PTR;

        if (bytesPerPixel == 4)
        {
          type = OSP_UCHAR3;
        }
        else
        {
          printf("error! bytesperpixel !=4\n");
          Assert(0);
          type = OSP_UCHAR3;
        }

        this->OSPRayTexture = (osp::Texture2D*)ospNewTexture2D(xsize,
         ysize,
         type,
         &pixels[0],
         nearest ? OSP_TEXTURE_FILTER_NEAREST : 0);

        ospCommit((OSPTexture2D)this->OSPRayTexture);

        this->LoadTime.Modified();
      }
    }

//----------------------------------------------------------------------------
    void vtkOSPRayTexture::PrintSelf(ostream& os, vtkIndent indent)
    {
      this->Superclass::PrintSelf(os,indent);
    }
