// ************************************************************************* //
//                         avtDataRepresentation.C                           //
// ************************************************************************* //

#include <avtDataRepresentation.h>

#include <vtkCharArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetReader.h>
#include <vtkDataSetWriter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkRectilinearGrid.h>
#include <vtkRectilinearGridReader.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>

#include <NoInputException.h>
#include <ImproperUseException.h>
#include <DebugStream.h>


using std::string;


//
// Function Prototypes
//

static DataSetType DatasetTypeForVTK(vtkDataSet *);


// ****************************************************************************
//  Method: avtDataRepresentation default constructor
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Sep 17 09:14:03 PDT 2001
//    Removed references to asCharRef.
//
//    Hank Childs, Thu Sep 27 16:43:27 PDT 2001
//    Initialized datasetType.
//
// ****************************************************************************

avtDataRepresentation::avtDataRepresentation()
{
    asVTK        = NULL;
    asChar       = NULL;
    asCharLength = 0;
    domain       = -1;
    datasetType  = DATASET_TYPE_UNKNOWN;
}


// ****************************************************************************
//  Method: avtDataRepresentation constructor
//
//  Arguments:
//      d       The dataset this object should represent.
//      dom     The domain for this dataset.
//      s       The label for this dataset.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 25, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun  4 14:02:19 PDT 2001
//    Added domain argument.
//
//    Hank Childs, Mon Sep 17 09:14:03 PDT 2001
//    Removed references to asCharRef.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Added string argument. 
//
//    Hank Childs, Thu Sep 27 16:43:27 PDT 2001
//    Initialized datasetType.
//
// ****************************************************************************

avtDataRepresentation::avtDataRepresentation(vtkDataSet *d, int dom, string s)
{
    asVTK        = d;
    if (asVTK != NULL)
    {
        asVTK->Register(NULL);
    }
    asChar       = NULL;
    asCharLength = 0;
    datasetType  = DATASET_TYPE_UNKNOWN;
    domain       = dom;
    label        = s ;
}


// ****************************************************************************
//  Method: avtDataRepresentation constructor
//
//  Arguments:
//      d       A character string representation of d.
//      dl      The length of d.
//      dom     The domain for this dataset.
//      s       The label for this dataset.
//      os      A character string reference that somehow contains 'd'.
//      dst     The dataset type.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 25, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jun  4 14:02:19 PDT 2001
//    Added domain argument.
//
//    Hank Childs, Mon Sep 17 09:27:11 PDT 2001
//    Changed method for reference counting the character string.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Added string argument. 
//
//    Hank Childs, Thu Sep 27 16:43:27 PDT 2001
//    Added argument to dataset type.
//
// ****************************************************************************

avtDataRepresentation::avtDataRepresentation(char *d, int dl, int dom, 
                                     string s, CharStrRef &os, DataSetType dst)
{
    asVTK  = NULL;

    asCharLength = dl;
    asChar       = (unsigned char *) d;
    datasetType  = dst;

    originalString = os;
    domain         = dom;
    label          = s; 
}


// ****************************************************************************
//  Method: avtDataRepresentation copy constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September 25, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Wed Sep 27 10:42:22 PDT 2000
//    Initialize the pointers to NULL in case they aren't copied from rhs.
//
//    Hank Childs, Mon Jun  4 14:02:19 PDT 2001
//    Copied over domain.
//
//    Hank Childs, Mon Sep 17 09:27:11 PDT 2001
//    Changed method for reference counting the character string.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001
//    Copy over label. 
//
//    Hank Childs, Thu Sep 27 16:43:27 PDT 2001
//    Initialized datasetType.
//
// ****************************************************************************

avtDataRepresentation::avtDataRepresentation(const avtDataRepresentation &rhs)
{
    asVTK = NULL;
    asChar = NULL;

    if (rhs.asVTK)
    {
        asVTK  = rhs.asVTK;
        asVTK->Register(NULL);
    }
    if (rhs.asChar)
    {
        asChar         = rhs.asChar;
        asCharLength   = rhs.asCharLength; 
        originalString = rhs.originalString;
    }

    datasetType = rhs.datasetType;
    domain = rhs.domain;
    label = rhs.label; 
}


