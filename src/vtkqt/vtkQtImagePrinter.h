// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef __vtkQtImagePrinter_h
#define __vtkQtImagePrinter_h
#include <vtkqt_exports.h>
#include <QtCore>
#include <QPrinter>
#include <vtkImageWriter.h>

// ****************************************************************************
// Class: vtkQtImagePrinter
//
// Purpose:
//   This is an image file writer class that writes an image to a Qt printer
//   and thus prints the image. Printing was done in this manner to easily
//   fit into the rest of the pipeline while taking advantage of Qt's
//   printing capabilities.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 15:46:31 PST 2002
//
// Modifications:
//   Burlen Loring, Thu Aug 13 10:13:23 PDT 2015
//   delete the copy constructor and assignment operator
//
// ****************************************************************************

class VTKQT_API vtkQtImagePrinter : public vtkImageWriter
{
public:
  static vtkQtImagePrinter *New();
  vtkTypeMacro(vtkQtImagePrinter,vtkImageWriter);

  QPrinter &printer() { return print; };
protected:
  vtkQtImagePrinter();
  virtual ~vtkQtImagePrinter() {};
  vtkQtImagePrinter(const vtkQtImagePrinter&); // not implemented
  void operator=(const vtkQtImagePrinter&); // not implemented

  void WriteFile(ostream *file, vtkImageData *data, int ext[6], int wext[6]) override;
  void WriteFileHeader(ostream *, vtkImageData *, int [6]) override { };
private:
  QPrinter print;
};

#endif
