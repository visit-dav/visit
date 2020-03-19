// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef TYPEREPRESENTATION_H
#define TYPEREPRESENTATION_H
#include <comm_exports.h>

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
//   
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

    int CharSize() const;
    int IntSize() const;
    int LongSize() const;
    int FloatSize() const;
    int DoubleSize() const;
    SupportedFormat GetSupportedFormat() const;
    void SetSupportedFormat(SupportedFormat format);

    unsigned char Format; /// toggle between ascii and binary..
    unsigned char IntFormat;
    unsigned char LongFormat;
    unsigned char FloatFormat;
    unsigned char DoubleFormat;
};

#endif
