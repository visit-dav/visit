#ifndef QVIS_DATABASECORRELATION_WINDOW_H
#define QVIS_DATABASECORRELATION_WINDOW_H
#include <QvisWindowBase.h>

class DatabaseCorrelation;
class QComboBox;
class QLineEdit;
class QListBox;
class QPushButton;

// ****************************************************************************
// Class: QvisDatabaseCorrelationWindow
//
// Purpose:
//   This class provides controls for designing a database correlation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Sat Jan 31 02:45:19 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisDatabaseCorrelationWindow : public QvisWindowBase
{
    Q_OBJECT
public:
    QvisDatabaseCorrelationWindow(const QString &correlationName,
        const char *caption);
    QvisDatabaseCorrelationWindow(const DatabaseCorrelation &correlation,
        const char *caption);
    virtual ~QvisDatabaseCorrelationWindow();

signals:
    void deleteMe(QvisWindowBase *);
protected slots:
    void setAddButtonEnabled(int);
    void setRemoveButtonEnabled(int);
    void addSources();
    void removeSources();
    void actionClicked();
    void cancelClicked();
protected:
    void CreateWidgets(const DatabaseCorrelation &correlation);
    void UpdateAddRemoveButtonsEnabledState();
    int  SelectedCount(const QListBox *) const;
    void TransferItems(QListBox *srcLB, QListBox *destLB);

    static int   instanceCount;
    bool         createMode;

    // Widgets and layout
    QLineEdit    *correlationNameLineEdit;
    QListBox     *sourcesListBox;
    QListBox     *correlatedSourcesListBox;
    QPushButton  *addButton;
    QPushButton  *removeButton;
    QComboBox    *correlationMethodComboBox;
//    QCheckBox    *automaticNStatesCheckBox;
//    QSpinBox     *nStatesSpinBox;
};

#endif
