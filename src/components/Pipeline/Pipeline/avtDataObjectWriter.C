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
//                           avtDataObjectWriter.C                           //
// ************************************************************************* //

#include <avtDataObjectWriter.h>

#include <convert.h>

#include <avtDataObjectString.h>
#include <avtDataObjectInformation.h>
#include <AttributeGroup.h>
#include <BufferConnection.h>

#include <NoInputException.h>


// ****************************************************************************
//  Method: avtDataObjectWriter constructor
//
//  Programmer: Hank Childs
//  Creation:   December 29, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Thu Jul 26 12:40:06 PDT 2001
//    Made it derive from avtOriginatingSink.
//
//    Hank Childs, Mon Oct  1 09:08:13 PDT 2001
//    Removed initialization of dataset, image.
//
//    Mark C. Miller, Wed Dec 14 16:43:07 PST 2005
//    Added initialization of useCompression
//
// ****************************************************************************

avtDataObjectWriter::avtDataObjectWriter() 
{
    useCompression = false;
}


// ****************************************************************************
//  Method: avtDataObjectWriter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataObjectWriter::~avtDataObjectWriter()
{
    ;
}


// ****************************************************************************
//  Method: avtDataObjectWriter::Write
//
//  Purpose:
//      Starts a write for the derived types.  When the actual writing of the
//      data object starts, it uses a pure virtual function.
//
//  Arguments:
//      str       The data object string to write to.
//
//  Programmer:   Hank Childs
//  Creation:     October 1, 2001
//
// ****************************************************************************

void
avtDataObjectWriter::Write(avtDataObjectString &str)
{
    avtDataObject_p input = GetInput();

    if (*input == NULL)
    {
        EXCEPTION0(NoInputException);
    }

    //
    // Write out what type of data object this is so the reader will know what
    // type to instantiate.
    //
    const char *type = input->GetType();
    int len = strlen(type);
    WriteInt(str, len);
    str.Append((char *) type, len,
               avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);

    //
    // Write out the data object information.
    //
    avtDataObjectInformation &info = input->GetInfo();
    info.Write(str, this);

    //
    // Now let our derived types do the hard work.
    //
    DataObjectWrite(str);
}


// ****************************************************************************
//  Method: avtDataObjectWriter::SetDestinationFormat
//
//  Purpose:
//      Sets the destination format to use when writing the dataset.
//
//  Arguments:
//      dest    The new destination type representation.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 23 10:47:45 PDT 2000
//
// ****************************************************************************

void
avtDataObjectWriter::SetDestinationFormat(const TypeRepresentation &dest)
{
    destinationFormat = dest;
}


// ****************************************************************************
//  Method: avtDataObjectWriter::WriteInt
//
//  Purpose:
//      Writes a single integer to the data object string in the format of the
//      destination machine.
//
//  Arguments:
//      str     The data object string to write into.
//      data    The integer to write.
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2001
//
// *****************************************************************************

void
avtDataObjectWriter::WriteInt(avtDataObjectString &str, int data) const
{
    WriteInt(str, &data, 1);
}


// ****************************************************************************
//  Method: avtDataObjectWriter::WriteFloat
//
//  Purpose:
//      Writes a single float to the data object string in the format of the
//      destination machine.
//
//  Arguments:
//      str     The data object string to write into.
//      data    The float to write.
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2001
//
// *****************************************************************************

void
avtDataObjectWriter::WriteFloat(avtDataObjectString &str, float data) const
{
    WriteFloat(str, &data, 1);
}


// ****************************************************************************
//  Method: avtDataObjectWriter::WriteDouble
//
//  Purpose:
//      Writes a single double to the data object string in the format of the
//      destination machine.
//
//  Arguments:
//      str     The data object string to write into.
//      data    The double to write.
//
//  Programmer: Hank Childs
//  Creation:   September 19, 2001
//
// *****************************************************************************

void
avtDataObjectWriter::WriteDouble(avtDataObjectString &str, double data) const
{
    WriteDouble(str, &data, 1);
}


// ****************************************************************************
//  Method: avtDataObjectWriter::WriteInt
//
//  Purpose:
//      Writes an integer, in the format of the destination machine, to a 
//      string.
//
//  Arguments:
//      str     The data object string to write into.
//      data    The integers to write.
//      nData   The number of integers in data.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Oct 23 10:47:45 PDT 2000
//
//  Modifications:
//
//    Hank Childs, Sat May 26 09:02:06 PDT 2001
//    Changed arguments to be an avtDataObjectString to get around
//    string::append bottleneck.
//
//    Hank Childs, Sun Sep 16 17:30:43 PDT 2001
//    Reflect new interface for data object strings.
//
//    Hank Childs, Wed Sep 19 09:51:09 PDT 2001
//    Allowed for multiple integers.
//
// ****************************************************************************

