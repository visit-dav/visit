// ************************************************************************* //
//                             avtDataObjectReader.C                         //
// ************************************************************************* //

#include <string>
#include <avtDataObjectReader.h>

#include <avtDataSetReader.h>
#include <avtImageReader.h>
#include <avtNullDataReader.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>


// ****************************************************************************
//  Method: avtDataObjectReader constructor
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//      Mark C. Miller, Wed Jan  8 11:43:44 PST 2003
//         Initialized boolean data members for type of object
// ****************************************************************************

avtDataObjectReader::avtDataObjectReader()
{
    datasetReader  = new avtDataSetReader;
    imageReader    = new avtImageReader;
    nullDataReader = new avtNullDataReader;
    haveInput      = false;
    datasetInput   = false;
    imageInput     = false;
    nullDataInput  = false;
}


// ****************************************************************************
//  Method: avtDataObjectReader::InputIsDataset
//
//  Purpose:
//      Shows if the input is a dataset or not 
//
//  Returns:    true if the input is a dataset, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// ****************************************************************************

bool
avtDataObjectReader::InputIsDataset(void)
{
    if (!haveInput)
    {
        EXCEPTION0(NoInputException);
    }

    return datasetInput;
}

// ****************************************************************************
//  Method: avtDataObjectReader::InputIsImage
//
//  Purpose:
//      Shows if the input is an image or not 
//
//  Returns:    true if the input is an image, false otherwise.
//
//  Programmer: Mark C. Miller
//  Creation:   January 8, 2003 
//
// ****************************************************************************

bool
avtDataObjectReader::InputIsImage(void)
{
    if (!haveInput)
    {
        EXCEPTION0(NoInputException);
    }

    return imageInput;
}

// ****************************************************************************
//  Method: avtDataObjectReader::InputIsNullData
//
//  Purpose:
//      Shows if the input is null or not
//
//  Returns:    true if the input is null, false otherwise.
//
//  Programmer: Mark C. Miller 
//  Creation:   January 8, 2003 
//
// ****************************************************************************

bool
avtDataObjectReader::InputIsNullData(void)
{
    if (!haveInput)
    {
        EXCEPTION0(NoInputException);
    }

    return nullDataInput;
}


// ****************************************************************************
//  Method: avtDataObjectReader::InputIs
//
//  Purpose:
//      Shows if the input type string is equal to given string 
//
//  Returns:    true if the input string and given string match 
//
//  Programmer: Mark C. Miller 
//  Creation:   January 8, 2003 
//
// ****************************************************************************

bool
avtDataObjectReader::InputIs(const char *givenStr)
{
    if (!haveInput)
    {
        EXCEPTION0(NoInputException);
    }

    return !strcmp(inputTypeStr.c_str(),givenStr); 
}

// ****************************************************************************
//  Method: avtDataObjectReader::GetImageOutput
//
//  Purpose:
//      Gets the output of the image reader.
//
//  Returns:    An avtImage.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
// **************************************************************************** 

avtImage_p
avtDataObjectReader::GetImageOutput(void)
{
    if (!InputIsImage())
    {
        EXCEPTION0(ImproperUseException);
    }

    return imageReader->GetTypedOutput();
}


// ****************************************************************************
//  Method: avtDataObjectReader::GetDatasetOutput
//
//  Purpose:
//      Gets the output of the dataset reader.
//
//  Returns:     An avtDataset.
//
//  Programmer:  Hank Childs
//  Creation:    December 28, 2000
//
// ****************************************************************************

avtDataset_p
avtDataObjectReader::GetDatasetOutput(void)
{
    if (!InputIsDataset())
    {
        EXCEPTION0(ImproperUseException);
    }

    return datasetReader->GetTypedOutput();
}

// ****************************************************************************
//  Method: avtDataObjectReader::GetNullDataOutput
//
//  Purpose:
//      Gets the output of the dataset reader.
//
//  Returns:     An avtNullData.
//
//  Programmer:  Mark C. Miller 
//  Creation:    January 8, 2003
//
// ****************************************************************************

avtNullData_p
avtDataObjectReader::GetNullDataOutput(void)
{
    if (!InputIsNullData())
    {
        EXCEPTION0(ImproperUseException);
    }

    return nullDataReader->GetTypedOutput();
}


// ****************************************************************************
//  Method: avtDataObjectReader::GetOutput
//
//  Purpose:
//      Gets the output of the appropriate reader.
//
//  Programmer: Hank Childs
//  Creation:   October 1, 2001
//
//  Modifications
//     Mark C. Miller, Wed Jan  8 11:47:31 PST 2003
//     Added support for null data object
//
// ****************************************************************************

