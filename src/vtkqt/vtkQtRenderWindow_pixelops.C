//----------------------------------------------------------------------
// the following lines are similar to vtkOpenGLRenderWindow:
/*----------------------------------------------------------------------
  Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

  * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
  of any contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
  misrepresented as being the original software.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  ----------------------------------------------------------------------*/
unsigned char *vtkQtRenderWindow::GetPixelData(int x1, int y1, 
                                               int x2, int y2, 
                                               int front) {
    int     y_low, y_hi;
    int     x_low, x_hi;
    unsigned char   *data = NULL;

    if (y1 < y2) {
        y_low = y1; 
        y_hi  = y2;
    }
    else {
        y_low = y2; 
        y_hi  = y1;
    }

    if (x1 < x2) {
        x_low = x1; 
        x_hi  = x2;
    }
    else {
        x_low = x2; 
        x_hi  = x1;
    }

    data = new unsigned char[(x_hi - x_low + 1)*(y_hi - y_low + 1)*3];

#ifdef sparc
    // We need to read the image data one row at a time and convert it
    // from RGBA to RGB to get around a bug in Sun OpenGL 1.1
    long    xloop, yloop;
    unsigned char *buffer;
    unsigned char *p_data = NULL;
  
    buffer = new unsigned char [4*(x_hi - x_low + 1)];
    p_data = data;
    for (yloop = y_low; yloop <= y_hi; yloop++)
    {
        // read in a row of pixels
        glReadPixels(x_low,yloop,(x_hi-x_low+1),1,
                     GL_RGBA, GL_UNSIGNED_BYTE, buffer);
        for (xloop = 0; xloop <= x_hi-x_low; xloop++) {
            *p_data = buffer[xloop*4]; p_data++;
            *p_data = buffer[xloop*4+1]; p_data++;
            *p_data = buffer[xloop*4+2]; p_data++;
        }
    }
  
    delete [] buffer;  
#else
    // If the Sun bug is ever fixed, then we could use the following
    // technique which provides a vast speed improvement on the SGI
  
    // Calling pack alignment ensures that we can grab the any size window
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels(x_low, y_low, x_hi-x_low+1, y_hi-y_low+1, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
#endif

    return data;
}

int
vtkQtRenderWindow::GetPixelData(int x1, int y1, int x2, int y2, 
    int front, vtkUnsignedCharArray *buffer) {
    int     y_low, y_hi;
    int     x_low, x_hi;
    unsigned char   *data = NULL;

    if (y1 < y2) {
        y_low = y1; 
        y_hi  = y2;
    }
    else {
        y_low = y2; 
        y_hi  = y1;
    }

    if (x1 < x2) {
        x_low = x1; 
        x_hi  = x2;
    }
    else {
        x_low = x2; 
        x_hi  = x1;
    }

    buffer->Allocate((x_hi - x_low + 1)*(y_hi - y_low + 1)*3);
    data = buffer->GetPointer(0);

#ifdef sparc
    // We need to read the image data one row at a time and convert it
    // from RGBA to RGB to get around a bug in Sun OpenGL 1.1
    long    xloop, yloop;
    unsigned char *buffer2;
    unsigned char *p_data = NULL;
  
    buffer2 = new unsigned char [4*(x_hi - x_low + 1)];
    p_data = data;
    for (yloop = y_low; yloop <= y_hi; yloop++)
    {
        // read in a row of pixels
        glReadPixels(x_low,yloop,(x_hi-x_low+1),1,
                     GL_RGBA, GL_UNSIGNED_BYTE, buffer);
        for (xloop = 0; xloop <= x_hi-x_low; xloop++) {
            *p_data = buffer2[xloop*4]; p_data++;
            *p_data = buffer2[xloop*4+1]; p_data++;
            *p_data = buffer2[xloop*4+2]; p_data++;
        }
    }
  
    delete [] buffer2;  
#else
    // If the Sun bug is ever fixed, then we could use the following
    // technique which provides a vast speed improvement on the SGI
  
    // Calling pack alignment ensures that we can grab the any size window
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels(x_low, y_low, x_hi-x_low+1, y_hi-y_low+1, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
#endif

    return 1;
}

//
// Modifications:
//   Brad Whitlock, Mon Apr 15 15:42:26 PST 2002
//   Made it return an int.
//
int
vtkQtRenderWindow::SetPixelData(int x1, int y1, int x2, int y2,
    unsigned char *data, int front)
{
    int     y_low, y_hi;
    int     x_low, x_hi;

    if (y1 < y2) {
        y_low = y1; 
        y_hi  = y2;
    }
    else {
        y_low = y2; 
        y_hi  = y1;
    }
  
    if (x1 < x2) {
        x_low = x1; 
        x_hi  = x2;
    }
    else {
        x_low = x2; 
        x_hi  = x1;
    }

#ifdef sparc
    // We need to read the image data one row at a time and convert it
    // from RGBA to RGB to get around a bug in Sun OpenGL 1.1
    long    xloop, yloop;
    unsigned char *buffer;
    unsigned char *p_data = NULL;
  
    buffer = new unsigned char [4*(x_hi - x_low + 1)];

    // now write the binary info one row at a time
    glDisable(GL_BLEND);
    p_data = data;
    for (yloop = y_low; yloop <= y_hi; yloop++) {
        for (xloop = 0; xloop <= x_hi - x_low; xloop++) {
            buffer[xloop*4] = *p_data; p_data++;
            buffer[xloop*4+1] = *p_data; p_data++;
            buffer[xloop*4+2] = *p_data; p_data++;
            buffer[xloop*4+3] = 0xff;
        }
        /* write out a row of pixels */
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        glLoadIdentity();
        glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1),
                       (2.0 * (GLfloat)(yloop) / this->Size[1] - 1),
                       -1.0 );
        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
        glPopMatrix();

        glDrawPixels((x_hi-x_low+1),1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }
    glEnable(GL_BLEND);
#else
    // If the Sun bug is ever fixed, then we could use the following
    // technique which provides a vast speed improvement on the SGI
  
    // now write the binary info
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1), 
                   (2.0 * (GLfloat)(y_low) / this->Size[1] - 1),
                   -1.0 );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    glDisable(GL_BLEND);
    glDrawPixels((x_hi-x_low+1), (y_hi - y_low + 1),
                 GL_RGB, GL_UNSIGNED_BYTE, data);
    glEnable(GL_BLEND);
