#include "vtkQtImagePrinter.h"
#include "vtkObjectFactory.h"
#include <qpainter.h>
#include <qimage.h>
#include <qpaintdevicemetrics.h>

#include <vtkImageData.h>
#include <vtkPointData.h>

// ****************************************************************************
// Method: vtkQtImagePrinter::New
//
// Purpose: 
//   Creates a new instance of a vtkQtImagePrinter object.
//
// Returns:    A pointer to a new instance of a vtkQtImagePrinter object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 21 15:57:48 PST 2002
//
// Modifications:
//   
// ****************************************************************************

vtkQtImagePrinter *
vtkQtImagePrinter::New()
{
    // First try to create the object from the vtkObjectFactory
    vtkObject* ret = vtkObjectFactory::CreateInstance("vtkQtImagePrinter");
    if(ret)
    {
        return (vtkQtImagePrinter*)ret;
    }

    // If the factory was unable to create the object, then create it here.
    return new vtkQtImagePrinter;
}

// ****************************************************************************
// Method: vtkQtImagePrinter::vtkQtImagePrinter
//
// Purpose: 
//   Constructor for the vtkQtImagePrinter class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 21 15:56:22 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 16 11:50:02 PDT 2005
//   Made it use higher resolution for the printed image.
//
// ****************************************************************************

vtkQtImagePrinter::vtkQtImagePrinter() : print(QPrinter::HighResolution)
{
    this->FileLowerLeft = 1;
}

