// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITMAKEFILE_H
#define XMLEDITMAKEFILE_H

#include <QFrame>

class XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;

// ****************************************************************************
//  Class:  XMLEditMakefile
//
//  Purpose:
//    Makefile editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Jul  7 17:08:03 PDT 2004
//    Allow for mdserver-specific code in a plugin's source files.
//
//    Brad Whitlock, Fri Feb 23 17:47:21 PST 2007
//    Added viewer widgets.
//
//    Cyrus Harrison, Wed Mar  7 09:17:11 PST 2007
//    Allow for engine-specific code in a plugin's source files.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Cyrus Harrison, Fri Sep 19 13:58:39 PDT 2008
//    Added support for custom libs for gui,engine,mdserver, and viewer libs.
//
//    Jeremy Meredith, Tue Sep  8 15:11:35 EDT 2009
//    Split custom engine libs into serial and parallel versions.
//
//    Kathleen Biagas, Thu Nov  6 11:18:22 PST 2014
//    Added DEFINES.
//
//    Kathleen Biagas, Wed May 4, 2022
//    Added support for component-specific DEFINES, CXXFLAGS, and LDFLAGS.
//
// ****************************************************************************
class XMLEditMakefile : public QFrame
{
    Q_OBJECT
  public:
    XMLEditMakefile(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void cxxflagsTextChanged(const QString&);
    void ldflagsTextChanged(const QString&);
    void libsTextChanged(const QString&);
    void defsTextChanged(const QString&);
    void gfilesTextChanged(const QString&);
    void glibsTextChanged(const QString&);
    void sfilesTextChanged(const QString&);
    void vfilesTextChanged(const QString&);
    void vlibsTextChanged(const QString&);
    void mfilesTextChanged(const QString&);
    void mlibsTextChanged(const QString&);
    void efilesTextChanged(const QString&);
    void elibsSerTextChanged(const QString&);
    void elibsParTextChanged(const QString&);
    void wfilesTextChanged(const QString&);
    void vwfilesTextChanged(const QString&);
    void customgfilesChanged();
    void customglibsChanged();
    void customsfilesChanged();
    void customvfilesChanged();
    void customvlibsChanged();
    void custommfilesChanged();
    void custommlibsChanged();
    void customefilesChanged();
    void customelibsSerChanged();
    void customelibsParChanged();
    void customwfilesChanged();
    void customvwfilesChanged();
    void mdSpecificCodeChanged();
    void engSpecificCodeChanged();

    void mcxxflagsTextChanged(const QString&);
    void mldflagsTextChanged(const QString&);
    void mdefsTextChanged(const QString&);

    void ecxxflagsSerTextChanged(const QString&);
    void eldflagsSerTextChanged(const QString&);
    void edefsSerTextChanged(const QString&);

    void ecxxflagsParTextChanged(const QString&);
    void eldflagsParTextChanged(const QString&);
    void edefsParTextChanged(const QString&);

  private:
    XMLDocument     *xmldoc;

    QLineEdit       *CXXFLAGS;
    QLineEdit       *LDFLAGS;
    QLineEdit       *LIBS;
    QLineEdit       *DEFINES;
    QCheckBox       *customGFiles;
    QLineEdit       *GFiles;
    QCheckBox       *customSFiles;
    QLineEdit       *SFiles;
    QCheckBox       *customVFiles;
    QLineEdit       *VFiles;
    QCheckBox       *customMFiles;
    QLineEdit       *MFiles;
    QCheckBox       *customEFiles;
    QLineEdit       *EFiles;
    QCheckBox       *customWFiles;
    QLineEdit       *WFiles;
    QCheckBox       *customVWFiles;
    QLineEdit       *VWFiles;
    QCheckBox       *customGLibs;
    QLineEdit       *GLibs;
    QCheckBox       *customELibsSer;
    QLineEdit       *ELibsSer;
    QCheckBox       *customELibsPar;
    QLineEdit       *ELibsPar;
    QCheckBox       *customMLibs;
    QLineEdit       *MLibs;
    QCheckBox       *customVLibs;
    QLineEdit       *VLibs;

    QLineEdit       *MCXXFlags;
    QLineEdit       *MLDFlags;
    QLineEdit       *MDefines;

    QLineEdit       *ECXXFlagsSer;
    QLineEdit       *ELDFlagsSer;
    QLineEdit       *EDefinesSer;

    QLineEdit       *ECXXFlagsPar;
    QLineEdit       *ELDFlagsPar;
    QLineEdit       *EDefinesPar;

    QCheckBox       *mdSpecificCode;
    QCheckBox       *engSpecificCode;
};

#endif
