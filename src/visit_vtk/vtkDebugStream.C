#include <vtkDebugStream.h>

#include <vtkObjectFactory.h>

#include <DebugStream.h>


// *****************************************************
// Modifications:
//   Kathleen Bonnell, Wed Mar  6 13:48:48 PST 2002
//   Use macro for 'New' method.
// *****************************************************

vtkStandardNewMacro(vtkDebugStream);


void vtkDebugStream::Initialize(void)
{
  vtkDebugStream *ds = vtkDebugStream::New();
  vtkOutputWindow::SetInstance(ds);
}


void vtkDebugStream::DisplayText(const char *txt)
{
  debug1 << txt << endl;
}