#endif

    return 1;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetPixelData
//
// Purpose: 
//   Sets the pixel data using a vtkUnsignedCharArray object.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 15:44:56 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
vtkQtRenderWindow::SetPixelData(int x1, int y1, int x2, int y2,
    vtkUnsignedCharArray *data, int front)
{
    return SetPixelData(x1, y1, x2, y2, data->GetPointer(0), front);
}

float *vtkQtRenderWindow::GetRGBAPixelData(int x1, int y1, int x2, int y2, int front) {
    int     y_low, y_hi;
    int     x_low, x_hi;
    int     width, height;
    float   *data = NULL;
    
    if (y1 < y2) {
        y_low = y1; 
        y_hi  = y2;
    }
    else {
        y_low = y2; 
        y_hi  = y1;
    }

    if (x1 < x2) {
        x_low = x1; 
        x_hi  = x2;
    }
    else {
        x_low = x2; 
        x_hi  = x1;
    }

    width  = abs(x_hi - x_low) + 1;
    height = abs(y_hi - y_low) + 1;

    data = new float[ (width*height*4) ];

    glReadPixels( x_low, y_low, width, height, GL_RGBA, GL_FLOAT, data);

    return data;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::GetRGBAPixelData
//
// Purpose: 
//   Returns the frame buffer stored in a vtkFloatArray.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 15:58:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
vtkQtRenderWindow::GetRGBAPixelData(int x1, int y1, int x2, int y2, int front,
    vtkFloatArray *buffer)
{
    int     y_low, y_hi;
    int     x_low, x_hi;
    int     width, height;
    
    if (y1 < y2) {
        y_low = y1; 
        y_hi  = y2;
    }
    else {
        y_low = y2; 
        y_hi  = y1;
    }

    if (x1 < x2) {
        x_low = x1; 
        x_hi  = x2;
    }
    else {
        x_low = x2; 
        x_hi  = x1;
    }

    width  = abs(x_hi - x_low) + 1;
    height = abs(y_hi - y_low) + 1;

    buffer->Allocate(width*height*sizeof(float));
    float *data = buffer->GetPointer(0);

    glReadPixels( x_low, y_low, width, height, GL_RGBA, GL_FLOAT, data);

    return 1;
}

int
vtkQtRenderWindow::SetRGBAPixelData(int x1, int y1, int x2, int y2,
                                         float *data, int front, int blend) {
    int     y_low, y_hi;
    int     x_low, x_hi;
    int     width, height;
    
    if (y1 < y2) {
        y_low = y1; 
        y_hi  = y2;
    }
    else {
        y_low = y2; 
        y_hi  = y1;
    }
  
    if (x1 < x2) {
        x_low = x1; 
        x_hi  = x2;
    }
    else {
        x_low = x2; 
        x_hi  = x1;
    }
  
    width  = abs(x_hi-x_low) + 1;
    height = abs(y_hi-y_low) + 1;

    /* write out a row of pixels */
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1), 
                   (2.0 * (GLfloat)(y_low) / this->Size[1] - 1),
                   -1.0 );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
  
    if (!blend) {
        glDisable(GL_BLEND);
        glDrawPixels( width, height, GL_RGBA, GL_FLOAT, data);
        glEnable(GL_BLEND);
    }
    else {
        glDrawPixels( width, height, GL_RGBA, GL_FLOAT, data);
    }
    return 1;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetRGBAPixelData
