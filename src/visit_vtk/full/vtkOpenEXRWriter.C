// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkOpenEXRWriter.h"
#include <vtkFloatArray.h>
#include <vtkDataArray.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkObjectFactory.h>
#include <vtkTemplateAliasMacro.h>

#ifdef _WIN32
// We built OpenEXR as a DLL on Windows.
#define OPENEXR_DLL
#endif

#include <ImfRgba.h>
#include <ImfRgbaFile.h>
#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>

#define DO_REFERENCE_COUNT

vtkStandardNewMacro(vtkOpenEXRWriter);

vtkOpenEXRWriter::vtkOpenEXRWriter() : vtkImageWriter()
{
    zbuffer = NULL;
}

vtkOpenEXRWriter::~vtkOpenEXRWriter() 
{
#ifdef DO_REFERENCE_COUNT
    if(zbuffer != NULL)
    {
        zbuffer->Delete();
        zbuffer = NULL;
    }
#endif
}

//
// Data conversion functions
//

template <typename SrcPrecision>
class MakeHalf
{
public:
    MakeHalf() {}
    ~MakeHalf() {}
    half operator()(SrcPrecision value) const
    {
        return half(value);
    }
};

template <>
half MakeHalf<unsigned char>::operator()(unsigned char value) const
{
    const float inv_255 = 1.f / 255.f;
    return half(float(value) * inv_255);
}

template <typename SrcPrecision>
void
CreateImageChannels1(half *r, const SrcPrecision *base, int width, int height)
{
    MakeHalf<SrcPrecision> toHalf;
    for(int j = 0; j < height; ++j)
    {
        const SrcPrecision *src = base + (height-1-j)*width*1; // flip
        for(int i = 0; i < width; ++i)
        {
            *r++ = toHalf(src[0]);
            src ++;
        }
    }
}

template <typename SrcPrecision>
void
CreateImageChannels2(half *r, half *g, const SrcPrecision *base, int width, int height)
{
    MakeHalf<SrcPrecision> toHalf;
    for(int j = 0; j < height; ++j)
    {
        const SrcPrecision *src = base + (height-1-j)*width*2; // flip
        for(int i = 0; i < width; ++i)
        {
            *r++ = toHalf(src[0]);
            *g++ = toHalf(src[1]);
            src += 2;
        }
    }
}

template <typename SrcPrecision>
void
CreateImageChannels3(half *r, half *g, half *b, const SrcPrecision *base, int width, int height)
{
    MakeHalf<SrcPrecision> toHalf;
    for(int j = 0; j < height; ++j)
    {
        const SrcPrecision *src = base + (height-1-j)*width*3; // flip
        for(int i = 0; i < width; ++i)
        {
            *r++ = toHalf(src[0]);
            *g++ = toHalf(src[1]);
            *b++ = toHalf(src[2]);
            src += 3;
        }
    }
}

template <typename SrcPrecision>
void
CreateImageChannels4(half *r, half *g, half *b, half *a, const SrcPrecision *base, int width, int height)
{
    MakeHalf<SrcPrecision> toHalf;
    for(int j = 0; j < height; ++j)
    {
        const SrcPrecision *src = base + (height-1-j)*width*4; // flip
        for(int i = 0; i < width; ++i)
        {
            *r++ = toHalf(src[0]);
            *g++ = toHalf(src[1]);
            *b++ = toHalf(src[2]);
            *a++ = toHalf(src[3]);
            src += 4;
        }
    }
}

static float *
FlipFloatImage(const float *base, int width, int height)
{
    float *image = new float[width*height];
    float *dest = image;
    for(int j = 0; j < height; ++j)
    {
        const float *src = base + (height-1-j)*width; // flip
        memcpy(dest, src, width * sizeof(float));
        dest += width;
    }
    return image;
}

