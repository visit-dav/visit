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
        string GetAsString() const;

        static bool Unique(const UniqueFileName &a, const UniqueFileName &b);
        static void Uniquify(UniqueFileName &a, UniqueFileName &b);
      private:
        string       host;        // host name
        stringVector path;        // path components, split by separator
        int          pathLen;     // length of the 'path' vector
        string       file;        // raw file name without path
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
