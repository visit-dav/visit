// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISCRACKSCLIPPERWINDOW_H
#define QVISCRACKSCLIPPERWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class CracksClipperAttributes;
class QCheckBox;
class QLabel;
class QvisVariableButton;

// ****************************************************************************
//  Class: QvisCracksClipperWindow
//
//  Purpose: 
//    Defines QvisCracksClipperWindow class.
//
//  Notes:      This class was automatically generated!
//
//  Programmer: xml2window
//  Creation:   Mon Aug 22 09:10:02 PDT 2005
//
//  Modifications:
//    Kathleen Bonnell, Mon May  7 15:48:42 PDT 2007
//    Added calculateDensity, inMassVar, outDenVar.
//   
//    Kathleen Bonnell, Wed Sep 29 09:01:48 PDT 2010
//    Removed calculateDensity, outDenVar.
//
// ****************************************************************************

class QvisCracksClipperWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisCracksClipperWindow(const int type,
                         CracksClipperAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisCracksClipperWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void crack1VarChanged(const QString &);
    void crack2VarChanged(const QString &);
    void crack3VarChanged(const QString &);
    void strainVarChanged(const QString &);
    void showCrack1Changed(bool val);
    void showCrack2Changed(bool val);
    void showCrack3Changed(bool val);
    void inMassVarChanged(const QString &);
  private:
    QvisVariableButton *crack1Var;
    QvisVariableButton *crack2Var;
    QvisVariableButton *crack3Var;
    QvisVariableButton *strainVar;
    QCheckBox *showCrack1;
    QCheckBox *showCrack2;
    QCheckBox *showCrack3;
    QvisVariableButton *inMassVar;
    QLabel *inMassVarLabel;

    CracksClipperAttributes *atts;
};



#endif
