// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QUALIFIED_FILENAME_H
#define QUALIFIED_FILENAME_H
#include <string>
#include <vector>

// ****************************************************************************
// Class: QualifiedFilename
//
// Purpose:
//   This class contains filename information that includes: host,
//   path, filename. It should be used anywhere a filename is needed.
//
// Notes:      
//   This class was created to deprecate the usage of strings as
//   filenames. This is because it was often unclear how the file
//   information was stored in the string. This way, we always know
//   what is in a filename.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 12 14:31:16 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Aug 26 17:04:06 PST 2002
//   I added a separator.
//
//   Brad Whitlock, Fri Mar 28 12:01:51 PDT 2003
//   I added a flag that contains the file's access and virtual bits.
//
// ****************************************************************************

struct QualifiedFilename
{
    QualifiedFilename();
    QualifiedFilename(std::string host_, std::string path_,
                      std::string filename_, bool isVirtual = false);
    QualifiedFilename(const std::string &fullname);
    QualifiedFilename(const QualifiedFilename &qf);
    ~QualifiedFilename();
    void operator = (const QualifiedFilename &qf);
    bool operator == (const QualifiedFilename &qf) const;
    bool operator < (const QualifiedFilename &qf) const;
    bool operator > (const QualifiedFilename &qf) const;
    bool operator != (const QualifiedFilename &qf) const;
    bool Empty() const;

    void SetFromString(const std::string &str);
    std::string FullName() const;
    std::string PathAndFile() const;

    bool IsVirtual() const    { return (flag & 1) == 1; }
    void SetVirtual(bool val) { flag = ((flag & 0xfe) | (val?1:0)); }

    std::string host;
    std::string path;
    std::string filename;
    char        separator;
private:
    char DetermineSeparator(const std::string &p) const;

    // Virtual is bit 0
    unsigned char flag;
};

typedef std::vector<QualifiedFilename> QualifiedFilenameVector;

// Function to combine two QualifiedFilenameVector so that there are no
// duplicate filenames.
QualifiedFilenameVector
CombineQualifiedFilenameVectors(const QualifiedFilenameVector &a,
    const QualifiedFilenameVector &b);

#endif
