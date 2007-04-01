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
//
// ****************************************************************************

vtkQtImagePrinter::vtkQtImagePrinter() : print()
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
//   Brad Whitlock, Tue Feb 22 09:34:09 PDT 2005
//   I rewrote the code so it makes a single call to QPainter::drawImage
//   instead of drawing each scanline independently so we don't get horizontal
//   stripes in the output image. This uses more memory but it looks way better
//   in the printed image.
//
// ****************************************************************************

void
vtkQtImagePrinter::WriteFile(ofstream *, vtkImageData *data, int extent[6])
{  
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
    int rowLength = extent[1] - extent[0] + 1;
    int columnHeight = extent[3] - extent[2] + 1;

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
//    qDebug("paint.setViewport1(%d,%d,%d,%d)", vptX*scale, vptY*scale, vptW*scale, vptH*scale);
#else
    paint.setViewport(vptX, vptY, vptW, vptH);
//    qDebug("paint.setViewport2(%d,%d,%d,%d)", vptX, vptY, vptW, vptH);
#endif
    paint.setWindow(0, 0, rowLength, columnHeight);
//    qDebug("paint.setWindow(0,0,%d,%d)", rowLength, columnHeight);

    //
    // Crudely determine the machine endianness.
    //
    unsigned int endianVal = 1;
    unsigned char *endianuptr = (unsigned char *)&endianVal;
    bool bigEndian = (endianuptr[0] != 1);

    //
    // Set up a pointer to the buffer that we'll use to create the image.
    //
    int bpp = data->GetNumberOfScalarComponents();
//    qDebug("bpp = %d", bpp);
    unsigned char *imageData = 0;
    bool ownImageData = false;
    if(bpp < 4)
    {
        imageData = (unsigned char *)(new unsigned int[rowLength * columnHeight]);
        unsigned char *uptr = imageData;
        ownImageData = true;

        // Rearrange memory into the format the QImage wants so we can make
        // a single call to QPrinter::drawImage.
        for(int y = extent[3]; y >= extent[2]; --y)
        {
            unsigned char *ptr = (unsigned char *)data->GetScalarPointer(extent[0], y, 0);
            if(bpp == 3)
            {
                if(bigEndian)
                {
                    for(int i = 0; i < rowLength; ++i, ptr += 3)
                    {
                        *uptr++ = 255;
                        *uptr++ = ptr[0];
                        *uptr++ = ptr[1];
                        *uptr++ = ptr[2];
                    }
                }
                else
                {
                    for(int i = 0; i < rowLength; ++i, ptr += 3)
                    {
                        *uptr++ = ptr[2];
                        *uptr++ = ptr[1];
                        *uptr++ = ptr[0];
                        *uptr++ = 255;
                    }
                }
            }
            else if(bpp == 1)
            {
                for(int i = 0; i < rowLength; ++i)
                {
                    *uptr++ = ptr[i];
                    *uptr++ = ptr[i];
                    *uptr++ = ptr[i];
                    *uptr++ = ptr[i];
                }
            }
            else if(bpp == 2)
            {
                for(int i = 0; i < rowLength; ++i, ptr += 2)
                {
                    *uptr++ = ptr[0];
                    *uptr++ = ptr[0];
                    *uptr++ = ptr[0];
                    *uptr++ = ptr[0];
                }
            }
        }
    }
    else
        imageData = (unsigned char *)data->GetScalarPointer(extent[0], extent[2], 0);

    //
    // Draw the image on the printer surface.
    //
    QImage image(imageData, rowLength, columnHeight, 32, 0, 0, QImage::IgnoreEndian);
    paint.drawImage(QPoint(0, 0), image);

    // Indicate that we're done drawing.
    paint.end();

    if(ownImageData)
        delete [] imageData;
}
