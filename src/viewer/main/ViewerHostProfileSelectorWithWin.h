#ifndef VIEWER_HOSTPROFILESELECTOR_WITHWIN_H
#define VIEWER_HOSTPROFILESELECTOR_WITHWIN_H
#include <viewer_exports.h>
#include <qdialog.h>
#include <ViewerHostProfileSelector.h>

#include <string>
#include <vector>
#include <map>

class QLineEdit;
class QLabel;
class QSpinBox;
class QListBox;
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
//
// ****************************************************************************
class VIEWER_API ViewerHostProfileSelectorWithWin : public QDialog, 
                                                    public ViewerHostProfileSelector
{
    Q_OBJECT
  public:
             ViewerHostProfileSelectorWithWin(QWidget *parent=NULL, const char *name=NULL);
    virtual ~ViewerHostProfileSelectorWithWin();

    virtual bool SelectProfile(HostProfileList*, const std::string&, 
                               bool skip);

  public slots:
    void   newProfileSelected();

  private:
    QListBox  *profiles;
    QLabel    *numProcsLabel;
    QSpinBox  *numProcs;
    QLabel    *numNodesLabel;
    QSpinBox  *numNodes;
    QLabel    *bankNameLabel;
    QLineEdit *bankName;
    QLabel    *timeLimitLabel;
    QLineEdit *timeLimit;

    QPushButton *okayButton;
    QPushButton *cancelButton;
};

#endif
