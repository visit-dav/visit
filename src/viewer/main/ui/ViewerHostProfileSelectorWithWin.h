// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_HOSTPROFILESELECTOR_WITHWIN_H
#define VIEWER_HOSTPROFILESELECTOR_WITHWIN_H
#include <viewer_exports.h>
#include <QDialog>
#include <ViewerHostProfileSelector.h>

#include <string>
#include <vector>
#include <map>

class QComboBox;
class QLineEdit;
class QLabel;
class QSpinBox;
class QListWidget;
class QPushButton;
class HostProfileList;

// ****************************************************************************
//  Class:  ViewerHostProfileSelectorWithWin
//
//  Purpose:
//    Selects a host profile. 
//
//  Notes:  Extracted from ViewerEngineChooser.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
//  Modifications:
//    Jeremy Meredith, Wed Oct 27 13:56:37 PDT 2004
//    Added flag so we know when we are waiting on a user already so that
//    we don't try to ask them multiple times about launching the same engine.
//
//    Jeremy Meredith, Thu Sep 15 16:42:48 PDT 2005
//    Added machine file for some mpich implementations.
//
//    Brad Whitlock, Fri May 23 11:34:00 PDT 2008
//    Qt 4.
//
//    Brad Whitlock, Thu Oct 23 10:39:59 PDT 2014
//    I added combo box versions of the node and proc controls.
//
// ****************************************************************************

class VIEWER_API ViewerHostProfileSelectorWithWin : public QDialog, 
                                                    public ViewerHostProfileSelector
{
    Q_OBJECT
  public:
             ViewerHostProfileSelectorWithWin(QWidget *parent = 0);
    virtual ~ViewerHostProfileSelectorWithWin();

    virtual bool SelectProfile(HostProfileList*, const std::string&, 
                               bool skip);

  public slots:
    void   newProfileSelected();
    void   numProcsFixedActivated(int);
    void   numNodesFixedActivated(int);

  private:
    bool       waitingOnUser;

    QListWidget  *profiles;
    QLabel    *numProcsLabel;
    QSpinBox  *numProcs;
    QComboBox *numProcsFixed;
    QLabel    *numNodesLabel;
    QSpinBox  *numNodes;
    QComboBox *numNodesFixed;
    QLabel    *bankNameLabel;
    QLineEdit *bankName;
    QLabel    *timeLimitLabel;
    QLineEdit *timeLimit;
    QLabel    *machinefileLabel;
    QLineEdit *machinefile;

    QPushButton *okayButton;
    QPushButton *cancelButton;
};

#endif
