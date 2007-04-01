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
//   
// ****************************************************************************

class GUI_API NameSimplifier
{
public:
    NameSimplifier();
    ~NameSimplifier();

    void AddName(const std::string &n);
    void AddName(const QualifiedFilename &n);
    void ClearNames();
    void GetSimplifiedNames(stringVector &n) const;
private:
    QualifiedFilenameVector names;
};

#endif
