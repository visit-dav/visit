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
#include <VolumeRLEFunctions.h>

//
// These functions are adapted from RGB image RLE encoding, which RLE encodes
// a channel at a time. These functions are used to RLE encode single channel
// images stored in unsignedCharVector.
//
 
// ****************************************************************************
// Method: VolumeRLECompress
//
// Purpose: 
//   RLE compresses a buffer.
//
// Arguments:
//   in  : The buffer to compress.
//   out : The vector that holds the compressed data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan  8 14:25:34 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
VolumeRLECompress(const unsignedCharVector &in, unsignedCharVector &out)
{
    const unsigned char *iptr = &in[0];
    const unsigned char *ibufend = iptr + in.size();
     
    while(iptr < ibufend)
    {
        short         todo, cc;
        int           count;
        const unsigned char *sptr = iptr;
        iptr += 2;
        while((iptr<ibufend)&&((iptr[-2]!=iptr[-1])||(iptr[-1]!=iptr[0])))
            iptr++;
        iptr -= 2;
        count = iptr - sptr;
        while(count)
        {
            todo = (count > 126) ? 126 : count;
            count -= todo;
            out.push_back(0x80 | todo);
            while(todo--)
               out.push_back(*sptr++);
        }
        sptr = iptr;
        cc = *iptr++;
        while((iptr < ibufend) && (*iptr == cc)) 
            iptr++;
        count = iptr-sptr;
        while(count)
        {
            todo = (count > 126) ? 126 : count;
            count -= todo;
            out.push_back(todo);
            out.push_back(cc);
        }
    }
    out.push_back(0);
}

// ****************************************************************************
// Method: VolumeRLEDecompress
//
// Purpose: 
//   RLE decompresses a buffer.
//
// Arguments:
//   in  : The buffer to decompress.
//   out : The vector that holds the decompressed data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan  8 14:25:34 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void 
VolumeRLEDecompress(const unsignedCharVector &in, unsignedCharVector &out) 
{
    if(in.size() < 1)
        return;
    
    unsigned char count;
    const unsigned char *iptr = &in[0];
    for(;;)
    {
        unsigned char value = *iptr++;
        if(! (count = (value & 0x7f)) )
            return;

        if(value & 0x80)
        {
            while(count--)
               out.push_back(*iptr++);
        }
        else
        {
            /* decompress RLE span */
            value = *iptr++;
            while(count--)
                out.push_back(value);
        }        
    }
}