// Convert n-channel image to 1 channel half.
template <typename SrcPrecision>
void
GetLuminanceEx(half *dest, const SrcPrecision *base, int nComps, int width, int height)
{
    MakeHalf<SrcPrecision> toHalf;
    for(int j = 0; j < height; ++j)
    {
        const SrcPrecision *src = base + (height-1-j)*width*nComps; // flip
        for(int i = 0; i < width; ++i)
        {
            *dest++ = toHalf(*src);
            src += nComps;
        }
    }
}

static half *
GetLuminanceImage(vtkImageData *img, int width, int height)
{
    half *retval = NULL;
    vtkDataArray *lum = img->GetPointData()->GetArray("luminance");
    if(lum != NULL)
    {
        retval = new half[width * height];
        switch(lum->GetDataType())
        {
        vtkTemplateAliasMacro(
            GetLuminanceEx(retval, 
                           static_cast<VTK_TT*>(lum->GetVoidPointer(0)),
                           lum->GetNumberOfComponents(),
                           width, height)
        );
        }
    }
    return retval;
}

static float *
GetValueImage(vtkImageData *img, int width, int height)
{
    float *retval = NULL;
    vtkDataArray *v = img->GetPointData()->GetArray("value");
    if(v != NULL)
    {
        retval = FlipFloatImage((const float *)v->GetVoidPointer(0), width, height);
    }
    return retval;
}

// ****************************************************************************
// Method: WriteOpenEXR
//
// Purpose:
//   This helper function writes different images channels to the OpenEXR file.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 15:24:25 PDT 2017
//
// Modifications:
//
// ****************************************************************************

static void
WriteOpenEXR(const char *filename, half *channels[4], const float *z, 
    const half *lum, const float *value, 
    int nChannels, int width, int height)
{
    const char *channelNames[] = {"R", "G", "B", "A"};

    Imf::Header header (width, height);
    for(int i = 0; i < nChannels; ++i)
        header.channels().insert(channelNames[i], Imf::Channel(Imf::HALF));
    if(z != NULL)
        header.channels().insert("Z", Imf::Channel(Imf::FLOAT));
    if(lum != NULL)
        header.channels().insert("L", Imf::Channel(Imf::HALF));
    if(value != NULL)
        header.channels().insert("value", Imf::Channel(Imf::FLOAT));

    Imf::OutputFile file(filename, header);
    Imf::FrameBuffer frameBuffer;

    for(int i = 0; i < nChannels; ++i)
    {
        frameBuffer.insert(channelNames[i],    // name
            Imf::Slice(Imf::HALF,              // type
                       (char *) channels[i],   // base
                       sizeof(half) * 1,       // xStride
                       sizeof(half) * width)); // yStride
    }
    if(z != NULL)
    {
        frameBuffer.insert ("Z",               // name
            Imf::Slice(Imf::FLOAT,             // type
                      (char *) z,              // base
                       sizeof(float) * 1,      // xStride
                       sizeof(float) * width));// yStride
    }
    if(lum != NULL)
    {
        frameBuffer.insert ("L",               // name
            Imf::Slice(Imf::HALF,              // type
                      (char *) lum,            // base
                       sizeof(half) * 1,       // xStride
                       sizeof(half) * width)); // yStride
    }
    if(value != NULL)
    {
        frameBuffer.insert ("value",           // name
            Imf::Slice(Imf::FLOAT,             // type
                      (char *) value,          // base
                       sizeof(float) * 1,      // xStride
                       sizeof(float) * width));// yStride
    }

    file.setFrameBuffer(frameBuffer);
    file.writePixels(height);

#if 0
    // Debugging. Save the zbuffer to plain text so we can plot it.
    if(z != NULL)
    {
        char zfilename[100];
        sprintf(zfilename, "%s.Z", filename);
        FILE *f = fopen(zfilename, "wt");
        float *zp = z;
        for(int j = 0; j < height; ++j)
        {
            for(int j = 0; j < width-1; ++j)
                fprintf(f, "%f,", *zp++);
            fprintf(f, "%f\n", *zp++);
        }
        fclose(f);
    }
#endif
}

