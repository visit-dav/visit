/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <visit-config.h>
#include <convert.h>
#include <TypeRepresentation.h>

// *******************************************************************
// Method: TypeRepresentation::TypeRepresentation
//
// Purpose: 
//   Constructor for the TypeRepresentation class.
//
// Note:       
//   The purpose of this constructor is to fill the values with the
//   relevant type information for the host platform.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 12:38:48 PDT 2000
//
// Modifications:
//   
// *******************************************************************

TypeRepresentation::TypeRepresentation()
{
    // Set the int format
#if(SIZEOF_INT == 4)
#ifdef WORDS_BIGENDIAN
    IntFormat = B32;
#else
    IntFormat = L32;
#endif
#elif(SIZEOF_INT == 8)
#ifdef WORDS_BIGENDIAN
    IntFormat = B64;
#else
    IntFormat = L64;
#endif
#else
#pragma error "Unsupported int size"
#endif

    // Set the long format
#if(SIZEOF_LONG == 4)
#ifdef WORDS_BIGENDIAN
    LongFormat = B32;
#else
    LongFormat = L32;
#endif
#elif(SIZEOF_LONG == 8)
#ifdef WORDS_BIGENDIAN
    LongFormat = B64;
#else
    LongFormat = L64;
#endif
#else
#pragma error "Unsupported long size"
#endif

    // Set the float format
#if(SIZEOF_FLOAT == 4)
#ifdef WORDS_BIGENDIAN
    FloatFormat = B32;
#else
    FloatFormat = L32;
#endif
#else
#pragma error "Unsupported float size"
#endif

    // Set the double format
#if(SIZEOF_DOUBLE == 8)
#ifdef WORDS_BIGENDIAN
    DoubleFormat = B64;
#else
    DoubleFormat = L64;
#endif
#else
#pragma error "Unsupported double size"
#endif
}

// *******************************************************************
// Method: TypeRepresentation::TypeRepresentation
//
// Purpose: 
//   Copy constructor for the TypeRepresentation class.
//
// Arguments:
//   copy : The object to copy into the current object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 12:41:04 PDT 2000
//
// Modifications:
//   
// *******************************************************************

TypeRepresentation::TypeRepresentation(const TypeRepresentation &copy)
{
    IntFormat  = copy.IntFormat;
    LongFormat = copy.LongFormat;
    FloatFormat = copy.FloatFormat;
    DoubleFormat = copy.DoubleFormat;
}

// *******************************************************************
// Method: TypeRepresentation::~TypeRepresentation
//
// Purpose: 
//   Destructor for the TypeRepresentation class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 12:41:42 PDT 2000
//
// Modifications:
//   
// *******************************************************************

TypeRepresentation::~TypeRepresentation()
{
    // nothing
}

// *******************************************************************
// Method: TypeRepresentation::operator =
//
// Purpose: 
//   Assignment operator for the TypeRepresentation class.
//
// Arguments:
//   obj : The object to copy into the current object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 12:41:04 PDT 2000
//
// Modifications:
//   
// *******************************************************************
void
TypeRepresentation::operator = (const TypeRepresentation &obj)
{
    IntFormat  = obj.IntFormat;
    LongFormat = obj.LongFormat;
    FloatFormat = obj.FloatFormat;
    DoubleFormat = obj.DoubleFormat;
}

// *******************************************************************
// Method: TypeRepresentation::operator ==
//
// Purpose: 
//   Equals operator for the TypeRepresentation class.
//
// Arguments:
//   obj : The object to compare to the current object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 12:41:04 PDT 2000
//
// Modifications:
//   
// *******************************************************************

bool
TypeRepresentation::operator == (const TypeRepresentation &obj)
{
    return (IntFormat  == obj.IntFormat) &&
           (LongFormat == obj.LongFormat) &&
           (FloatFormat == obj.FloatFormat) &&
           (DoubleFormat == obj.DoubleFormat);
}

//
// Methods to return the size of the datatypes based on their format.
//

int
TypeRepresentation::CharSize()
{
    return 1;
}

int
TypeRepresentation::IntSize()
{
    return (IntFormat == B32 || IntFormat == L32) ? 4 : 8;
}

int
TypeRepresentation::LongSize()
{
    return (LongFormat == B32 || LongFormat == L32) ? 4 : 8;
}

int
TypeRepresentation::FloatSize()
{
    return (FloatFormat == B32 || FloatFormat == L32) ? 4 : 8;
}

int   
TypeRepresentation::DoubleSize()
{
    return (DoubleFormat == B32 || DoubleFormat == L32) ? 4 : 8;
}