// ****************************************************************************
//  Method: avtDataRepresentation destructor
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Sep 26 09:27:19 PDT 2000
//    Added reference counting of the string.
//
//    Hank Childs, Mon Sep 17 09:27:11 PDT 2001
//    Changed method for reference counting the character string.
//
// ****************************************************************************

avtDataRepresentation::~avtDataRepresentation()
{
    if (asVTK)
    {
        asVTK->Delete();
        asVTK = NULL;
    }
    if (asChar)
    {
        //
        // Do not free asChar -- it should be contained in originalString.
        //
        asChar = NULL;
        asCharLength = 0;
    }
}


// ****************************************************************************
//  Method: avtDataRepresentation::operator=
//
//  Purpose:
//      Sets the data from another.
//
//  Arguments:
//      rhs     the source avtDataRepresentation
//
//  Programmer: Jeremy Meredith
//  Creation:   Tue Sep 26 09:27:32 PDT 2000
//
//  Modifications:
//
//    Jeremy Meredith, Wed Sep 27 10:42:22 PDT 2000
//    Initialize the pointers to NULL in case they aren't copied from rhs.
//
//    Hank Childs, Mon Jun  4 14:02:19 PDT 2001
//    Copied over domain.
//
//    Hank Childs, Mon Sep 17 09:27:11 PDT 2001
//    Changed method for reference counting the character string.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Copied over label. 
//
//    Hank Childs, Thu Sep 27 16:43:27 PDT 2001
//    Copied over datasetType.
//
// ****************************************************************************

const avtDataRepresentation &
avtDataRepresentation::operator=(const avtDataRepresentation &rhs)
{
    if (asVTK)
    {
        asVTK->Delete();
    }
    if (asChar)
    {
        asChar = NULL;
        asCharLength = 0;
        originalString = NULL;
    }

    asVTK = NULL;
    asChar = NULL;

    if (rhs.asVTK)
    {
        asVTK  = rhs.asVTK;
        asVTK->Register(NULL);
    }
    if (rhs.asChar)
    {
        asChar         = rhs.asChar;
        asCharLength   = rhs.asCharLength; 
        originalString = rhs.originalString;
    }

    datasetType = rhs.datasetType;
    domain = rhs.domain;
    label = rhs.label; 

    return *this;
}


// ****************************************************************************
//  Method: avtDataRepresentation::Valid
//
//  Purpose:
//      Determines if this object is valid.
//
//  Returns:     true if the object is valid, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
// ****************************************************************************

bool
avtDataRepresentation::Valid(void)
{
    return (asVTK != NULL || asChar != NULL ? true : false);
}


// ****************************************************************************
//  Method: avtDataRepresentation::GetDataString
//
//  Purpose:
//      Gets the data as a character string.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Sep 26 09:27:19 PDT 2000
//    Added reference counting of the string.
//
//    Hank Childs, Mon Sep 17 09:27:11 PDT 2001
//    Changed method for reference counting the character string.
//
//    Hank Childs, Thu Sep 27 16:43:27 PDT 2001
//    Added return value for dataset type.
//
// ****************************************************************************

unsigned char *
avtDataRepresentation::GetDataString(int &length, DataSetType &dst)
{
    if (asChar == NULL)
    {
        if (asVTK == NULL)
        {
            EXCEPTION0(NoInputException);
        }

        dst = DatasetTypeForVTK(asVTK);
        datasetType = dst;
         
        vtkDataSetWriter *writer = vtkDataSetWriter::New();
        writer->SetInput(asVTK);
        writer->SetWriteToOutputString(1);
        writer->SetFileTypeToBinary();
        writer->Write();
        asCharLength = writer->GetOutputStringLength();
        asChar = (unsigned char *) writer->RegisterAndGetOutputString();
        originalString = (char *)asChar;
        writer->Delete();
    }
    else
    {
        dst = datasetType;
    }

    length = asCharLength;
    return asChar;
}