void
avtDataObjectWriter::WriteInt(avtDataObjectString &str, const int *data,
                              int nData) const
{
    int nbytes = 0;

    if(sourceFormat.IntFormat == destinationFormat.IntFormat)
    {
        nbytes = sizeof(int);
        str.Append((char *)data, nbytes*nData,
                    avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    }
    else
    {
        unsigned char *buffer = new unsigned char[8*nData];
        for (int i = 0 ; i < nData ; i++)
        {
            int newAmount = IntConvert(data[i], buffer+nbytes, 
                                       destinationFormat.IntFormat);
            nbytes += newAmount;
        }
        str.Append((char *)buffer, nbytes,
            avtDataObjectString::DATA_OBJECT_STRING_OWNS_REFERENCE_AFTER_CALL);
    }
}


// ****************************************************************************
//  Method: avtDataObjectWriter::WriteFloat
//
//  Purpose:
//    Writes a float, in the format of the destination machine, to a string.
//
//  Arguments:
//    str        The data object string to write into.
//    data       The floats to write.
//    nData      The number of integers in data.
//
//  Programmer:  Hank Childs
//  Creation:    October 25, 2000
//
//  Modifications:
//
//    Hank Childs, Sat May 26 09:02:06 PDT 2001
//    Changed arguments to be an avtDataObjectString to get around
//    string::append bottleneck.
//
//    Hank Childs, Sun Sep 16 17:30:43 PDT 2001
//    Reflect new interface for data object strings.
//
//    Hank Childs, Wed Sep 19 09:51:09 PDT 2001
//    Allowed for multiple integers.
//
// ****************************************************************************

void
avtDataObjectWriter::WriteFloat(avtDataObjectString &str, const float *data,
                                int nData) const
{
    int nbytes = 0;

    if(sourceFormat.FloatFormat == destinationFormat.FloatFormat)
    {
        nbytes = sizeof(float);
        str.Append((char *)data, nbytes*nData,
                    avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    }
    else
    {
        unsigned char *buffer = new unsigned char[8*nData];
        for (int i = 0 ; i < nData ; i++)
        {
            int newAmount = FloatConvert(data[i], buffer+nbytes, 
                                         destinationFormat.FloatFormat);
            nbytes += newAmount;
        }
        str.Append((char *)buffer, nbytes,
            avtDataObjectString::DATA_OBJECT_STRING_OWNS_REFERENCE_AFTER_CALL);
    }
}


// ****************************************************************************
//  Method: avtDataObjectWriter::WriteDouble
//
//  Purpose:
//    Writes a double, in the format of the destination machine, to a string.
//
//  Arguments:
//    str        The data object string to write into.
//    data       The double to write.
//    nData      The number of integers in data.
//
//  Programmer:  Hank Childs
//  Creation:    March 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sat May 26 09:02:06 PDT 2001
//    Changed arguments to be an avtDataObjectString to get around
//    string::append bottleneck.
//
//    Hank Childs, Sun Sep 16 17:30:43 PDT 2001
//    Reflect new interface for data object strings.
//
//    Hank Childs, Wed Sep 19 09:51:09 PDT 2001
//    Allowed for multiple integers.
//
// ****************************************************************************

void
avtDataObjectWriter::WriteDouble(avtDataObjectString &str, const double *data,
                                 int nData) const
{
    int nbytes = 0;

    if(sourceFormat.DoubleFormat == destinationFormat.DoubleFormat)
    {
        nbytes = sizeof(double);
        str.Append((char *)data, nbytes*nData,
                    avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    }
    else
    {
        unsigned char *buffer = new unsigned char[8*nData];
        for (int i = 0 ; i < nData ; i++)
        {
            int newAmount = DoubleConvert(data[i], buffer+nbytes, 
                                          destinationFormat.DoubleFormat);
            nbytes += newAmount;
        }
        str.Append((char *)buffer, nbytes,
            avtDataObjectString::DATA_OBJECT_STRING_OWNS_REFERENCE_AFTER_CALL);
    }
}


// ****************************************************************************
//  Method: avtDataObjectWriter::WriteAtts
//
//  Purpose:
//    Writes an AttributeGroup, in the format of the destination machine, 
//    to a string.
//
//  Arguments:
//    str        The data object string to write into.
//    atts       The AttributeGroup to write.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 27, 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtDataObjectWriter::WriteAtts(avtDataObjectString &str, 
                               AttributeGroup *atts) const
{
    if (atts == NULL)
    {
        WriteInt(str, 0);
        return;
    }

    BufferConnection buf;
    buf.SetDestinationFormat(destinationFormat);

    atts->SelectAll();
    atts->Write(buf);
    int size = atts->CalculateMessageSize(buf);

    unsigned char *b1 = new unsigned char[size];
    char *b2 = new char[size];
    for (int i = 0; i < size; i++)
    {
        buf.Read(b1+i);
        b2[i] = (char)b1[i];
    }
    
    WriteInt(str, size);
    str.Append((char*) b2, size,
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
    delete [] b1;
    delete [] b2;
}


