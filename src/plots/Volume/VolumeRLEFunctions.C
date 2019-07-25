// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
