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
//                       avtWholeImageCompositerWithZ.C                      //
// ************************************************************************* //
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtParallel.h>
#include <avtWholeImageCompositerWithZ.h>
#include <vtkImageData.h>
#include <ImproperUseException.h>

typedef struct zfpixel {
    float         z;
    unsigned char r;
    unsigned char g;
    unsigned char b;
} ZFPixel_t;

// declare initialize static data members
int avtWholeImageCompositerWithZ::objectCount = 0;

// ****************************************************************************
// Function:     MergeZFPixelBuffers
//
// Purpose:      User-defined function for MPI_Op_create.  Will be used
//               to perform collective buffer merge operations.  Merges
//               frame and z-buffers.
//
// Programmer:   Mark C. Miller (plagiarized from Katherine Price)
// Date:         26Feb03 
//
// Modifications:
//
//   Hank Childs, Tue Nov 29 16:13:06 PST 2005
//   Add some hand optimizations.
//
// ****************************************************************************

static void
#ifdef PARALLEL
MergeZFPixelBuffers(void *ibuf, void *iobuf, int *count, MPI_Datatype *datatype)
#else
MergeZFPixelBuffers(void *ibuf, void *iobuf, int *count, void *datatype)
#endif
{
    ZFPixel_t *in_zfpixels    = (ZFPixel_t *) ibuf;
    ZFPixel_t *inout_zfpixels = (ZFPixel_t *) iobuf;
    int i;

    // quiet the compiler
    datatype = datatype;

    // get the background color info out of last entry
    const int amount = *count-1;
    unsigned char local_bg_r = in_zfpixels[amount].r; 
    unsigned char local_bg_g = in_zfpixels[amount].g; 
    unsigned char local_bg_b = in_zfpixels[amount].b; 
 
    for (i = 0; i < amount; i++)
    {
        if ( in_zfpixels[i].z < inout_zfpixels[i].z )
        {
            memcpy(inout_zfpixels + i, in_zfpixels + i, sizeof(ZFPixel_t));
        }
        else if (in_zfpixels[i].z == inout_zfpixels[i].z)
        {
            if ((inout_zfpixels[i].r == local_bg_r) &&
                (inout_zfpixels[i].g == local_bg_g) &&
                (inout_zfpixels[i].b == local_bg_b))
            {
                // Since 'inout' is background color, take whatever
                // is in 'in' even if it too is background color
                inout_zfpixels[i].r = in_zfpixels[i].r;
                inout_zfpixels[i].g = in_zfpixels[i].g;
                inout_zfpixels[i].b = in_zfpixels[i].b;
            }
            else if ((in_zfpixels[i].r != local_bg_r) || 
                     (in_zfpixels[i].g != local_bg_g) || 
                     (in_zfpixels[i].b != local_bg_b))
            {
                // Neither 'inout' nor 'in' is the background color.
                // So, average them.
                float newr = (float)    in_zfpixels[i].r +
                             (float) inout_zfpixels[i].r; 
                float newg = (float)    in_zfpixels[i].g +
                             (float) inout_zfpixels[i].g; 
                float newb = (float)    in_zfpixels[i].b +
                             (float) inout_zfpixels[i].b; 
                inout_zfpixels[i].r = (unsigned char) (newr / 2.0); 
                inout_zfpixels[i].g = (unsigned char) (newg / 2.0);
                inout_zfpixels[i].b = (unsigned char) (newb / 2.0); 
            }
        }
    }
}

#ifdef PARALLEL

MPI_Op avtWholeImageCompositerWithZ::mpiOpMergeZFPixelBuffers = MPI_OP_NULL;
MPI_Datatype avtWholeImageCompositerWithZ::mpiTypeZFPixel;

// ****************************************************************************
// Function:    InitializeMPIStuff 
//
// Purpose:      Build MPI objects necessary to support
//               avtWholeImageCompositerWithZ class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03 
// ****************************************************************************

