// ************************************************************************* //
//                            avtWholeImageCompositer.C                      //
// ************************************************************************* //
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtParallel.h>
#include <avtWholeImageCompositer.h>
#include <vtkImageData.h>
#include <ImproperUseException.h>

typedef struct zfpixel {
    float         z;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} ZFPixel_t;

// ****************************************************************************
// Function:     MergeZFPixelBuffers
//
// Purpose:      User-defined function for MPI_Op_create.  Will be used
//               to perform collective buffer merge operations.  Merges
//               frame and z-buffers.
//
// Programmer:   Mark C. Miller (plagerized from Katherine Price)
// Date:         26Feb03 
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

    // get the bacground color info out of last entry
    unsigned char local_bg_r = in_zfpixels[*count-1].r; 
    unsigned char local_bg_g = in_zfpixels[*count-1].g; 
    unsigned char local_bg_b = in_zfpixels[*count-1].b; 
 
    for (i = 0; i < *count-1; i++)
    {
        if ( in_zfpixels[i].z < inout_zfpixels[i].z )
        {
            inout_zfpixels[i].z = in_zfpixels[i].z;
            inout_zfpixels[i].r = in_zfpixels[i].r;
            inout_zfpixels[i].g = in_zfpixels[i].g;
            inout_zfpixels[i].b = in_zfpixels[i].b;
        }
        else if (in_zfpixels[i].z == inout_zfpixels[i].z &&
                 inout_zfpixels[i].r == local_bg_r &&
                 inout_zfpixels[i].g == local_bg_g &&
                 inout_zfpixels[i].b == local_bg_b)
        {
            inout_zfpixels[i].r = in_zfpixels[i].r;
            inout_zfpixels[i].g = in_zfpixels[i].g;
            inout_zfpixels[i].b = in_zfpixels[i].b;
        }
    }
 
}

// declare initialize static data members
int avtWholeImageCompositer::objectCount = 0;

#ifdef PARALLEL

MPI_Op avtWholeImageCompositer::mpiOpMergeZFPixelBuffers = MPI_OP_NULL;
// MPI doesn't have any 'unset' constant to assign for a datatype
MPI_Datatype avtWholeImageCompositer::mpiTypeZFPixel;

// ****************************************************************************
// Function:    InitializeMPIStuff 
//
// Purpose:      Build MPI objects necessary to support avtWholeImageCompositer
//               class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03 
// ****************************************************************************

void 
avtWholeImageCompositer::InitializeMPIStuff(void)
{

#define UCH MPI_UNSIGNED_CHAR
#define FLT MPI_FLOAT
   int                lengths[] = {  1,   1,   1,   1,   1};
   MPI_Aint     displacements[] = {  0,   0,   0,   0,   0};
   MPI_Datatype         types[] = {FLT, UCH, UCH, UCH, UCH};
   int                      i, n = sizeof lengths / sizeof lengths[0];
   ZFPixel_t    onePixel;
#undef UCH
#undef FLT

   // create the MPI data type for ZFPixel
   MPI_Address(&onePixel.z, &displacements[0]);
   MPI_Address(&onePixel.r, &displacements[1]);
   MPI_Address(&onePixel.g, &displacements[2]);
   MPI_Address(&onePixel.b, &displacements[3]);
   MPI_Address(&onePixel.a, &displacements[4]);
   for (i = n-1; i >= 0; i--)
      displacements[i] -= displacements[0];
   MPI_Type_struct(n, lengths, displacements, types,
      &avtWholeImageCompositer::mpiTypeZFPixel);
   MPI_Type_commit(&avtWholeImageCompositer::mpiTypeZFPixel);
   MPI_Op_create(MergeZFPixelBuffers, 1,
      &avtWholeImageCompositer::mpiOpMergeZFPixelBuffers);
}

// ****************************************************************************
// Function:    FinalizeMPIStuff 
//
// Purpose:      Free MPI objects used support avtWholeImageCompositer class.
//
// Programmer:   Mark C. Miller
// Date:         01Apr03 
// ****************************************************************************
void
avtWholeImageCompositer::FinalizeMPIStuff(void)
{
   MPI_Op_free(&avtWholeImageCompositer::mpiOpMergeZFPixelBuffers);
   MPI_Type_free(&avtWholeImageCompositer::mpiTypeZFPixel);
}

#else

void avtWholeImageCompositer::InitializeMPIStuff(void) {;}
void avtWholeImageCompositer::FinalizeMPIStuff(void) {;}

#endif

// ****************************************************************************
//  Method: avtWholeImageCompositer constructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 12, 2003
//
// ****************************************************************************

avtWholeImageCompositer::avtWholeImageCompositer()
{

   if (avtWholeImageCompositer::objectCount == 0)
      InitializeMPIStuff();
   avtWholeImageCompositer::objectCount++;
   chunkSize = 1000000;
}


