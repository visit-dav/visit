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

void avtWholeImageCompositer::InitializeMPIStuff(void) {;};
void avtWholeImageCompositer::FinalizeMPIStuff(void) {;};

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
{  int i, numRows, numCols;
   float *ioz, *rioz;
   unsigned char *iorgb, *riorgb;
   bool allocatedInput;

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
       ioz                 = new         float [nPixels];
       iorgb               = new unsigned char [3*nPixels];
       allocatedInput      = true;
       float           *z0 = zeroImageRep.GetZBuffer();
       unsigned char *rgb0 = zeroImageRep.GetRGBBuffer();
       for (i = 0; i < nPixels; i++)
       {
          ioz[i]       = z0[i];
          iorgb[3*i+0] = rgb0[3*i+0];
          iorgb[3*i+1] = rgb0[3*i+1];
          iorgb[3*i+2] = rgb0[3*i+2];
       }
    }
    else
    {
       allocatedInput = false;
       ioz            = zeroImageRep.GetZBuffer();
       iorgb          = zeroImageRep.GetRGBBuffer();
    }

    // merge within a processor
    for (i = 1; i < inputImages.size(); i++)
    {
        float           *z = inputImages[i]->GetImage().GetZBuffer();
        unsigned char *rgb = inputImages[i]->GetImage().GetRGBBuffer();
        MergeBuffers(this, nPixels, false, z, rgb, ioz, iorgb);
    }

    if (mpiRank == mpiRoot)
    {
       rioz   = new         float [nPixels];
       riorgb = new unsigned char [3*nPixels];
    }
    else
    {
       rioz   = NULL;
       riorgb = NULL;
    }

    vtkImageData *image = avtImageRepresentation::NewImage(outCols, outRows);
    unsigned char *data = (unsigned char *)image->GetScalarPointer(0, 0, 0);

    // Now, merge across processors 
    if (mpiRoot >= 0)
    {
       MergeBuffers(this, nPixels, true, ioz, iorgb, rioz, riorgb);

       if (mpiRank == mpiRoot)
       {
          if (!shouldOutputZBuffer)
             delete [] rioz;
          for (i = 0; i < 3*nPixels; i++)
             data[i] = riorgb[i];
          delete [] riorgb;
       }
    }
    else
    {
       for (i = 0; i < 3*nPixels; i++)
          data[i] = iorgb[i];
    }

    if (allocatedInput)
    {
       delete [] ioz;
       delete [] iorgb;
    }

    if (shouldOutputZBuffer)
    {
       avtImageRepresentation theOutput(image,rioz);
       SetOutputImage(theOutput);
    }
    else
    {
       avtImageRepresentation theOutput(image);
       SetOutputImage(theOutput);
    }

    image->Delete();
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
   float *inz, unsigned char *inrgb, float *ioz, unsigned char *iorgb)
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

   if (chunk < npixels)
      cerr << "avtWholeImageCompositer.MergeBuffers is using multiple chunks" << endl;

   io = 0;
   while (npixels)
   {
      int len = npixels < chunk ? npixels : chunk;

      for (int i = 0, j = io; i < len; i++, j++)
      {
         inzf[i].z = inz[j];
         inzf[i].r = inrgb[3*j+0];
         inzf[i].g = inrgb[3*j+1];
         inzf[i].b = inrgb[3*j+2];
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
      MergeZFPixelBuffers(inzf, iozf, &len, NULL);
#endif

      if (!doParallel || mpiRank == mpiRoot)
      {
         for (int i = 0; i < len; i++, io++)
         {
                  ioz[io] = iozf[i].z;
            iorgb[3*io+0] = iozf[i].r;
            iorgb[3*io+1] = iozf[i].g;
            iorgb[3*io+2] = iozf[i].b;
         }
      }
      else
         io += len;

      npixels -= len;
   }

   delete [] inzf;
   delete [] iozf;
}