// ****************************************************************************
// Method: vtkQtImagePrinter::WriteFile
//
// Purpose: 
//   Extracts the pixels from the image data and sends them to a Qt printer
//   object where they go to a file or a network printer.
//
// Arguments:
//   data   : The image data object.
//   extent : The size of the image.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 21 15:55:05 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
vtkQtImagePrinter::WriteFile(ofstream *, vtkImageData *data, int extent[6])
{
    int rowLength, columnHeight, i;
    unsigned char *ptr;
    int bpp;
    unsigned long count = 0;
    unsigned long target;
    float progress = this->Progress;
    float area;
    int *wExtent;
  
    // Get the number of bits per pixel.
    bpp = data->GetNumberOfScalarComponents();
  
    // Make sure we actually have data.
    if(!data->GetPointData()->GetScalars())
    {
        vtkErrorMacro(<< "Could not get data from input.");
        return;
    }

    // take into consideration the scalar type
    if(data->GetScalarType() != VTK_UNSIGNED_CHAR)
    {
        vtkErrorMacro("vtkQtImagePrinter only accepts unsigned char scalars!");
        return; 
    }

    // Try and start drawing on the printer.
    QPainter paint;
    if(!paint.begin(&print))
    {
        vtkErrorMacro("vtkQtImagePrinter cannot print!");
        return;
    }

    // Get the size of the printer. Figure out a suitable viewport and
    // window to use for the painter.
    QPaintDeviceMetrics metrics(&print);
    // Find the image dimensions
    rowLength = extent[1] - extent[0] + 1;
    columnHeight = extent[3] - extent[2] + 1;

    //
    // Create a temp scanline buffer that we'll use to create images.
    //
    unsigned char *imageData = 0;
    unsigned char *uptr = 0;
    if(bpp < 4)
    {
        imageData = new unsigned char[rowLength * 4];
        for(i = 0; i < rowLength * 4; ++i)
            imageData[i] = 0;
    }

    //
    // Figure out a viewport that will center the image on the printer page.
    //
    int vptW = rowLength;
    int vptH = columnHeight;
    bool squeezeX = vptW > metrics.width();
    bool squeezeY = vptH > metrics.height();
    float aspect = float(vptH) / float(vptW);
    do
    {
        if(squeezeX && squeezeY)
        {
            if(aspect > 1.)
            {
                // Taller than wide
                vptH = metrics.height();
                vptW = int(float(vptH) / aspect);
                squeezeY = false;
            }
            else
            {
                // Wider than tall
                vptW = metrics.width();
                vptH = int(float(vptW) * aspect);
                squeezeX = false;
            }
        }

        if(squeezeX)
        {
            // X dimension larger, Y smaller
            vptW = metrics.width();
            vptH = int(float(metrics.width()) * aspect);
        }
        else if(squeezeY)
        {
            // Y dimension larger, X smaller.
            vptH = metrics.height();
            vptW = int(float(metrics.height()) / aspect);
        }

        // See if we need to go through the loop again.
        squeezeX = vptW > metrics.width();
        squeezeY = vptH > metrics.height();
    } while(squeezeX || squeezeY);

    //
    // Set the viewport and the window.
    //
    int vptX = (metrics.width() - vptW) / 2;
    int vptY = (metrics.height() - vptH) / 2;
#if defined(_WIN32)
    // At this point, I'm not sure whether this is because of Windows
    // or because of printing support in Qt 3.0.2. Just make the image's
    // viewport larger for now.
    float scale = 6.3333f;
    paint.setViewport(vptX*scale, vptY*scale, vptW*scale, vptH*scale);
#else
    paint.setViewport(vptX, vptY, vptW, vptH);
#endif
    paint.setWindow(0, 0, rowLength, columnHeight);

    //
    // Initialize some variables.
    //
    wExtent = this->GetInput()->GetWholeExtent();
    area = ((extent[5] - extent[4] + 1)*(extent[3] - extent[2] + 1)*
            (extent[1] - extent[0] + 1)) / 
           ((wExtent[5] -wExtent[4] + 1)*(wExtent[3] -wExtent[2] + 1)*
            (wExtent[1] -wExtent[0] + 1));
    
    target = (unsigned long)((extent[5]-extent[4]+1)*
                             (extent[3]-extent[2]+1)/(50.0*area));
    target++;

    //
    // Crudely determine the machine endianness.
    //
    unsigned int endianVal = 1;
    unsigned char *endianuptr = (unsigned char *)&endianVal;
    bool bigEndian = (endianuptr[0] != 1);

    //
    // Draw the scanlines on the printer surface.
    //
    for(int y = extent[3]; y >= extent[2]; --y)
    {
        // Output progress
        if(!(count%target))
        {
            this->UpdateProgress(progress + count/(50.0*target));
        }
        count++;

        // Draw the scanline
        ptr = (unsigned char *)data->GetScalarPointer(extent[0], y, 0);
        if(bpp == 1)
        {
            uptr = imageData;
            for(i = 0; i < rowLength; ++i)
            {
                *uptr++ = ptr[i];
                *uptr++ = ptr[i];
                *uptr++ = ptr[i];
                *uptr++ = ptr[i];
            }
            ptr = imageData;
        }
        else if(bpp == 2)
        {
            uptr = imageData;
            for(i = 0; i < rowLength; ++i, ptr += 2)
            {
                *uptr++ = ptr[0];
                *uptr++ = ptr[0];
                *uptr++ = ptr[0];
                *uptr++ = ptr[0];
            }
            ptr = imageData;
        }
        else if(bpp == 3)
        {
            uptr = imageData;
            if(bigEndian)
            {
                for(i = 0; i < rowLength; ++i, ptr += 3)
                {
                    *uptr++ = 255;
                    *uptr++ = ptr[0];
                    *uptr++ = ptr[1];
                    *uptr++ = ptr[2];
                }
            }
            else
            {
                for(i = 0; i < rowLength; ++i, ptr += 3)
                {
                    *uptr++ = ptr[2];
                    *uptr++ = ptr[1];
                    *uptr++ = ptr[0];
                    *uptr++ = 255;
                }
            }
            ptr = imageData;
        }

        QImage image((unsigned char *)ptr, rowLength, 1, 32, 0, 0, QImage::IgnoreEndian);
        paint.drawImage(QPoint(extent[0], extent[3] - y), image);
    }

    // Indicate that we're done drawing.
    paint.end();

    delete [] imageData;
}
