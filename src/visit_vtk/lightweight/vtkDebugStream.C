// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vtkDebugStream.h>

#include <vtkObjectFactory.h>

#include <DebugStream.h>


// *****************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Use macro for 'New' method.
// *****************************************************

vtkStandardNewMacro(vtkDebugStream);

// *****************************************************
// Modifications:
//   Cyrus Harrison, Sat Aug 11 20:37:23 PDT 2007
//   Added init of filterFrequentEvents
// *****************************************************
bool vtkDebugStream::filterFrequentEvents = false;

// *****************************************************
// Modifications:
//   Hank Childs, Tue Mar 22 16:19:59 PST 2005
//   Fix memory leak.
// *****************************************************

void vtkDebugStream::Initialize(void)
{
  vtkDebugStream *ds = vtkDebugStream::New();
  vtkOutputWindow::SetInstance(ds);
  ds->Delete();
}

// *****************************************************
// Modifications:
//   Cyrus Harrison, Sat Aug 11 20:37:23 PDT 2007
//   Added a filter for frequent events to reduce
//   strain on file system.
// *****************************************************
void vtkDebugStream::DisplayText(const char *txt)
{
    // exclude very frequent events to spare the fs
  if(filterFrequentEvents)
  {
      bool ok = true;
      ok = ok && strstr(txt,"Returning cell type") == NULL;
      
      if(ok)
      {debug1 << txt << endl;}
  }
  else
  {debug1 << txt << endl;}

}