// ****************************************************************************
// Method: vtkOpenEXRWriter::Write
//
// Purpose:
//   Writes the vtkImageData to an OpenEXR file.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 15:24:01 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void vtkOpenEXRWriter::Write()
{
    // Make sure we actually have data.
    if ( !GetInput()->GetPointData()->GetScalars())
    {
        vtkErrorMacro(<< "Could not get data from input.");
        return;
    }

    int dims[3];
    GetInput()->GetDimensions(dims);
    int width = dims[0];
    int height = dims[1];
    int bpp = GetInput()->GetNumberOfScalarComponents();

    // Set up a buffer to contain the channels.
    half *buffer = new half[width * height * bpp];
    half *channels[4];
    for(int i = 0; i < 4; ++i)
        channels[i] = buffer + i*width*height;
    // Flip the z buffer if we have one.
    float *z = NULL;
    if(zbuffer != NULL)
        z = FlipFloatImage((const float *)zbuffer->GetVoidPointer(0), width, height);

    // See if there is luminance data.
    half *lum = GetLuminanceImage(GetInput(), width, height);

    // See if there is value data.
    float *v = GetValueImage(GetInput(), width, height);

    // Write the RGB(A) data to the file.
    if(bpp == 4)
    {
        switch(GetInput()->GetScalarType())
        {
        vtkTemplateAliasMacro(
            CreateImageChannels4(channels[0], channels[1], channels[2], channels[3],
                static_cast<VTK_TT*>(GetInput()->GetScalarPointer(0, 0, 0)), width, height)
        );
        }
        WriteOpenEXR(this->GetFileName(), channels, z, lum, v, bpp, width, height);
    }
    else if(bpp == 3)
    {
        switch(GetInput()->GetScalarType())
        {
        vtkTemplateAliasMacro(
            CreateImageChannels3(channels[0], channels[1], channels[2], 
                static_cast<VTK_TT*>(GetInput()->GetScalarPointer(0, 0, 0)), width, height)
        );
        }
        WriteOpenEXR(this->GetFileName(), channels, z, lum, v, bpp, width, height);
    }
    else if(bpp == 2)
    {
        switch(GetInput()->GetScalarType())
        {
        vtkTemplateAliasMacro(
            CreateImageChannels2(channels[0], channels[1],
                static_cast<VTK_TT*>(GetInput()->GetScalarPointer(0, 0, 0)), width, height)
        );
        }
        WriteOpenEXR(this->GetFileName(), channels, z, lum, v, bpp, width, height);
    }
    else if(bpp == 1)
    {
        switch(GetInput()->GetScalarType())
        {
        vtkTemplateAliasMacro(
            CreateImageChannels1(channels[0],
                static_cast<VTK_TT*>(GetInput()->GetScalarPointer(0, 0, 0)), width, height)
        );
        }
        WriteOpenEXR(this->GetFileName(), channels, z, lum, v, bpp, width, height);
    }

    // Clean up
    delete [] buffer;
    delete [] z;
    delete [] lum;
    delete [] v;
}

// ****************************************************************************
// Method: vtkOpenEXRWriter::SetZBuffer
//
// Purpose:
//   Lets the caller associate a vtkFloatArray zbuffer with the writer.
//
// Arguments:
//   arr : The new zbuffer.
//
// Returns:    
//
// Note:       We have this because avtImageRepresentation keeps the image data
//             and the z buffer separate.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 26 15:25:16 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void
vtkOpenEXRWriter::SetZBuffer(vtkFloatArray *arr)
{
#ifdef DO_REFERENCE_COUNT
    if(zbuffer != NULL)
    {
        zbuffer->Delete();
        zbuffer = NULL;
    }

    if(arr != NULL)
    {
        zbuffer = arr;
        zbuffer->Register(NULL);
    }
#else
    zbuffer = arr;
#endif
}

vtkFloatArray *
vtkOpenEXRWriter::GetZBuffer()
{
    return zbuffer;
}

