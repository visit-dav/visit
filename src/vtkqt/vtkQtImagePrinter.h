#ifndef __vtkQtImagePrinter_h
#define __vtkQtImagePrinter_h
#include <vtkqt_exports.h>
#include <vtkImageWriter.h>
#include <qprinter.h>

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
  vtkQtImagePrinter(const vtkQtImagePrinter&) {};
  void operator=(const vtkQtImagePrinter&) {};

  virtual void WriteFile(ofstream *file, vtkImageData *data, int ext[6]);
  virtual void WriteFileHeader(ofstream *, vtkImageData *) { };
private:
  QPrinter print;
};

#endif
