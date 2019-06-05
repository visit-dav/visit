/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                       avtValueImageCompositer.C                           //
// ************************************************************************* //

// Just to be sure, on Windows.
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <cmath>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkImageData.h>
#include <avtParallel.h>
#include <avtValueImageCompositer.h>
#include <ImproperUseException.h>
#include <TimingsManager.h>

typedef struct zfpixel {

    zfpixel() : z(0.0f), value(0.0f) {}

    float z;
    float value;

} ZFPixel_t;

// declare initialize static data members
int avtValueImageCompositer::objectCount = 0;

// ****************************************************************************
// Function:     MergeZFPixelBuffers
//
// Purpose:      User-defined function for MPI_Op_create.  Will be used
//               to perform collective buffer merge operations.  Merges
//               frame and z-buffers.
//
// Programmer:   Mark C. Miller (plagiarized from Katherine Price)
// Date:         February 26, 2003
//
// Modifications:
//
// ****************************************************************************
static float local_bg;

static void
#ifdef PARALLEL
MergeZFPixelBuffers(void *ibuf, void *iobuf, int *count, MPI_Datatype *)
#else
MergeZFPixelBuffers(void *ibuf, void *iobuf, int *count, void *)
#endif
{
    int t1 = visitTimer->StartTimer();
    ZFPixel_t *in_zfpixels    = (ZFPixel_t *) ibuf;
    ZFPixel_t *inout_zfpixels = (ZFPixel_t *) iobuf;

    const int amount = *count;
    for (int i = 0; i < amount; i++)
    {
        if ( in_zfpixels[i].z < inout_zfpixels[i].z )
        {
            inout_zfpixels[i] = in_zfpixels[i];
        }
#if 0
//Q: Does it make sense?
        else if (in_zfpixels[i].z == inout_zfpixels[i].z)
        {
            if (inout_zfpixels[i].value == local_bg)
            {
                // Since 'inout' is background color, take whatever
                // is in 'in' even if it too is background color
                inout_zfpixels[i].value = in_zfpixels[i].value;
            }
//Q: Does it make sense to average the values for a value image?
            else if (in_zfpixels[i].value != local_bg)
            {
                // Neither 'inout' nor 'in' is the background color.
                // So, average them.
                float newv = (in_zfpixels[i].value +
                              inout_zfpixels[i].value) / 2.f;
                inout_zfpixels[i].r = newv;
            }
        }
#endif
    }
    visitTimer->StopTimer(t1, "Time to run compare function");
}

#ifdef PARALLEL

MPI_Op avtValueImageCompositer::mpiOpMergeZFPixelBuffers = MPI_OP_NULL;
MPI_Datatype avtValueImageCompositer::mpiTypeZFPixel;

// ****************************************************************************
// Function:    InitializeMPIStuff 
//
// Purpose:      Build MPI objects necessary to support
//               avtValueImageCompositer class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03 
// ****************************************************************************

void 
avtValueImageCompositer::InitializeMPIStuff(void)
{

#define UCH MPI_UNSIGNED_CHAR
#define FLT MPI_FLOAT
   int                lengths[] = {  1,   1};
   MPI_Aint     displacements[] = {  0,   0};
   MPI_Datatype         types[] = {FLT, FLT};
   ZFPixel_t    onePixel;
#undef UCH
#undef FLT

   // create the MPI data type for ZFPixel
#if MPI_VERSION >= 2
   MPI_Get_address(&onePixel.z, &displacements[0]);
   MPI_Get_address(&onePixel.value, &displacements[1]);
#else
   MPI_Address(&onePixel.z, &displacements[0]);
   MPI_Address(&onePixel.value, &displacements[1]);
#endif

   for (int i = 1; i >= 0; --i)
      displacements[i] -= displacements[0];

   MPI_Type_create_struct(2, lengths, displacements, types,
      &avtValueImageCompositer::mpiTypeZFPixel);

   // check that the datatype has the correct extent
   MPI_Aint ext;
#if MPI_VERSION >= 2
   MPI_Aint lb;
   MPI_Type_get_extent(avtValueImageCompositer::mpiTypeZFPixel, &lb, &ext);
#else
   MPI_Type_extent(avtValueImageCompositer::mpiTypeZFPixel, &ext);
#endif
   if (ext != sizeof(onePixel))
   {
       MPI_Datatype tmp = avtValueImageCompositer::mpiTypeZFPixel;
       MPI_Type_create_resized(tmp, 0, sizeof(ZFPixel_t),
           &avtValueImageCompositer::mpiTypeZFPixel);
       MPI_Type_free(&tmp);
   }

   MPI_Type_commit(&avtValueImageCompositer::mpiTypeZFPixel);

   MPI_Op_create((MPI_User_function *)MergeZFPixelBuffers, 1,
      &avtValueImageCompositer::mpiOpMergeZFPixelBuffers);
}

