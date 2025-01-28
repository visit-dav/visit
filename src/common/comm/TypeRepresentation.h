// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef TYPEREPRESENTATION_H
#define TYPEREPRESENTATION_H
#include <comm_exports.h>
#include <cstddef>

// *******************************************************************
// Class: TypeRepresentation
//
// Purpose:
//   This class contains information about a machine's data type
//   sizes and endianess.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 12:36:08 PDT 2000
//
// Modifications:
//     Cyrus Harrison, Tue Jan 28 09:55:51 PST 2025
//     Convert to size_t for buffer sizes
// *******************************************************************

class COMM_API TypeRepresentation
{
public:
    enum SupportedFormat { ASCIIFORMAT,BINARYFORMAT };
    TypeRepresentation();
    TypeRepresentation(const TypeRepresentation &);
    ~TypeRepresentation();
    void operator = (const TypeRepresentation &);
    bool operator == (const TypeRepresentation &);

    size_t CharSize() const;
    size_t IntSize() const;
    size_t LongSize() const;
    size_t FloatSize() const;
    size_t DoubleSize() const;
    SupportedFormat GetSupportedFormat() const;
    void SetSupportedFormat(SupportedFormat format);

    unsigned char Format; /// toggle between ascii and binary..
    unsigned char IntFormat;
    unsigned char LongFormat;
    unsigned char FloatFormat;
    unsigned char DoubleFormat;
};

#endif
