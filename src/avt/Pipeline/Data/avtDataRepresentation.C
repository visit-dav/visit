/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         avtDataRepresentation.C                           //
// ************************************************************************* //

#include <avtDataRepresentation.h>

#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkDataSetReader.h>
#include <vtkDataSetWriter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkRectilinearGrid.h>
#include <vtkRectilinearGridReader.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>

#include <avtCommonDataFunctions.h>
#include <avtDebugDumpOptions.h>
#include <avtParallel.h>
#include <avtWebpage.h>

#include <NoInputException.h>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <visitstream.h>
#include <snprintf.h>

using std::string;
using std::ostringstream;

//
// Static members
//
bool          avtDataRepresentation::initializedNullDataset = false;
vtkDataSet   *avtDataRepresentation::nullDataset = NULL;


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
//
//    Hank Childs, Fri Feb 15 16:17:18 PST 2008
//    Initialize more data members.
//
// ****************************************************************************

avtDataRepresentation::avtDataRepresentation(const avtDataRepresentation &rhs)
{
    asVTK        = NULL;
    asChar       = NULL;
    asCharLength = 0;

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
//
//    Hank Childs, Tue Dec 18 10:25:09 PST 2007
//    Remove const qualification of return type.
//
// ****************************************************************************

avtDataRepresentation &
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
//
//    Kathleen Bonnell, Wed May 17 14:51:16 PDT 2006
//    Removed call to SetSource(NULL) as it now removes information necessary
//    to the dataset.
//
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
            //asVTK->SetSource(NULL);
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


// ****************************************************************************
//  Method: avtDataRepresentation::DebugDump
//
//  Purpose:
//      Writes out its information to a webpage.
//
//  Programmer: Hank Childs
//  Creation:   December 21, 2006
//
//  Modifications:
//
//    Cyrus Harrison, Tue Mar 13 11:41:22 PDT 2007
//    Added case for debug dumps without dumping vtk datasets (-info-dump)
//
//    Hank Childs, Fri Dec  7 09:59:34 PST 2007
//    Added reference counts to output.
//
//    Cyrus Harrison, Thu Jan 10 10:52:42 PST 2008
//    Add more information about number of tuples, components and fixed 
//    bug where some info was droped in the output string.
//
//    Hank Childs, Mon Jan 14 20:27:10 PST 2008
//    Prevent ABRs (array bounds reads) when dealing with bad array sizes.
//    This is particularly needed for singleton expressions.
//
//    Cyrus Harrison, Wed Feb 13 09:40:09 PST 2008
//    Added support for optional -dump output directory.
//
//    Cyrus Harrison, Tue Feb 19 11:41:18 PST 2008
//    Fixed a bug with naming -dump vtk output files in the parallel case. 
//
// **************************************************************************** 

const char *
avtDataRepresentation::DebugDump(avtWebpage *webpage, const char *prefix)
{
    if (asVTK == NULL)
    {
        return "EMPTY DATA SET";
    }
    
    const int strsize = 4096;
    static char str[strsize];

    ostringstream oss;
    
    bool dataset_dump = avtDebugDumpOptions::DatasetDumpEnabled();
    string vtk_fname ="";
    
    if(dataset_dump)
    {
        int  i;
        static int times = 0;
        
        // Construct a data set where all mis-sized arrays are added as
        // field data.  This prevents array bounds reads.  It is also
        // necessary for the way we do singletons with constant expressions.
        vtkDataSet *newDS = (vtkDataSet *) asVTK->NewInstance();
        newDS->ShallowCopy(asVTK);
        int  npt = newDS->GetPointData()->GetNumberOfArrays();
        for (i = npt-1 ; i >= 0 ; i--)
        {
            vtkDataArray *arr = newDS->GetPointData()->GetArray(i);
            if (arr->GetNumberOfTuples() != newDS->GetNumberOfPoints())
            {
                newDS->GetPointData()->RemoveArray(arr->GetName());
                newDS->GetFieldData()->AddArray(arr);
            }
        }
        int  ncell = newDS->GetCellData()->GetNumberOfArrays();
        for (i = ncell-1 ; i >= 0 ; i--)
        {
            vtkDataArray *arr = newDS->GetCellData()->GetArray(i);
            if (arr->GetNumberOfTuples() != newDS->GetNumberOfCells())
            {
                newDS->GetCellData()->RemoveArray(arr->GetName());
                newDS->GetFieldData()->AddArray(arr);
            }
        }
        
        const string &dump_dir = avtDebugDumpOptions::GetDumpDirectory();
        
        ostringstream oss_vtk_fname;
        
        if (PAR_Size() > 1)
        {
            // %s%d.%d.vtk
            oss_vtk_fname << prefix
                          << times << "."
                          << PAR_Rank() << ".vtk";
        }
        else
        {
            // %s%d.vtk
            oss_vtk_fname << prefix
                          << times << ".vtk";
        }
        times++;
        vtk_fname = oss_vtk_fname.str();
        string vtk_fpath = dump_dir + vtk_fname;
        
        vtkDataSetWriter *wrtr = vtkDataSetWriter::New();
        wrtr->SetInput(newDS);
        wrtr->SetFileName(vtk_fpath.c_str());
        wrtr->Write();
        wrtr->Delete();
        newDS->Delete();
    }

    string mesh_type = "<unknown mesh type>";
    int nzones = asVTK->GetNumberOfCells();
    int nnodes = asVTK->GetNumberOfPoints();
    int dims[3] = { -1, -1, -1 };
    int vtktype = asVTK->GetDataObjectType();
    int ptcnt = -1;

    switch (vtktype)
    {
      case VTK_RECTILINEAR_GRID:
        mesh_type = "rectilinear mesh";
        ((vtkRectilinearGrid *) asVTK)->GetDimensions(dims);
        break;

      case VTK_STRUCTURED_GRID:
        mesh_type = "curvilinear mesh";
        ((vtkStructuredGrid *) asVTK)->GetDimensions(dims);
        ptcnt=((vtkStructuredGrid *) asVTK)->GetPoints()->GetReferenceCount();
        break;

      case VTK_UNSTRUCTURED_GRID:
        mesh_type = "unstructured mesh";
        ptcnt=((vtkUnstructuredGrid *) asVTK)->GetPoints()->GetReferenceCount();
        break;

      case VTK_POLY_DATA:
        mesh_type = "poly data mesh";
        ptcnt=((vtkPolyData *) asVTK)->GetPoints()->GetReferenceCount();
        break;
    }

    if (dataset_dump)
    {
        oss << vtk_fname << "<br> " << mesh_type << " ";
    }
    
    if (dims[0] > 0)
    {
        oss << "ncells = " << nzones << " npts = " <<  nnodes 
            << " dims = " << dims[0] << "," << dims[1] << "," << dims[2];
    }
    else
    {
        oss << "ncells = " << nzones << " npts = " <<  nnodes;
    }
    
    oss << "<br>";
   
    if (ptcnt >= 0)
    {
        oss << "Refs to mesh = " << asVTK->GetReferenceCount() 
            << ", to points = "  << ptcnt << "<br>";
    }
    else
    {
        oss << "Refs to mesh = " << asVTK->GetReferenceCount() << "<br>";
    }

    // Do field data.
    vtkFieldData *data[3];
    data[0] = asVTK->GetFieldData();
    data[1] = asVTK->GetPointData();
    data[2] = asVTK->GetCellData();
    char *names[3];
    names[0] = "field";
    names[1] = "point";
    names[2] = "cell";

    for (int fd = 0 ; fd < 3 ; fd++)
    {
        oss << "<b>" << names[fd] << "_data:</b>";
        if(data[fd]->GetNumberOfArrays() == 0)
        {
            oss << " (None)<br>" << endl;
        }
        else
        {
            oss << "<ul>";
            for (int i=0; i<data[fd]->GetNumberOfArrays(); i++)
            {
                oss << "<li>" << data[fd]->GetArray(i)->GetName() 
                    << "<ul>" 
                    << "<li>" 
                    << "refs = "  << data[fd]->GetArray(i)->GetReferenceCount()
                    << "</li><li>" 
                    << "vals = " << data[fd]->GetArray(i)->GetNumberOfTuples()
                    << "</li><li>" 
                    << "ncomps = " 
                    << data[fd]->GetArray(i)->GetNumberOfComponents()
                    << "</li></ul>"
                    << "</li>";
            }
            oss << "</ul>";
        }
    }
    
    
    
    SNPRINTF(str,strsize,oss.str().c_str());
    return str;
}

