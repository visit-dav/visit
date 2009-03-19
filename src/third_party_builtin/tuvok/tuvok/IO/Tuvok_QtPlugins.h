/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
#pragma once

#ifndef TUVOK_QTPLUGINS_H
#define TUVOK_QTPLUGINS_H

/**
  \file    TUVOK_QtPlugins.h
  \date    December 2008
*/

#include <QtCore/QtPlugin>

#ifndef TUVOK_NO_IO
  #ifndef QT_SHARED
    Q_IMPORT_PLUGIN(qjpeg)
    Q_IMPORT_PLUGIN(qtiff)
    Q_IMPORT_PLUGIN(qgif)

    #ifdef _WIN32
      #pragma comment( lib, "Imm32.lib" )
      #pragma comment( lib, "Winmm.lib" )
      #pragma comment( lib, "ws2_32.lib" )
      #ifdef _DEBUG
        #pragma comment( lib, "qjpegd.lib" )
        #pragma comment( lib, "qtiffd.lib" )
        #pragma comment( lib, "qgifd.lib" )
      #else
        #pragma comment( lib, "qjpeg.lib" )
        #pragma comment( lib, "qtiff.lib" )
        #pragma comment( lib, "qgif.lib" )
      #endif
    #endif // _WIN32
  #endif // QT_SHARED
#endif // TUVOK_NO_IO

#endif // TUVOK_QTPLUGINS_H
