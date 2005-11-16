// ************************************************************************* //
//                         avtDataRepresentation.C                           //
// ************************************************************************* //
#include <avtDataRepresentation.h>
#include <avtCommonDataFunctions.h>

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
// Static members
//
bool          avtDataRepresentation::initializedNullDataset = false;
vtkDataSet   *avtDataRepresentation::nullDataset = NULL;

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
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression data members
// ****************************************************************************

avtDataRepresentation::avtDataRepresentation()
{
    asVTK        = NULL;
    asChar       = NULL;
    asCharLength = 0;
    domain       = -1;
    datasetType  = DATASET_TYPE_UNKNOWN;
    compressionRatio = -1.0;
    timeToCompress   = -1.0;
    timeToDecompress = -1.0;
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
//    Hank Childs, Wed Mar 17 19:21:22 PST 2004
//    Make use of the null dataset to make sure we don't blow memory in SR-mode
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression data members
// ****************************************************************************

avtDataRepresentation::avtDataRepresentation(vtkDataSet *d, int dom, string s,
                                             bool dontCopyData)
{
    InitializeNullDataset();

    if (dontCopyData)
    {
       asVTK = nullDataset;
       datasetType = DATASET_TYPE_NULL;
       asVTK->Register(NULL);
    }
    else
    {
       asVTK        = d;
       if (asVTK != NULL)
       {
           asVTK->Register(NULL);
       }
    }
    asChar       = NULL;
    asCharLength = 0;
    datasetType  = DATASET_TYPE_UNKNOWN;
    domain       = dom;
    label        = s ;
    compressionRatio = -1.0;
    timeToCompress   = -1.0;
    timeToDecompress = -1.0;
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
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression data members
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
    compressionRatio = -1.0;
    timeToCompress   = -1.0;
    timeToDecompress = -1.0;
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
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression data members
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
    compressionRatio = rhs.compressionRatio;
    timeToCompress = rhs.timeToCompress;
    timeToDecompress = rhs.timeToDecompress;
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
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression data members
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
    compressionRatio = rhs.compressionRatio;
    timeToCompress = rhs.timeToCompress;
    timeToDecompress = rhs.timeToDecompress;

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
//  Method: avtDataRepresentation::GetNumberOfCells
//
//  Purpose:
//     Count number of cells in this object. If we're counting polygons only
//     we'll return the value for the underlying vtk's GetNumberOfCells()
//     only if topoDim < 3;
//
//  Programmer: Mark C. Miller 
//  Creation:   19Aug03 
//
//  Modifications:
//
//  Mark C. Miller, Wed Nov  5 09:48:13 PST 2003
//  Added option to count polygons only
//
// ****************************************************************************

int
avtDataRepresentation::GetNumberOfCells(int topoDim, bool polysOnly) const
{
   if (asVTK == NULL)
   {
      if (asChar == NULL)
         EXCEPTION0(NoInputException);
      return -1;
   }
   else
   {
      int numCells = 0;

      if (polysOnly)
      {
         if (topoDim < 3)
            numCells = (int) asVTK->GetNumberOfCells();
         else
            numCells = 0;
      }
      else
      {
         numCells = (int) asVTK->GetNumberOfCells();
      }
       
      return numCells;

   }
}

// ****************************************************************************
//  Method: avtDataRepresentation::GetDataString
//
//  Purpose: Public interface to get the data as a character string
//           w/o compression
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005
//
// ****************************************************************************

unsigned char *
avtDataRepresentation::GetDataString(int &length, DataSetType &dst)
{
    const bool useCompression = false;
    return GetDataString(length, dst, useCompression);
}

// ****************************************************************************
//  Method: avtDataRepresentation::GetCompressedDataString
//
//  Purpose: Public interface to get the data as a character string
//           with compression
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005
//
// ****************************************************************************

unsigned char *
avtDataRepresentation::GetCompressedDataString(int &length, DataSetType &dst)
{
    const bool useCompression = true;
    return GetDataString(length, dst, useCompression);
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
//    Hank Childs, Wed Mar 17 19:21:22 PST 2004
//    Make use of the null dataset to make sure we don't blow memory in SR-mode
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression
// ****************************************************************************

unsigned char *
avtDataRepresentation::GetDataString(int &length, DataSetType &dst, bool compress)
{
    InitializeNullDataset();

    if (asChar == NULL)
    {
        if (asVTK == NULL)
        {
            EXCEPTION0(NoInputException);
        }
        else if (asVTK == nullDataset)
        {
            dst = DATASET_TYPE_NULL;
            asCharLength = 0;
            asChar = NULL;
        }
        else
        {
            dst = DatasetTypeForVTK(asVTK);
            datasetType = dst;
             
            vtkDataSetWriter *writer = vtkDataSetWriter::New();
            writer->SetInput(asVTK);
            writer->SetWriteToOutputString(1);
            writer->SetFileTypeToBinary();
            writer->Write();
            asCharLength = writer->GetOutputStringLength();
            asChar = (unsigned char *) writer->RegisterAndGetOutputString();
            writer->Delete();

            if (compress)
            {
                int asCharLengthNew = 0;
                unsigned char *asCharNew = 0;
                if (CCompressDataString(asChar, asCharLength,
                                        &asCharNew, &asCharLengthNew,
                                        &timeToCompress, &compressionRatio))
                {
                    delete [] asChar;
                    asChar = asCharNew;
                    asCharLength = asCharLengthNew;
                }
            }

            originalString = (char *)asChar;
        }
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
//    Hank Childs, Wed Mar 17 19:21:22 PST 2004
//    Make use of the null dataset to make sure we don't blow memory in SR-mode
//
//    Hank Childs, Sun Mar  6 08:18:53 PST 2005
//    Remove the data string after creating the VTK data set.  This 
//    substantially reduces memory footprint.
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression
// ****************************************************************************

vtkDataSet *
avtDataRepresentation::GetDataVTK(void)
{
    InitializeNullDataset();

    if (asVTK == NULL)
    {
        if (asChar == NULL)
        {
            EXCEPTION0(NoInputException);
        }
        else if (datasetType == DATASET_TYPE_NULL)
        {
            asVTK = nullDataset;
            asVTK->Register(NULL);
        }
        else
        {
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
                vtkUnstructuredGridReader *r1 = 
                                              vtkUnstructuredGridReader::New();
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

            int asCharLengthNew = 0;
            int asCharLengthTmp = asCharLength;
            unsigned char *asCharNew = 0;
            unsigned char *asCharTmp = asChar;
            if (CDecompressDataString(asChar, asCharLength, &asCharNew, &asCharLengthNew,
                              &timeToCompress, &timeToDecompress, &compressionRatio))
            {
                
                asCharTmp = asCharNew;
                asCharLengthTmp = asCharLengthNew;
            }

            vtkCharArray *charArray = vtkCharArray::New();
            int iOwnIt = 1;  // 1 means we own it -- you don't delete it.
            charArray->SetArray((char *) asCharTmp, asCharLengthTmp, iOwnIt);
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
            originalString = NULL;
            if (asCharNew)
                delete [] asCharNew;
        }
    }

    return asVTK;
}


// ****************************************************************************
//  Method: avtDataRepresentation::InitializeNullDataset
//
//  Purpose:
//      The null dataset is used to represent that the contents of a tree
//      are not present (because we are in scalable rendering mode).  Prior
//      to using the null dataset (which is a singleton), each node of a tree
//      got its own, personal null dataset.  In cases where there were 10,000
//      domains, this was a large memory hit.  For this reason, all of the
//      nodes now share this single null dataset.  The null dataset is a
//      static to the avt data representation class.  It is important that
//      any method that might reference the null dataset call this routine
//      first.
//
//  Programmer: Hank Childs
//  Creation:   March 17, 2004
//
// ****************************************************************************

void
avtDataRepresentation::InitializeNullDataset(void)
{
    if (initializedNullDataset)
        return;

    // build the points object (not a vtkDataSet object)
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
    vtkPoints           *dummyPoints = vtkPoints::New();
    dummyPoints->SetNumberOfPoints(0);
    ugrid->SetPoints(dummyPoints);
    dummyPoints->Delete();

    nullDataset = ugrid;
    initializedNullDataset = true;
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
//  Modifications:
//
//    Hank Childs, Wed Mar 17 19:51:05 PST 2004
//    Account for the null dataset.
//
// ****************************************************************************

DataSetType
avtDataRepresentation::DatasetTypeForVTK(vtkDataSet *ds)
{
    InitializeNullDataset();

    if (ds == nullDataset)
        return DATASET_TYPE_NULL;

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

// ****************************************************************************
//  Method: avtDataRepresentation::GetCompressionRatio
//
//  Purpose: Gets the compression ratio, if any, without uncompressing 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
// ****************************************************************************

float
avtDataRepresentation::GetCompressionRatio() const
{
    if (compressionRatio != -1.0)
        return compressionRatio;

    if (asChar != NULL)
    {
        float ratioc;
        CGetCompressionInfoFromDataString(asChar, asCharLength, 
            0, &ratioc);
        return ratioc;
    }

    return compressionRatio;
}

// ****************************************************************************
//  Method: avtDataRepresentation::GetTimeToCompress
//
//  Purpose: Gets the time to compress, if any, without uncompressing 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
// ****************************************************************************

float
avtDataRepresentation::GetTimeToCompress() const
{
    if (timeToCompress != -1.0)
        return timeToCompress;

    if (asChar != NULL)
    {
        float timec;
        CGetCompressionInfoFromDataString(asChar, asCharLength, 
            &timec, 0);
        return timec;
    }

    return timeToCompress;
}

// ****************************************************************************
//  Method: avtDataRepresentation::GetTimeToDecompress
//
//  Purpose: Gets the time to decompress, if any, without uncompressing 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 15, 2005 
//
// ****************************************************************************

float
avtDataRepresentation::GetTimeToDecompress() const
{
    return timeToDecompress;
}