// ****************************************************************************
//  Method: avtWholeImageCompositer destructor
//
//  Programmer: Mark C. Miller 
//  Creation:   February 18, 2003
//
// ****************************************************************************

avtWholeImageCompositer::~avtWholeImageCompositer()
{
   inputImages.clear();
   avtWholeImageCompositer::objectCount--;
   if (avtWholeImageCompositer::objectCount == 0)
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
// ****************************************************************************

void
avtWholeImageCompositer::Execute(void)
{   int i, numRows, numCols;
    float *ioz, *rioz;
    unsigned char *iorgb, *riorgb;
    vtkImageData *mergedLocalImage, *mergedGlobalImage;

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
       mergedLocalImage    = avtImageRepresentation::NewImage(outCols, outRows);
       iorgb               = (unsigned char *) mergedLocalImage->GetScalarPointer(0, 0, 0);
       ioz                 = new float [nPixels];
       float                 *z0 = zeroImageRep.GetZBuffer();
       const unsigned char *rgb0 = zeroImageRep.GetRGBBuffer();

       // we memcpy because we can't alter any of the input images
       memcpy(ioz, z0, nPixels * sizeof(float));
       memcpy(iorgb, rgb0, nPixels * 3 * sizeof(unsigned char));

       // do the merges, accumulating results in ioz and iorgb
       for (i = 1; i < inputImages.size(); i++)
       {
           float                 *z = inputImages[i]->GetImage().GetZBuffer();
           const unsigned char *rgb = inputImages[i]->GetImage().GetRGBBuffer();
           MergeBuffers(this, nPixels, false, z, rgb, ioz, iorgb);
       }
    }
    else
    {
       mergedLocalImage = NULL;
       ioz              = zeroImageRep.GetZBuffer();
       iorgb            = zeroImageRep.GetRGBBuffer();
    }


    if (mpiRoot >= 0)
    {
       // only root allocates output AVT imge
       if (mpiRank == mpiRoot)
       {
          mergedGlobalImage = avtImageRepresentation::NewImage(outCols, outRows);
          riorgb = (unsigned char *) mergedGlobalImage->GetScalarPointer(0, 0, 0);
          rioz   = new float [nPixels];
       }

       //
       // Merge across processors
       //
       MergeBuffers(this, nPixels, true, ioz, iorgb, rioz, riorgb);

       if (mergedLocalImage != NULL)
       {
          mergedLocalImage->Delete();
          delete [] ioz;
       }

       if (mpiRank == mpiRoot)
       {
          if (shouldOutputZBuffer)
          {
             avtImageRepresentation theOutput(mergedGlobalImage,rioz);
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
             avtImageRepresentation theOutput(mergedLocalImage,ioz);
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
          if (shouldOutputZBuffer)
          {
             avtImageRepresentation theOutput(zeroImageRep.GetImageVTK(),zeroImageRep.GetZBuffer());
             SetOutputImage(theOutput);
          }
          else
          {
             avtImageRepresentation theOutput(zeroImageRep.GetImageVTK());
             SetOutputImage(theOutput);
          }
       }
    }
}

// ****************************************************************************
// Function:     MergeBuffers
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
// Programmer:   Mark C. Miller (plagerized from Kat Price's MeshTV version)
// Date:         04Mar03 
// ****************************************************************************
void
MergeBuffers(avtWholeImageCompositer *thisObj, int npixels, bool doParallel,
   const float *inz, const unsigned char *inrgb, float *ioz, unsigned char *iorgb)
{
   // build local var surrogates of object members for this friend function
   int                   chunkSize = thisObj->chunkSize; 
   int                     mpiRank = thisObj->mpiRank;
   int                     mpiRoot = thisObj->mpiRoot;
   unsigned char              bg_r = thisObj->bg_r;
   unsigned char              bg_g = thisObj->bg_g;
   unsigned char              bg_b = thisObj->bg_b;
#ifdef PARALLEL
   MPI_Comm                mpiComm = thisObj->mpiComm;
#endif

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
         MPI_Reduce(inzf, iozf, len+1, avtWholeImageCompositer::mpiTypeZFPixel,
            avtWholeImageCompositer::mpiOpMergeZFPixelBuffers, mpiRoot, mpiComm);
      else
      {  int adjustedLen = len+1;
         MergeZFPixelBuffers(inzf, iozf, &adjustedLen, NULL);
      }
#else
      if (doParallel)
         EXCEPTION0(ImproperUseException);
      {  int adjustedLen = len+1;
         MergeZFPixelBuffers(inzf, iozf, &adjustedLen, NULL);
      }
#endif

      if (!doParallel || mpiRank == mpiRoot)
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
         io += len;

      npixels -= len;
   }

   delete [] inzf;
   delete [] iozf;
}
