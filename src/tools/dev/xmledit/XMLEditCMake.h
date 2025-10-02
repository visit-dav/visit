// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITCMAKE_H
#define XMLEDITCMAKE_H

#include <QFrame>

class XMLDocument;
class QLineEdit;
class QCheckBox;

// ****************************************************************************
//  Class:  XMLEditCMake
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
//    Kathleen Biagas, Fri Mar 21, 2025
//    Removed QString arg from xxxTextChanged slots as they are now connected
//    to 'editingFinished' signal.
//
// ****************************************************************************
class XMLEditCMake : public QFrame
{
    Q_OBJECT
  public:
    XMLEditCMake(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void cxxflagsTextChanged();
    void ldflagsTextChanged();
    void libsTextChanged();
    void defsTextChanged();
    void gfilesTextChanged();
    void glibsTextChanged();
    void sfilesTextChanged();
    void vfilesTextChanged();
    void vlibsTextChanged();
    void mfilesTextChanged();
    void mlibsTextChanged();
    void efilesTextChanged();
    void elibsSerTextChanged();
    void elibsParTextChanged();
    void wfilesTextChanged();
    void vwfilesTextChanged();
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

    void mcxxflagsTextChanged();
    void mldflagsTextChanged();
    void mdefsTextChanged();

    void ecxxflagsSerTextChanged();
    void eldflagsSerTextChanged();
    void edefsSerTextChanged();

    void ecxxflagsParTextChanged();
    void eldflagsParTextChanged();
    void edefsParTextChanged();

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