avtDataObject_p
avtDataObjectReader::GetOutput(void)
{
    if (!haveInput)
    {
        EXCEPTION0(NoInputException);
    }

    if      (InputIsDataset())  return datasetReader->GetOutput();
    else if (InputIsImage())    return imageReader->GetOutput();
    else if (InputIsNullData()) return nullDataReader->GetOutput();
    else                       EXCEPTION0(NoInputException);
}


// ****************************************************************************
//  Method: avtDataObjectReader::GetInfo
//
//  Purpose:
//      Allows for the data object information to be read in without having
//      to worry about what data object type the reader has.
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2000
//
//  Modifications
//     Mark C. Miller, Wed Jan  8 11:47:31 PST 2003
//     Added support for null data object
//
// ****************************************************************************

avtDataObjectInformation &
avtDataObjectReader::GetInfo(void)
{
    if (!haveInput)
    {
        EXCEPTION0(NoInputException);
    }

    if      (InputIsDataset())  return datasetReader->GetOutput()->GetInfo();
    else if (InputIsImage())    return imageReader->GetOutput()->GetInfo();
    else if (InputIsNullData()) return nullDataReader->GetOutput()->GetInfo();
    else                       EXCEPTION0(NoInputException);

}


// ****************************************************************************
//  Method: avtDataObjectReader::Read
//
//  Purpose:
//      Reads from the character string and determines what the input type is,
//      then goes to the proper reader and lets it read it.
//
//  Arguments:
//      size    The size of the string output.
//      input   A character string containing information about the data and
//              the data itself.
//
//  Programmer: Hank Childs
//  Creation:   December 28, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Sep 17 10:55:44 PDT 2001
//    Made use of character string references to prevent unnecessary copies.
//
//    Hank Childs, Wed Sep 19 14:30:45 PDT 2001
//    Put reading of data object info into one spot for images and datasets.
//
//    Hank Childs, Mon Oct  1 09:34:22 PDT 2001
//    Reflect new way of distinguishing between data objects.
//
//    Hank Childs, Mon Jan  7 16:47:28 PST 2002
//    Fix memory leak.
//
// ****************************************************************************

void
avtDataObjectReader::Read(int size, char *input)
{
    //
    // Derived types keep track of this so they don't have to copy it.
    //
    CharStrRef originalString = input;

    int  strLen;
    memcpy(&strLen, input, sizeof(int));
    input += sizeof(int); size -= sizeof(int);
    char *dataObjectType = new char[strLen+1];
    int toRead = sizeof(char)*strLen;
    memcpy(dataObjectType, input, toRead);
    input += toRead; size -= toRead;
    dataObjectType[strLen] = '\0';

    //
    // The data object information in the dataset is not valid yet, so
    // lets use our own and copy it over later.
    //
    avtDataObjectInformation info;
    int  infoSize = info.Read(input);
    input += infoSize;
    size  -= infoSize;

    if (strcmp(dataObjectType, "avtDataset") == 0)
    {
        //
        // We have a dataset to read in.
        //
        datasetInput  = true;
        imageInput    = false;
        nullDataInput = false;

        int datasetSize = datasetReader->Read(input, originalString);
        input += datasetSize;
        size  -= datasetSize;

        datasetReader->GetOutput()->GetInfo().Copy(info);
    }
    else if (strcmp(dataObjectType, "avtImage") == 0)
    {
        //
        // We have an image to read in.
        //
        datasetInput  = false;
        imageInput    = true; 
        nullDataInput = false;

        int imageSize = imageReader->Read(input);
        input += imageSize;
        size  -= imageSize;

        imageReader->GetOutput()->GetInfo().Copy(info);
    }
    else if ((strcmp(dataObjectType, "avtNullData") == 0) ||
             (strcmp(dataObjectType, AVT_NULL_IMAGE_MSG) == 0) ||
             (strcmp(dataObjectType, AVT_NULL_DATASET_MSG) == 0))
    {
        datasetInput  = false;
        imageInput    = false; 
        nullDataInput = true;

        int nullSize = nullDataReader->Read(input);
        input += nullSize;
        size  -= nullSize;

        nullDataReader->GetOutput()->GetInfo().Copy(info);
    }
    else
    {
        debug1 << "Unknown data object type " << "\"" << dataObjectType
               << "\"" << endl;
        EXCEPTION0(ImproperUseException);
    }

    inputTypeStr = std::string(dataObjectType);
    delete [] dataObjectType;

    if (size != 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    haveInput = true;
}


