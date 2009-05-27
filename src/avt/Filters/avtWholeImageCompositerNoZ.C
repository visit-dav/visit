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
//                         avtWholeImageCompositerNoZ.C                      //
// ************************************************************************* //
#include <cmath>
#include <sstream>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtParallel.h>
#include <avtWholeImageCompositerNoZ.h>
#include <vtkImageData.h>
#include <ImproperUseException.h>

typedef struct zfpixel {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} ZFPixel_t;


// place holders for global data used in MergeZFPixelBuffers 
static unsigned char local_bg_r;
static unsigned char local_bg_g;
static unsigned char local_bg_b;
 

// ****************************************************************************
// Function:     MergeZFPixelBuffers
//
// Purpose:      User-defined function for MPI_Op_create.  Will be used
//               to perform collective buffer merge operations.  Merges
//               frame and z-buffers.
//
// Programmer:   Mark C. Miller (plagerized from Katherine Price)
// Date:         26Feb03 
//
// Modifications:
//
//   Tom Fogal, Tue May 26 16:18:14 MDT 2009
//   Don't name last (unused) argument.
//
// ****************************************************************************

static void
#ifdef PARALLEL
MergeZFPixelBuffers(void *ibuf, void *iobuf, int *count, MPI_Datatype *)
#else
MergeZFPixelBuffers(void *ibuf, void *iobuf, int *count, void *)
#endif
{
    ZFPixel_t *in_zfpixels    = (ZFPixel_t *) ibuf;
    ZFPixel_t *inout_zfpixels = (ZFPixel_t *) iobuf;
    int i;

    for (i = 0; i < *count; i++)
    {
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

// declare initialize static data members
int avtWholeImageCompositerNoZ::objectCount = 0;

#ifdef PARALLEL

MPI_Op avtWholeImageCompositerNoZ::mpiOpMergeZFPixelBuffers = MPI_OP_NULL;
MPI_Datatype avtWholeImageCompositerNoZ::mpiTypeZFPixel;

// ****************************************************************************
// Function:    InitializeMPIStuff 
//
// Purpose:      Build MPI objects necessary to support avtWholeImageCompositerNoZ
//               class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03 
// ****************************************************************************

void 
avtWholeImageCompositerNoZ::InitializeMPIStuff(void)
{
   MPI_Type_contiguous(3, MPI_UNSIGNED_CHAR, &avtWholeImageCompositerNoZ::mpiTypeZFPixel);
   MPI_Type_commit(&avtWholeImageCompositerNoZ::mpiTypeZFPixel);
   MPI_Op_create(MergeZFPixelBuffers, 1,
      &avtWholeImageCompositerNoZ::mpiOpMergeZFPixelBuffers);
}

// ****************************************************************************
// Function:    FinalizeMPIStuff 
//
// Purpose:      Free MPI objects used support avtWholeImageCompositerNoZ class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03 
// ****************************************************************************
void
avtWholeImageCompositerNoZ::FinalizeMPIStuff(void)
{
   MPI_Op_free(&avtWholeImageCompositerNoZ::mpiOpMergeZFPixelBuffers);
   MPI_Type_free(&avtWholeImageCompositerNoZ::mpiTypeZFPixel);
}

#else

void avtWholeImageCompositerNoZ::InitializeMPIStuff(void) {;}
void avtWholeImageCompositerNoZ::FinalizeMPIStuff(void) {;}

#endif

// ****************************************************************************
//  Method: avtWholeImageCompositerNoZ constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 12, 2003
//
//  Modifications:
//    Mark C. Miller, Tue Mar 30 10:58:01 PST 2004
//    Added code to initialize background color
//
// ****************************************************************************

avtWholeImageCompositerNoZ::avtWholeImageCompositerNoZ()
{

   if (avtWholeImageCompositerNoZ::objectCount == 0)
      InitializeMPIStuff();
   avtWholeImageCompositerNoZ::objectCount++;
}


// ****************************************************************************
//  Method: avtWholeImageCompositerNoZ destructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 18, 2003
//
// ****************************************************************************

avtWholeImageCompositerNoZ::~avtWholeImageCompositerNoZ()
{
   avtWholeImageCompositerNoZ::objectCount--;
   if (avtWholeImageCompositerNoZ::objectCount == 0)
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
//    Hank Childs, Wed Jan  3 16:50:14 PST 2007
//    Initialize arrays that don't get initialized if their rank is not 0.
//    (This removes some purify warnings ... no crashes are fixed.)
//
//    Tom Fogal, Tue May 26 16:27:45 MDT 2009
//    Use `empty' where possible.  Create more verbose errors.
//
// ****************************************************************************

void
avtWholeImageCompositerNoZ::Execute(void)
{
    int i, numRows, numCols;
    unsigned char *iorgb = NULL, *riorgb = NULL;
    vtkImageData *mergedLocalImage = NULL, *mergedGlobalImage = NULL;

    // sanity checks
    if (inputImages.empty())
    {
       EXCEPTION1(ImproperUseException, "No images to composite!");
    }
    for (i = 0; i < inputImages.size(); i++)
    {
        inputImages[i]->GetImage().GetSize(&numRows, &numCols);
        if (numRows != outRows || numCols != outCols)
        {
            std::ostringstream badsize;
            badsize << "Input image " << i << " has dimensions "
                    << numRows << "x" << numCols << ", which does not match "
                    << "output image dimensions ("
                    << outRows << "x" << outCols << ")!" << std::endl;
            EXCEPTION1(ImproperUseException, badsize.str().c_str());
        }
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
       const unsigned char *rgb0 = zeroImageRep.GetRGBBuffer();

       // we memcpy because we can't alter any of the input images
       memcpy(iorgb, rgb0, nPixels * 3 * sizeof(unsigned char));

       // do the merges, accumulating results in ioz and iorgb
       for (i = 1; i < inputImages.size(); i++)
       {
           const unsigned char *rgb = inputImages[i]->GetImage().GetRGBBuffer();
           MergeBuffers(nPixels, false, rgb, iorgb);
       }
    }
    else
    {
       mergedLocalImage = NULL;
       iorgb  = zeroImageRep.GetRGBBuffer();
    }


    if (mpiRoot >= 0)
    {
       // only root allocates output AVT image (for a non-allreduce)
       if (allReduce || mpiRank == mpiRoot)
       {
          mergedGlobalImage = avtImageRepresentation::NewImage(outCols, outRows);
          riorgb = (unsigned char *) mergedGlobalImage->GetScalarPointer(0, 0, 0);
       }

       //
       // Merge across processors
       //
       MergeBuffers(nPixels, true, iorgb, riorgb);

       if (mergedLocalImage != NULL)
       {
          mergedLocalImage->Delete();
       }

       if (allReduce || mpiRank == mpiRoot)
       {
          {
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
          {
             avtImageRepresentation theOutput(mergedLocalImage);
             SetOutputImage(theOutput);
          }
          mergedLocalImage->Delete();
       }
       else
       {
          {
             avtImageRepresentation theOutput(zeroImageRep.GetImageVTK());
             SetOutputImage(theOutput);
          }
       }
    }
}

// ****************************************************************************
// Function:     avtWholeImageCompositerNoZ::MergeBuffers
//
// Purpose:      Merge images represented by separate z and rgb buffers. 
//               The merge is broken into chunks to help MPI to digest it and
//               to reduce peak memory usage.
//
// Issues:       A combination of several different constraints conspire to
//               create a problem with getting background color information
//               into the MPI reduce operator, MergeZFPixelBuffers. So, to
//               get around this problem, we pass the background color
//               using static memory.
//
// Programmer:   Mark C. Miller (plagiarized from Kat Price's MeshTV version)
// Date:         04Mar03 
//
// Modifications:
//    Jeremy Meredith, October 20, 2004
//    Allowed for the use of an allreduce instead of a simple reduce.
//
//    Tom Fogal, Wed May 27 14:32:08 MDT 2009
//    Fixed comment.
//
// ****************************************************************************
void
avtWholeImageCompositerNoZ::MergeBuffers(int npixels, bool doParallel,
                                         const unsigned char *inrgb,
                                         unsigned char *iorgb)
{
   int io;
   int chunk = npixels < chunkSize ? npixels : chunkSize;

   //
   // Setup global data used by MPI merge operator
   //
   local_bg_r = bg_r;
   local_bg_g = bg_g;
   local_bg_b = bg_b;

   io = 0;
   while (npixels)
   {
      int len = npixels < chunk ? npixels : chunk;

#ifdef PARALLEL
      if (doParallel)
      {
          if (allReduce)
          {
              MPI_Allreduce((void*)&inrgb[3*io], &iorgb[3*io], len,
                          avtWholeImageCompositerNoZ::mpiTypeZFPixel,
                          avtWholeImageCompositerNoZ::mpiOpMergeZFPixelBuffers,
                          mpiComm);
          }
          else
          {
              MPI_Reduce((void*)&inrgb[3*io], &iorgb[3*io], len,
                         avtWholeImageCompositerNoZ::mpiTypeZFPixel,
                         avtWholeImageCompositerNoZ::mpiOpMergeZFPixelBuffers,
                         mpiRoot, mpiComm);
          }
      }
      else
         MergeZFPixelBuffers((void*)&inrgb[3*io], &iorgb[3*io], &len, NULL);
#else
      if (doParallel)
         EXCEPTION0(ImproperUseException);
      MergeZFPixelBuffers((void*)&inrgb[3*io], &iorgb[3*io], &len, NULL);
#endif

      io += len;

      npixels -= len;
   }
}