void 
avtWholeImageCompositerWithZ::InitializeMPIStuff(void)
{

#define UCH MPI_UNSIGNED_CHAR
#define FLT MPI_FLOAT
   int                lengths[] = {  1,   1,   1,   1};
   MPI_Aint     displacements[] = {  0,   0,   0,   0};
   MPI_Datatype         types[] = {FLT, UCH, UCH, UCH};
   int                      i, n = sizeof lengths / sizeof lengths[0];
   ZFPixel_t    onePixel;
#undef UCH
#undef FLT

   // create the MPI data type for ZFPixel
   MPI_Address(&onePixel.z, &displacements[0]);
   MPI_Address(&onePixel.r, &displacements[1]);
   MPI_Address(&onePixel.g, &displacements[2]);
   MPI_Address(&onePixel.b, &displacements[3]);
   for (i = n-1; i >= 0; i--)
      displacements[i] -= displacements[0];
   MPI_Type_struct(n, lengths, displacements, types,
      &avtWholeImageCompositerWithZ::mpiTypeZFPixel);
   MPI_Type_commit(&avtWholeImageCompositerWithZ::mpiTypeZFPixel);
   MPI_Op_create(MergeZFPixelBuffers, 1,
      &avtWholeImageCompositerWithZ::mpiOpMergeZFPixelBuffers);
}

// ****************************************************************************
// Function:    FinalizeMPIStuff 
//
// Purpose:      Free MPI objects used support avtWholeImageCompositerWithZ
//               class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03 
// ****************************************************************************
void
avtWholeImageCompositerWithZ::FinalizeMPIStuff(void)
{
   MPI_Op_free(&avtWholeImageCompositerWithZ::mpiOpMergeZFPixelBuffers);
   MPI_Type_free(&avtWholeImageCompositerWithZ::mpiTypeZFPixel);
}

#else

void avtWholeImageCompositerWithZ::InitializeMPIStuff(void) {;}
void avtWholeImageCompositerWithZ::FinalizeMPIStuff(void) {;}

#endif

// ****************************************************************************
//  Method: avtWholeImageCompositerWithZ constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Mar 30 10:58:01 PST 2004
//    Added code to initialize background color
//
// ****************************************************************************

avtWholeImageCompositerWithZ::avtWholeImageCompositerWithZ()
{

   if (avtWholeImageCompositerWithZ::objectCount == 0)
      InitializeMPIStuff();
   avtWholeImageCompositerWithZ::objectCount++;
}


// ****************************************************************************
//  Method: avtWholeImageCompositerWithZ destructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 18, 2003
//
// ****************************************************************************

