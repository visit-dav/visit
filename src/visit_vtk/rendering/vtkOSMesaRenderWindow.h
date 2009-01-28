/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkOSMesaRenderWindow.h,v $
  Language:  C++
  Date:      $Date: 2003/07/23 14:15:44 $
  Version:   $Revision: 1.24 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOSMesaRenderWindow - OpenGL rendering window
// .SECTION Description
// vtkOSMesaRenderWindow is a concrete implementation of the abstract class
// vtkMesaRenderWindow that makes it possible to render images into an
// offscreen Mesa buffer. vtkOSMesaRenderer interfaces to the Mesa graphics
// library. Application programmers should normally use vtkRenderWindow
// instead of the OSMesa specific version.

#ifndef __vtkOSMesaRenderWindow_h
#define __vtkOSMesaRenderWindow_h

#include "vtkMesaRenderWindow.h"
#include <rendering_visit_vtk_exports.h>


class vtkIdList;
class vtkOSMesaRenderWindowInternal;

class RENDERING_VISIT_VTK_API vtkOSMesaRenderWindow : public vtkMesaRenderWindow
{
public:
  static vtkOSMesaRenderWindow *New();
  vtkTypeRevisionMacro(vtkOSMesaRenderWindow,vtkMesaRenderWindow);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void SetSize(int,int);

  // Description:
  // Begin the rendering process.
  virtual void Start(void);

  // Description:
  // End the rendering process and display the image.
  virtual void Frame(void);

  // Description:
  // Initialize the window for rendering.
  virtual void WindowInitialize(void);

  // Description:
  // Initialize the rendering window.  This will setup all system-specific
  // resources.  This method and Finalize() must be symmetric and it
  // should be possible to call them multiple times, even changing WindowId
  // in-between.  This is what WindowRemap does.
  virtual void Initialize(void);

  // Description:
  // "Deinitialize" the rendering window.  This will shutdown all system-specific
  // resources.  After having called this, it should be possible to destroy
  // a window that was used for a SetWindowId() call without any ill effects.
  virtual void Finalize(void);

  // Description:
  // Resize the window.
  virtual void WindowRemap(void);

  // Description:
  // Make this window the current OpenGL context.
  void MakeCurrent();

  // Description:
  // Xwindow get set functions
  virtual void *GetGenericWindowId();
  virtual void *GetGenericContext();
  
  // Description:
  // Get the size of the screen in pixels
  virtual int     *GetScreenSize();


  // Methods that are pure virtual in vtkMesaRenderWindow that must be
  // defined for the thing to work.
   virtual void SetDisplayId(void*);
   virtual void SetWindowId(void*);
   virtual void SetParentId(void*);
   virtual void* GetGenericDisplayId();
   virtual void* GetGenericParentId();
   virtual void* GetGenericDrawable();
   virtual void SetWindowInfo(char*);
   virtual void SetParentInfo(char*);
   virtual void HideCursor();
   virtual void ShowCursor();
   virtual void SetFullScreen(int);
   virtual int GetEventPending();
   virtual void SetNextWindowId(void*);
   virtual void SetNextWindowInfo(char*);


   void SavePPM(const char *filename);

protected:
  vtkOSMesaRenderWindow();
  virtual ~vtkOSMesaRenderWindow();

  vtkOSMesaRenderWindowInternal *Internal;
 
private:
  vtkOSMesaRenderWindow(const vtkOSMesaRenderWindow&);  // Not implemented.
  void operator=(const vtkOSMesaRenderWindow&);  // Not implemented.
};



#endif
