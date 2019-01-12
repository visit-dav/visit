#include <vtkMultiFontVectorText.h>
#include <vtkVectorFontCharacterData.h>

#include <vtkCellArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

vtkStandardNewMacro(vtkMultiFontVectorText);

// ****************************************************************************
// Method: vtkMultiFontVectorText::vtkMultiFontVectorText
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 16:02:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

vtkMultiFontVectorText::vtkMultiFontVectorText() : vtkPolyDataAlgorithm()
{
  this->Text = NULL;
  this->FontFamily = VTK_TIMES;
  this->Bold = false;
  this->Italic = false;
  this->SetNumberOfInputPorts(0);
}

// ****************************************************************************
// Method: vtkMultiFontVectorText::~vtkMultiFontVectorText
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 16:03:03 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

vtkMultiFontVectorText::~vtkMultiFontVectorText()
{
    if (this->Text)
        delete [] this->Text;
}

// ****************************************************************************
// Method: vtkMultiFontVectorText::RequestData
//
// Purpose: 
//   Assembles the polydata for the input text using the font properties to
//   select the right polydata.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 16:03:16 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int vtkMultiFontVectorText::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
    // get the info object
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the ouptut
    vtkPolyData *output = vtkPolyData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));
 
    if (this->Text == NULL)
    {
        vtkErrorMacro (<< "Text is not set!");
        return 0;
    }

    // Set things up; allocate memory
    vtkPoints *newPoints = vtkPoints::New();
    vtkCellArray *newPolys = vtkCellArray::New();
    int pos = 0;
    float xpos = 0.f;
    float ypos = 0.f;
    float ftmp[3] = {0.f, 0.f, 0.f};

    // Create Text
    while (this->Text[pos])
    {
        switch (this->Text[pos])
        {
        case 10:
            ypos -= 1.4f;
            xpos = 0.f;
            break;
        case 32: 
            xpos += 0.4f;
            break; 
        default:
            // Get the font character data.
            const vtkVectorFontCharacterData *cData = 
                vtkVectorFontCharacterData::GetCharacterData(
                    this->Text[pos], this->FontFamily, this->Bold, this->Italic);
            if(cData != 0)
            {
                // Add the points.
                int i, index = 0;
                float scale = cData->scale;
                float bias = cData->bias;
                vtkIdType basePoint = newPoints->GetNumberOfPoints();
                for(i = 0; i < cData->npts; ++i, index += 2)
                {
                    float x = bias + (float)cData->pts[index] * scale;
                    float y = bias + (float)cData->pts[index+1] * scale;
                    ftmp[0] = xpos + x;
                    ftmp[1] = ypos + y;

                    newPoints->InsertNextPoint(ftmp);
                }
  
                // Add the cells.
                index = 0;
                for (i = 0; i < cData->ntris; ++i, index += 3)
                {
                    vtkIdType ids[3];
                    ids[0] = basePoint + (vtkIdType)cData->tris[index];
                    ids[1] = basePoint + (vtkIdType)cData->tris[index+1];
                    ids[2] = basePoint + (vtkIdType)cData->tris[index+2];
                    newPolys->InsertNextCell(3, ids);
                }

                xpos += cData->width;
            }
            break;
        }
        pos++;
    }
  
    //
    // Update ourselves and release memory
    //
    output->SetPoints(newPoints);
    newPoints->Delete();
  
    output->SetPolys(newPolys);
    newPolys->Delete();

    return 1;
}

// ****************************************************************************
// Method: vtkMultiFontVectorText::PrintSelf
//
// Purpose: 
//   Prints the object to a stream.
//
// Arguments:
//   os     : The stream to which we'll print.
//   indent : The indentation level.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 27 16:04:08 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
vtkMultiFontVectorText::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    os << indent << "Text: " << (this->Text ? this->Text : "(none)") << "\n";
    os << indent << "FontFamily: " << this->FontFamily << "\n";
    os << indent << "Bold: " << (this->Bold?"true":"false") << "\n";
    os << indent << "Italic: " << (this->Italic?"true":"false") << "\n";
}
