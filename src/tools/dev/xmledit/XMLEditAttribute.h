// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef XMLEDITATTRIBUTE_H
#define XMLEDITATTRIBUTE_H

#include <QFrame>

class XMLDocument;
class QLineEdit;
class QButtonGroup;
class QComboBox;
class QCheckBox;

// ****************************************************************************
//  Class:  XMLEditAttribute
//
//  Purpose:
//    Attribute editing widget for the XML editor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October 17, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Mar 7 16:04:45 PST 2007
//    Added control to turn off keyframe.
//
//    Cyrus Harrison, Thu May 15 16:00:46 PDT 200
//    First pass at porting to Qt 4.4.0
//
//    Mark C. Miller, Wed Aug 26 11:03:19 PDT 2009
//    Added support for custom base class for derived state objects.
// ****************************************************************************
class XMLEditAttribute : public QFrame
{
    Q_OBJECT
  public:
    XMLEditAttribute(QWidget *p);
    void SetDocument(XMLDocument *doc) { xmldoc = doc; }
    void BlockAllSignals(bool);
  public slots:
    void UpdateWindowContents();
    void UpdateWindowSensitivity();
    void nameTextChanged(const QString&);
    void purposeTextChanged(const QString&);
    void codefileTextChanged(const QString&);
    void exportAPITextChanged(const QString&);
    void exportIncludeTextChanged(const QString&);
    void persistentChanged();
    void keyframeChanged();
    void customBaseClassChanged();
    void baseClassTextChanged(const QString&);
  private:
    XMLDocument   *xmldoc;

    QLineEdit     *name;
    QLineEdit     *purpose;
    QLineEdit     *codefile;
    QLineEdit     *exportAPI;
    QLineEdit     *exportInclude;
    QCheckBox     *persistent;
    QCheckBox     *keyframe;
    QCheckBox     *customBaseClass;
    QLineEdit     *baseClass;
};

#endif