// ****************************************************************************
// Function:    FinalizeMPIStuff 
//
// Purpose:      Free MPI objects used support avtValueImageCompositer
//               class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03 
// ****************************************************************************
void
avtValueImageCompositer::FinalizeMPIStuff(void)
{
   MPI_Op_free(&avtValueImageCompositer::mpiOpMergeZFPixelBuffers);
   MPI_Type_free(&avtValueImageCompositer::mpiTypeZFPixel);
}

#else

void avtValueImageCompositer::InitializeMPIStuff(void) {;}
void avtValueImageCompositer::FinalizeMPIStuff(void) {;}

#endif

// ****************************************************************************
//  Method: avtValueImageCompositer constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 12, 2003
//
//  Modifications:
//
// ****************************************************************************

avtValueImageCompositer::avtValueImageCompositer()
{

   if (avtValueImageCompositer::objectCount == 0)
      InitializeMPIStuff();
   avtValueImageCompositer::objectCount++;
   bg_value = 0.f;
}


// ****************************************************************************
//  Method: avtValueImageCompositer destructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 18, 2003
//
// ****************************************************************************

avtValueImageCompositer::~avtValueImageCompositer()
{
   avtValueImageCompositer::objectCount--;
   if (avtValueImageCompositer::objectCount == 0)
      FinalizeMPIStuff();
}


void
avtValueImageCompositer::SetBackground(float v)
{
    bg_value = v;
}

// ****************************************************************************
//  Method: Execute
//
//  Purpose: Perform the composite
//
//  Programmer: Mark C. Miller (modified from orig code by Kat Price)
//  Creation:   February 18, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtValueImageCompositer::Execute(void)
{
    int numRows = 0;
    int numCols = 0;
    float *ioz = NULL;
    float *rioz = NULL;
    float *iovalue = NULL;
    float *riovalue = NULL;
    vtkImageData *mergedLocalImage = NULL;
    vtkImageData *mergedGlobalImage = NULL;

    // sanity checks
    if (inputImages.size() == 0)
       EXCEPTION0(ImproperUseException);
    for (size_t i = 0; i < inputImages.size(); i++)
    {
      inputImages[i]->GetImage().GetSize(&numRows, &numCols);
      if (numRows != outRows || numCols != outCols) 
         EXCEPTION0(ImproperUseException);
    }

    int nPixels = outRows * outCols;
    avtImageRepresentation &zeroImageRep = inputImages[0]->GetImage();

    if (inputImages.size() > 1)
    {
       //
       // Merge within a processor
       //
       int t1 = visitTimer->StartTimer();
       mergedLocalImage = avtImageRepresentation::NewValueImage(outCols, outRows);
       visitTimer->StopTimer(t1, "Allocating image");
       iovalue            = (float *) mergedLocalImage->GetScalarPointer(0, 0, 0);
       ioz              = new float [nPixels];
       float        *z0 = zeroImageRep.GetZBuffer();
       const float *value0 = (const float *)zeroImageRep.GetImageVTK()->GetScalarPointer(0,0,0);

       // we memcpy because we can't alter any of the input images
       int t2 = visitTimer->StartTimer();
       memcpy(ioz, z0, nPixels * sizeof(float));
       memcpy(iovalue, value0, nPixels * sizeof(float));
       visitTimer->StopTimer(t2, "Mem copies");

       // do the merges, accumulating results in ioz and iovalue
       int t3 = visitTimer->StartTimer();
       for (size_t i = 1; i < inputImages.size(); i++)
       {
           float *z = NULL;
           z = inputImages[i]->GetImage().GetZBuffer();
           const float *value = (const float *)inputImages[i]->GetImage().GetImageVTK()->GetScalarPointer(0,0,0);
           MergeBuffers(nPixels, false, z, value, ioz, iovalue);
       }
       visitTimer->StopTimer(t3, "merging multiple images");
    }
    else
    {
       mergedLocalImage = NULL;
       ioz    = zeroImageRep.GetZBuffer();
       iovalue  = (float *)zeroImageRep.GetImageVTK()->GetScalarPointer(0,0,0);
    }

    if (mpiRoot >= 0)
    {
       // only root allocates output AVT image (for a non-allreduce)
       if (allReduce || mpiRank == mpiRoot)
       {
          mergedGlobalImage = avtImageRepresentation::NewValueImage(outCols, outRows);
          riovalue = (float *) mergedGlobalImage->GetScalarPointer(0, 0, 0);
          rioz   = new float [nPixels];
       }

       //
       // Merge across processors
       //
       int t4 = visitTimer->StartTimer();
       MergeBuffers(nPixels, true, ioz, iovalue, rioz, riovalue);
       visitTimer->StopTimer(t4, "MergeBuffers");

       if (mergedLocalImage != NULL)
       {
          mergedLocalImage->Delete();
          delete [] ioz;
       }

       if (allReduce || mpiRank == mpiRoot)
       {
          if (shouldOutputZBuffer)
          {
             avtImageRepresentation theOutput(mergedGlobalImage,rioz,true);
             SetOutputImage(theOutput);
          }
          else
          {
             delete [] rioz;
             avtImageRepresentation theOutput(mergedGlobalImage);
             SetOutputImage(theOutput);
          }
          mergedGlobalImage->Delete();
       }
       else
       {
          avtImageRepresentation theOutput(NULL);
          SetOutputImage(theOutput);
       }
    }
    else
    {
       if (mergedLocalImage != NULL)
       {
          if (shouldOutputZBuffer)
          {
             avtImageRepresentation theOutput(mergedLocalImage,ioz,true);
             SetOutputImage(theOutput);
          }
          else
          {
             delete [] ioz;
             avtImageRepresentation theOutput(mergedLocalImage);
             SetOutputImage(theOutput);
          }
          mergedLocalImage->Delete();
       }
       else
       {
          avtImageRepresentation theOutput(zeroImageRep);
          SetOutputImage(theOutput);
       }
    }
}