avtWholeImageCompositerWithZ::~avtWholeImageCompositerWithZ()
{
   avtWholeImageCompositerWithZ::objectCount--;
   if (avtWholeImageCompositerWithZ::objectCount == 0)
      FinalizeMPIStuff();
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
//    Jeremy Meredith, October 20, 2004
//    Allowed for the use of an allreduce instead of a simple reduce.
//    
//    Hank Childs, Mon Feb  6 14:55:39 PST 2006
//    Fix memory leak ['6829].
//
//    Hank Childs, Wed Jan  3 14:18:42 PST 2007
//    Initialize memory.  This prevents purify warning.
//
// ****************************************************************************

void
avtWholeImageCompositerWithZ::Execute(void)
{
    int i, numRows, numCols;
    float *ioz = NULL, *rioz = NULL;
    unsigned char *iorgb = NULL, *riorgb = NULL;
    vtkImageData *mergedLocalImage = NULL, *mergedGlobalImage = NULL;

    // sanity checks
    if (inputImages.size() == 0)
       EXCEPTION0(ImproperUseException);
    for (i = 0; i < inputImages.size(); i++)
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
       mergedLocalImage = avtImageRepresentation::NewImage(outCols, outRows);
       iorgb            = (unsigned char *) mergedLocalImage->GetScalarPointer(0, 0, 0);
       ioz              = new float [nPixels];
       float        *z0 = zeroImageRep.GetZBuffer();
       const unsigned char *rgb0 = zeroImageRep.GetRGBBuffer();

       // we memcpy because we can't alter any of the input images
       memcpy(ioz, z0, nPixels * sizeof(float));
       memcpy(iorgb, rgb0, nPixels * 3 * sizeof(unsigned char));

       // do the merges, accumulating results in ioz and iorgb
       for (i = 1; i < inputImages.size(); i++)
       {
           float *z = NULL;
           z = inputImages[i]->GetImage().GetZBuffer();
           const unsigned char *rgb = inputImages[i]->GetImage().GetRGBBuffer();
           MergeBuffers(nPixels, false, z, rgb, ioz, iorgb);
       }
    }
    else
    {
       mergedLocalImage = NULL;
       ioz    = zeroImageRep.GetZBuffer();
       iorgb  = zeroImageRep.GetRGBBuffer();
    }

    if (mpiRoot >= 0)
    {
       // only root allocates output AVT image (for a non-allreduce)
       if (allReduce || mpiRank == mpiRoot)
       {
          mergedGlobalImage = avtImageRepresentation::NewImage(outCols, outRows);
          riorgb = (unsigned char *) mergedGlobalImage->GetScalarPointer(0, 0, 0);
          rioz   = new float [nPixels];
       }

       //
       // Merge across processors
       //
       MergeBuffers(nPixels, true, ioz, iorgb, rioz, riorgb);

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
// Function:     avtWholeImageCompositer::MergeBuffers
//
// Purpose:      Merge images represented by separate z and rgb buffers. 
//               The merge is broken into chunks to help MPI to digest it and
//               to reduce peak memory usage.
//
// Issues:       A combination of several different constraints conspire to
//               create a problem with getting background color information
//               into the MPI reduce operator, MergeZFPixelBuffers. So, to
//               get around this problem, we pass the background color as
//               the last ZFPixel entry in the array of ZFPixels. We allocate
//               one extr ZFPixel for this purpose.
//   
//
// Programmer:   Mark C. Miller (plagiarized from Kat Price's MeshTV version)
// Date:         04Mar03
//
// Modifications:
//   Jeremy Meredith, October 20, 2004
//   Allowed for the use of an allreduce instead of a simple reduce.
//
// ****************************************************************************
void
avtWholeImageCompositerWithZ::MergeBuffers(int npixels, bool doParallel,
                                           const float *inz,
                                           const unsigned char *inrgb,
                                           float *ioz,
                                           unsigned char *iorgb)
{

   int io;
   int chunk       = npixels < chunkSize ? npixels : chunkSize;
   ZFPixel_t *inzf = new ZFPixel_t [chunk+1];
   ZFPixel_t *iozf = new ZFPixel_t [chunk+1];

   io = 0;
   while (npixels)
   {
      int len = npixels < chunk ? npixels : chunk;

      // copy the separate zbuffer and rgb arrays into a single array of structs
      // Note, in parallel, the iozf array is simply used as a place to put the output
      // In serial, however, it also needs to be populated before the MergeZFBuffers 
      for (int i = 0, j = io; i < len; i++, j++)
      {
          int jj = 3*j;
          inzf[i].z = inz[j];
          inzf[i].r = inrgb[jj+0];
          inzf[i].g = inrgb[jj+1];
          inzf[i].b = inrgb[jj+2];
      }

      if (!doParallel)
      {
         for (int i = 0, j = io; i < len; i++, j++)
         {
             int jj = 3*j;
             iozf[i].z = ioz[j];
             iozf[i].r = iorgb[jj+0];
             iozf[i].g = iorgb[jj+1];
             iozf[i].b = iorgb[jj+2];
         }
      }

      // put the background color info in the last entry in the array
      inzf[len].r = bg_r;
      inzf[len].g = bg_g;
      inzf[len].b = bg_b;

#ifdef PARALLEL
      if (doParallel)
      {
          if (allReduce)
          {
              MPI_Allreduce(inzf, iozf, len+1,
                        avtWholeImageCompositerWithZ::mpiTypeZFPixel,
                        avtWholeImageCompositerWithZ::mpiOpMergeZFPixelBuffers,
                        mpiComm);
          }
          else
          {
              MPI_Reduce(inzf, iozf, len+1,
                        avtWholeImageCompositerWithZ::mpiTypeZFPixel,
                        avtWholeImageCompositerWithZ::mpiOpMergeZFPixelBuffers,
                        mpiRoot, mpiComm);
          }
      }
      else
      {
          int adjustedLen = len+1;
          MergeZFPixelBuffers(inzf, iozf, &adjustedLen, NULL);
      }
#else
      if (doParallel)
      {
         EXCEPTION0(ImproperUseException);
      }

      int adjustedLen = len+1;
      MergeZFPixelBuffers(inzf, iozf, &adjustedLen, NULL);
#endif

      if (!doParallel || allReduce || mpiRank == mpiRoot)
      {
         for (int i = 0; i < len; i++, io++)
         {
             int ii = 3*io;
             ioz[io] = iozf[i].z;
             iorgb[ii+0] = iozf[i].r;
             iorgb[ii+1] = iozf[i].g;
             iorgb[ii+2] = iozf[i].b;
         }
      }
      else
      {
          io += len;
      }

      npixels -= len;
   }

   delete [] inzf;
   delete [] iozf;

}
