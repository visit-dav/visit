/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <DSRObject.h>
#include <stdio.h>
#include <DebugStream.h>

// ****************************************************************************
// Function: reverse_endian
//
// Purpose:
//   Reverses the endian of the item that was passed in and returns the
//   reversed data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 24 17:01:30 PST 2003
//
// Modifications:
//   
// ****************************************************************************

template <class T>
T
reverse_endian(const T &data)
{
    T retval;

    // Rearrange the bytes.
    unsigned char *dest = (unsigned char *)&retval;
    unsigned char *src = (unsigned char *)&data + sizeof(T) - 1;
    for(int i = 0; i < sizeof(T); ++i)
        *dest++ = *src--;

    return retval;
}

// ****************************************************************************
// Method: DSRObject::DSRObject
//
// Purpose: 
//   Constructor for the DSRObject class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 25 12:29:23 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

DSRObject::DSRObject()
{
    unsigned char *cptr = (unsigned char *)&data;
    for(int i = 0; i < sizeof(dsr); ++i)
        *cptr++ = 0;
    reverseEndian = false;
}

// ****************************************************************************
// Method: DSRObject::~DSRObject
//
// Purpose: 
//   Destructor for the DSRObject class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 25 12:29:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

DSRObject::~DSRObject()
{
}

// ****************************************************************************
// Method: DSRObject::PopulateFromFile
//
// Purpose: 
//   Reads the dsr object from the header file.
//
// Arguments:
//   filename : The name of the header file to open.
//
// Returns:    True if successful; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 25 12:30:35 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
DSRObject::PopulateFromFile(const char *filename)
{
    bool retval = false;
    FILE *fp;

    if((fp = fopen(filename, "rb")) != 0)
    {
        int nbytes = int(fread((void *) &data, 1, sizeof(dsr), fp));
        debug1 << nbytes << " bytes read from file " << filename << endl;
        fclose(fp);

        // Reverse the endian representation if it is backwards.
        ReverseEndians();

        retval = true;
    }
    else
    {
        debug1 << "Could not open " << filename << endl;
    }

    return retval;
}

// ****************************************************************************
// Method: DSRObject::ReversedEndian
//
// Purpose: 
//   Returns whether we had to reverse the endian representation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 25 12:33:25 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
DSRObject::ReversedEndian() const
{
    return reverseEndian;
}

// ****************************************************************************
// Method: DSRObject::ReverseEndians
//
// Purpose: 
//   Reverses the multi-byte fields in the structs if they need to be
//   reversed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 25 12:34:13 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
DSRObject::ReverseEndians()
{
    //
    // If the int size of the header is not 348, assume that the data was
    // written in a reverse endian format that we have to reverse when we
    // read it in.
    //
    if(data.hk.sizeof_hdr != 348)
    {
        reverseEndian = true;
        data.hk.sizeof_hdr = reverse_endian(data.hk.sizeof_hdr);
        data.hk.extents = reverse_endian(data.hk.extents);
        data.hk.session_error = reverse_endian(data.hk.session_error);

        int i;
        for(i = 0; i < 8; ++i)
            data.dime.dim[i] = reverse_endian(data.dime.dim[i]);
        data.dime.unused1 = reverse_endian(data.dime.unused1);
        data.dime.datatype = reverse_endian(data.dime.datatype);
        data.dime.bitpix = reverse_endian(data.dime.bitpix);
        data.dime.dim_un0 = reverse_endian(data.dime.dim_un0);
        data.dime.bitpix = reverse_endian(data.dime.bitpix);
        for(i = 0; i < 8; ++i)
            data.dime.pixdim[i] = reverse_endian(data.dime.pixdim[i]);
        data.dime.vox_offset = reverse_endian(data.dime.vox_offset);
        data.dime.funused1 = reverse_endian(data.dime.funused1);
        data.dime.funused2 = reverse_endian(data.dime.funused2);
        data.dime.funused3 = reverse_endian(data.dime.funused3);
        data.dime.cal_max = reverse_endian(data.dime.cal_max);
        data.dime.cal_min = reverse_endian(data.dime.cal_min);
        data.dime.compressed = reverse_endian(data.dime.compressed);
        data.dime.verified = reverse_endian(data.dime.verified);
        data.dime.glmax = reverse_endian(data.dime.glmax);
        data.dime.glmin = reverse_endian(data.dime.glmin);

        data.hist.views = reverse_endian(data.hist.views);
        data.hist.vols_added = reverse_endian(data.hist.vols_added);
        data.hist.start_field = reverse_endian(data.hist.start_field);
        data.hist.field_skip = reverse_endian(data.hist.field_skip);
        data.hist.omax = reverse_endian(data.hist.omax);
        data.hist.omin = reverse_endian(data.hist.omin);
        data.hist.smax = reverse_endian(data.hist.smax);
        data.hist.smin = reverse_endian(data.hist.smin);
    }
}