// ****************************************************************************
// Function:     avtValueImageCompositer::MergeBuffers
//
// Purpose:      Merge images represented by separate z and rgb buffers. 
//               The merge is broken into chunks to help MPI to digest it and
//               to reduce peak memory usage.
//
// Issues:       A combination of several different constraints conspire to
//               create a problem with getting background color information
//               into the MPI reduce operator, MergeZFPixelBuffers.  We use a
//               small bit (well, 3 bytes ;) of static memory to communicate
//               the background color.
//
// Programmer:   Mark C. Miller (plagiarized from Kat Price's MeshTV version)
// Date:         March 3, 2004
//
// Modifications:
//
// ****************************************************************************

void
avtValueImageCompositer::MergeBuffers(int npixels, bool doParallel,
                                           const float *inz,
                                           const float *invalue,
                                           float *ioz,
                                           float *iovalue)
{
   int io;
   int chunk       = std::min(npixels, chunkSize);
   std::vector<ZFPixel_t> inzf(chunk);
   std::vector<ZFPixel_t> iozf(chunk);

   // Communicate bg pixel information to user-defined MergeZFPixelBuffers.
   local_bg = bg_value;

   io = 0;
   while (npixels)
   {
      int len = std::min(npixels, chunk);

      // copy the separate zbuffer and value arrays into a single array of structs
      // Note, in parallel, the iozf array is simply used as a place to put the output
      // In serial, however, it also needs to be populated before the MergeZFBuffers 
      for (int i = 0, j = io; i < len; i++, j++)
      {
          inzf[i].z = inz[j];
          inzf[i].value = invalue[j];
      }

      if (!doParallel)
      {
         for (int i = 0, j = io; i < len; i++, j++)
         {
             iozf[i].z = ioz[j];
             iozf[i].value = iovalue[j];
         }
      }

#ifdef PARALLEL
      if (doParallel)
      {
          int t1 = visitTimer->StartTimer();
          if (allReduce)
          {
              MPI_Allreduce(&inzf.at(0), &iozf.at(0), len,
                        avtValueImageCompositer::mpiTypeZFPixel,
                        avtValueImageCompositer::mpiOpMergeZFPixelBuffers,
                        mpiComm);
          }
          else
          {
              MPI_Reduce(&inzf.at(0), &iozf.at(0), len,
                        avtValueImageCompositer::mpiTypeZFPixel,
                        avtValueImageCompositer::mpiOpMergeZFPixelBuffers,
                        mpiRoot, mpiComm);
          }
          visitTimer->StopTimer(t1, "MPI reduces");
      }
      else
      {
          int t1 = visitTimer->StartTimer();
          MergeZFPixelBuffers(&inzf.at(0), &iozf.at(0), &len, NULL);
          visitTimer->StopTimer(t1, "MergeZFPixelBuffers");
      }
#else
      if (doParallel)
      {
         EXCEPTION0(ImproperUseException);
      }

      int t2 = visitTimer->StartTimer();
      MergeZFPixelBuffers(&inzf.at(0), &iozf.at(0), &len, NULL);
      visitTimer->StopTimer(t2, "MergeZFPixelBuffers");
#endif

      int t3 = visitTimer->StartTimer();
      if (!doParallel || allReduce || mpiRank == mpiRoot)
      {
         for (int i = 0; i < len; i++, io++)
         {
             ioz[io] = iozf[i].z;
             iovalue[io] = iozf[i].value;
         }
      }
      else
      {
          io += len;
      }

      npixels -= len;
      visitTimer->StopTimer(t3, "Array copies");
   }
}
