// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VTK_DEBUG_STREAM_H
#define VTK_DEBUG_STREAM_H
#include <visit_vtk_light_exports.h>

#include <vtkOutputWindow.h>

class VISIT_VTK_LIGHT_API vtkDebugStream : public vtkOutputWindow
{
public:
  vtkTypeMacro(vtkDebugStream,vtkObject);

  static vtkDebugStream *New();

  static void   Initialize(void);

  virtual void  DisplayText(const char *) override;
  
  static void   FrequentEventsFilter(bool on){filterFrequentEvents = on;}

protected:
  vtkDebugStream() {};
  virtual ~vtkDebugStream() {};

private:
  vtkDebugStream(const vtkDebugStream&); 
  void operator=(const vtkDebugStream&);
  static bool filterFrequentEvents;

};

#endif
