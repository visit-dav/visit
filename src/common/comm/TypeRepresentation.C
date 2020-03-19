// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
    Format = BINARYFORMAT; /// default is binary..

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
    Format = copy.Format;
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
    Format = obj.Format;
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
    if(Format != obj.Format) /// if types do not match
        return false;

    if(static_cast<SupportedFormat>(Format) == ASCIIFORMAT)
        return true;

    return (IntFormat  == obj.IntFormat) &&
           (LongFormat == obj.LongFormat) &&
           (FloatFormat == obj.FloatFormat) &&
           (DoubleFormat == obj.DoubleFormat);
}

//
// Methods to return the size of the datatypes based on their format.
//

int
TypeRepresentation::CharSize() const
{
    return 1;
}

int
TypeRepresentation::IntSize() const
{
    return (IntFormat == B32 || IntFormat == L32) ? 4 : 8;
}

int
TypeRepresentation::LongSize() const
{
    return (LongFormat == B32 || LongFormat == L32) ? 4 : 8;
}

int
TypeRepresentation::FloatSize() const
{
    return (FloatFormat == B32 || FloatFormat == L32) ? 4 : 8;
}

int   
TypeRepresentation::DoubleSize() const
{
    return (DoubleFormat == B32 || DoubleFormat == L32) ? 4 : 8;
}


TypeRepresentation::SupportedFormat
TypeRepresentation::GetSupportedFormat() const
{
    return static_cast<SupportedFormat>(Format);
}

void
TypeRepresentation::SetSupportedFormat(SupportedFormat _Format)
{
    Format = _Format;
}