// ****************************************************************************
//  Method: avtDataRepresentation::GetDataVTK
//
//  Purpose:
//      Gets the data as a vtkDataSet.
//
//  Returns:      The data as a vtkDataSet.
//
//  Programmer: Hank Childs
//  Creation:   September 22, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Sep 26 09:27:19 PDT 2000
//    Renamed to GetDomainVTK.
//
//    Hank Childs, Thu Sep 27 16:43:27 PDT 2001
//    Made use of datasetType argument to prevent string copy.
//
//    Hank Childs, Thu May  2 15:28:00 PDT 2002
//    Make use of new VTK method that avoids making an unnecessary copy of the
//    data.
//
// ****************************************************************************

vtkDataSet *
avtDataRepresentation::GetDataVTK(void)
{
    if (asVTK == NULL)
    {
        if (asChar == NULL)
        {
            EXCEPTION0(NoInputException);
        }

        vtkDataReader *reader = NULL;
        vtkDataSetReader *dsreader = NULL;
        bool readerIsAFrontEnd = false;

        switch (datasetType)
        {
          case DATASET_TYPE_RECTILINEAR:
          {
            vtkRectilinearGridReader *r1 = vtkRectilinearGridReader::New();
            reader = r1;
            asVTK = r1->GetOutput();
            readerIsAFrontEnd = false;
            break;
          }
          case DATASET_TYPE_CURVILINEAR:
          {
            vtkStructuredGridReader *r1 = vtkStructuredGridReader::New();
            reader = r1;
            asVTK = r1->GetOutput();
            readerIsAFrontEnd = false;
            break;
          }
          case DATASET_TYPE_UNSTRUCTURED:
          {
            vtkUnstructuredGridReader *r1 = vtkUnstructuredGridReader::New();
            reader = r1;
            asVTK = r1->GetOutput();
            readerIsAFrontEnd = false;
            break;
          }
          case DATASET_TYPE_POLYDATA:
          {
            vtkPolyDataReader *r1 = vtkPolyDataReader::New();
            reader = r1;
            asVTK = r1->GetOutput();
            readerIsAFrontEnd = false;
            break;
          }
          default:
          {
            debug1 << "Entered the VTK DATASET READER case. "
                   << "This should not happen." << endl;
            dsreader = vtkDataSetReader::New();
            reader = dsreader;
            readerIsAFrontEnd = true;
            break;
          }
        }
        vtkCharArray *charArray = vtkCharArray::New();
        int iOwnIt = 1;  // 1 means we own it -- you don't delete it.
        charArray->SetArray((char *) asChar, asCharLength, iOwnIt);
        reader->SetReadFromInputString(1);
        reader->SetInputArray(charArray);

        if (readerIsAFrontEnd)
        {
            //
            // Readers that are a front end to other readers automatically
            // do an Update when you get their output.  That is why we are
            // waiting to get their output here (->after we set the string
            // input).
            //
            asVTK = dsreader->GetOutput();
        }
        else
        {
            asVTK->Update();
        }

        asVTK->Register(NULL);
        asVTK->SetSource(NULL);
        reader->Delete();
        charArray->Delete();
    }

    return asVTK;
}


// ****************************************************************************
//  Function: DatasetTypeForVTK
//
//  Purpose:
//      Determines the appropriate DataSetType for a vtkDataSet.
//
//  Arguments:
//      ds      A vtk dataset.
//
//  Returns:    The DataSetType of ds.
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2001
//
// ****************************************************************************

DataSetType
DatasetTypeForVTK(vtkDataSet *ds)
{
    DataSetType rv = DATASET_TYPE_UNKNOWN;

    int vtktype = ds->GetDataObjectType();
    switch (vtktype)
    {
      case VTK_RECTILINEAR_GRID:
        rv = DATASET_TYPE_RECTILINEAR;
        break;

      case VTK_STRUCTURED_GRID:
        rv = DATASET_TYPE_CURVILINEAR;
        break;

      case VTK_UNSTRUCTURED_GRID:
        rv = DATASET_TYPE_UNSTRUCTURED;
        break;

      case VTK_POLY_DATA:
        rv = DATASET_TYPE_POLYDATA;
        break;

      default:
        rv = DATASET_TYPE_UNKNOWN;
        break;
    }

    return rv;
}


