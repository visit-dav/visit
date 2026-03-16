OpenGL in VisIt
===============

VisIt requires an OpenGL 3.2 context to work properly. Mesa provides a 3.3 context. Most desktop computers or laptops with graphics cards provide an OpenGL 4.6 or 4.7 context. For some unknown reason most (if not all) Linux HPC systems only provide a 3.0 context.

When using the QVTKOpenGLWidget with Qt, the following code snippet needs to be executed before creating the QApplication to tell Qt that it needs an OpenGL 3.2 context. ::

  //
  // Setting default QSurfaceFormat required with QVTKOpenGLwidget.
  //
  auto surfaceFormat = QVTKOpenGLWidget::defaultFormat();
  surfaceFormat.setSamples(0);
  surfaceFormat.setAlphaBufferSize(0);
  QSurfaceFormat::setDefaultFormat(surfaceFormat);

OpenGL in Qt
------------

The sections of Qt that deal with OpenGL are ::

  qtbase/src/opengl
  qtbase/src/openglextensions

  qtbase/src/plugins/platforms/xcb/gl_integrations/xcb_glx

  qtbase/src/platformsupport/glxconvenience

The context creation is performed in ::

  qtbase/src/plugins/platforms/xcb/gl_integrations/xcb_glx/qglxintegration.cpp

  void QGLXContext::init(QXcbScreen *screen, QPlatformOpenGLContext *share)

OpenGL in VTK
-------------

The sections of VTK that deal with OpenGL are :: 

  GUISupport/Qt
  Rendering/OpenGL2

The context creation is performed in ::

  GUISupport/Qt/QVTKOpenGLWidget.cxx

Other stuff is done in ::

  Rendering/OpenGL2/vtkOpenGLRenderWindow.cxx

OpenGL documentation
--------------------

GLX is the OpenGL extension to the X Window System. In the X Window System, OpenGL rendering is made available as an extension to X in the formal X sense: connection and authentication are accomplished with the normal X mechanisms. As with other X extensions, there is a defined network protocol for the OpenGL rendering commands encapsulated within the X byte stream.

Since performance is critical in 3D rendering, there is a way for OpenGL rendering to bypass the data encoding step, the data copying, and interpretation of that data by the X server. This direct rendering is possible only when a process has direct access to the graphics pipeline.

* `Documentation on GLX <https://www.khronos.org/registry/OpenGL/specs/gl/glx1.4.pdf>`_.
  * GLX functions all start with "glX" and GLX constants all start with "GLX".
* `Documentation on creating an OpenGL 3.0 context <https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)>`_.
  * It is the source of the test in build_visit to determine if the OpenGL on a system supports creating a 3.2 context.
* `Documentation on the history of the changes to OpenGL <https://www.khronos.org/opengl/wiki/History_of_OpenGL>`_.

