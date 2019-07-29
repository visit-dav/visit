// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef NAME_SIMPLIFIER_H
#define NAME_SIMPLIFIER_H

#include <gui_exports.h>
#include <QualifiedFilename.h>
#include <vectortypes.h>

// ****************************************************************************
// Class: NameSimplifier
//
// Purpose:
//   This is a simple class that takes a group of host qualified file names
//   and removes the bits that it can to make shorter names.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 4 14:22:58 PST 2004
//
// Modifications:
//    Jeremy Meredith, Tue Apr  6 14:16:54 PDT 2004
//    Keep track of the filenames in a way made for adding the minimal amount
//    of text needed for ensuring they are unique.  Added UniqueFileName.
//
//    Jeremy Meredith, Wed Apr  7 12:12:48 PDT 2004
//    Account for the fact that not all input names had full paths.
//
// ****************************************************************************

class GUI_API NameSimplifier
{
  private:
    // Class: UniqueFileName
    //
    // Purpose:
    //    Structure to add a minimal amount of path/host information
    //    to a filename to ensure it is unique w.r.t. other filenames
    class UniqueFileName
    {
      public:
        UniqueFileName(const QualifiedFilename &qfn);
        std::string GetAsString() const;

        static bool Unique(const UniqueFileName &a, const UniqueFileName &b);
        static void Uniquify(UniqueFileName &a, UniqueFileName &b);
      private:
        std::string  host;        // host name
        stringVector path;        // path components, split by separator
        int          pathLen;     // length of the 'path' vector
        std::string  file;        // raw file name without path
        bool         leadingSlash;// true if it was a full path originally
        char         separator;   // separator character

        bool         useHost;     // true if we need the host for uniqueness
        int          pathCount;   // number of needed path segment prefixes
    };

  public:
    NameSimplifier();
    ~NameSimplifier();

    void AddName(const std::string &n);
    void AddName(const QualifiedFilename &n);
    void ClearNames();
    void GetSimplifiedNames(stringVector &n) const;
private:
    std::vector<UniqueFileName> names;
};

#endif
