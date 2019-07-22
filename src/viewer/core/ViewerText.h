// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_TEXT_H
#define VIEWER_TEXT_H
#include <viewercore_exports.h>
#include <iostream>
#include <string>
#include <vector>
#include <visit-config.h>

// ****************************************************************************
// Class: ViewerText
//
// Purpose:
//   String class for viewer status/warning/error messages.
//
// Notes:    This doesn't do *everything* a proper string class does -- just
//           commonly used stuff.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 17:09:16 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerText
{
public:
    ViewerText();
    ViewerText(const char *s);
    ViewerText(const std::string &s);
    ViewerText(const ViewerText &obj);
    ~ViewerText();

    bool empty() const;
    size_t size() const;

    ViewerText operator = (const char *obj);
    ViewerText operator = (const std::string &obj);
    ViewerText operator = (const ViewerText &obj);

    ViewerText arg(char   val);
    ViewerText arg(int    val);
    ViewerText arg(unsigned int val);
    ViewerText arg(long   val);
#if (SIZEOF_UNSIGNED_INT != SIZEOF_SIZE_T)
    ViewerText arg(size_t val);
#endif
    ViewerText arg(float  val);
    ViewerText arg(double val);
    ViewerText arg(const std::string &val);

    std::string toStdString() const;

    ViewerText append(const ViewerText &obj);
    ViewerText operator += (const ViewerText &obj);
    ViewerText operator + (const ViewerText &obj) const;
    ViewerText operator + (const std::string &obj) const;

    const std::string &GetText() const;
    const std::vector<std::string> &GetArguments() const;
private:
    std::string              text;
    std::vector<std::string> args;
};

// Use this decoration function to denote a string that needs translation.
ViewerText VIEWERCORE_API TR(const char *s);

#endif