//
// Purpose: 
//   Sets the pixels using a vtkFloatArray.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 16:15:55 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
vtkQtRenderWindow::SetRGBAPixelData(int x1, int y1, int x2, int y2,
    vtkFloatArray *buffer, int front, int blend)
{
    return SetRGBAPixelData(x1, y1, x2, y2, buffer->GetPointer(0),
                            front, blend);
}

int 
vtkQtRenderWindow::GetZbufferData( int x1, int y1, int x2, int y2, float *data)
{
    data = GetZbufferData(x1, y1, x2, y2);
    return 1;
} 

// Modifications by LLNL:
//      y_hi and x_hi removed since they were not being used.
float *vtkQtRenderWindow::GetZbufferData( int x1, int y1, int x2, int y2  ) {
    int             y_low;
    int             x_low;
    int             width, height;
    float           *z_data = NULL;

    if (y1 < y2) {
        y_low = y1; 
    }
    else {
        y_low = y2; 
    }

    if (x1 < x2) {
        x_low = x1; 
    }
    else {
        x_low = x2; 
    }

    width =  abs(x2 - x1)+1;
    height = abs(y2 - y1)+1;

    z_data = new float[width*height];

    glReadPixels( x_low, y_low, 
                  width, height,
                  GL_DEPTH_COMPONENT, GL_FLOAT,
                  z_data );
    return z_data;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::GetZbufferData
//
// Purpose: 
//   Gets the Z buffer and stores it in a vtkFloatArray.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 15:38:09 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
vtkQtRenderWindow::GetZbufferData( int x1, int y1, int x2, int y2,
    vtkFloatArray *buffer)
{
    int             y_low;
    int             x_low;
    int             width, height;
    float           *z_data = NULL;

    if (y1 < y2) {
        y_low = y1; 
    }
    else {
        y_low = y2; 
    }

    if (x1 < x2) {
        x_low = x1; 
    }
    else {
        x_low = x2; 
    }

    width =  abs(x2 - x1)+1;
    height = abs(y2 - y1)+1;

    buffer->Allocate(width*height);
    z_data = buffer->GetPointer(0);

    glReadPixels( x_low, y_low, 
                  width, height,
                  GL_DEPTH_COMPONENT, GL_FLOAT,
                  z_data );

    return 1;
}

// Modifications by LLNL:
//    y_hi and x_hi removed since they were not being used.
//
//    Brad Whitlock, Mon Apr 15 15:32:18 PST 2002
//    Made the return type int and made it return 1.
//
int
vtkQtRenderWindow::SetZbufferData( int x1, int y1, int x2, int y2,
    float *buffer )
{
    int             y_low;
    int             x_low;
    int             width, height;
    
    if (y1 < y2) {
        y_low = y1; 
    }
    else {
        y_low = y2; 
    }

    if (x1 < x2) {
        x_low = x1; 
    }
    else {
        x_low = x2; 
    }

    width =  abs(x2 - x1)+1;
    height = abs(y2 - y1)+1;

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2f( 2.0 * (GLfloat)(x_low) / this->Size[0] - 1, 
                   2.0 * (GLfloat)(y_low) / this->Size[1] - 1);
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glDrawPixels( width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);

    return 1;
}

// ****************************************************************************
// Method: vtkQtRenderWindow::SetZbufferData
//
// Purpose: 
//   Sets the Zbuffer values using a vtkFloatArray.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 15 15:35:48 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
vtkQtRenderWindow::SetZbufferData( int x1, int y1, int x2, int y2,
    vtkFloatArray *buffer )
{
    return SetZbufferData(x1, y1, x2, y2, buffer->GetPointer(0));
}
