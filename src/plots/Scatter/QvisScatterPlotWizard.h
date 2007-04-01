#ifndef QVIS_SCATTER_PLOT_WIZARD_H
#define QVIS_SCATTER_PLOT_WIZARD_H
#include <QvisWizard.h>

class QButtonGroup;
class QFrame;
class QvisScatterWidget;

// ****************************************************************************
// Class: QvisScatterPlotWizard
//
// Purpose:
//   This class is a wizard that helps the user choose initial parameters for
//   the Scatter plot.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:53:27 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

class QvisScatterPlotWizard : public QvisWizard
{
    Q_OBJECT
public:
    QvisScatterPlotWizard(AttributeSubject *s, QWidget *parent,
        const char *name = 0);
    virtual ~QvisScatterPlotWizard();

    virtual bool appropriate(QWidget *) const;
private slots:
    void choseYVariable(const QString &);
    void choseZVariable(const QString &);
    void choseColorVariable(const QString &);
    void decideZ(int);
    void decideColor(int);
protected:
    void CreateVariablePage(QFrame **, QvisScatterWidget **,
        const char *prompt, const char *slot,
        bool highlight, bool threeD, bool colorPoints);
    void CreateYesNoPage(QFrame **, QvisScatterWidget **, QButtonGroup **,
        const char *prompt, const char *slot,
        bool highlight, bool threeD, bool colorPoints);
    void CreateFinishPage(QFrame **, QvisScatterWidget **,
        const char *prompt,
        bool highlight, bool threeD, bool colorPoints);

    QFrame            *page1;
    QFrame            *page2;
    QFrame            *page3;
    QFrame            *page4;
    QFrame            *page5;
    QFrame            *page6;

    QvisScatterWidget *scatter1;
    QvisScatterWidget *scatter2;
    QvisScatterWidget *scatter3;
    QvisScatterWidget *scatter4;
    QvisScatterWidget *scatter5;
    QvisScatterWidget *scatter6;

    QButtonGroup      *bg2;
    QButtonGroup      *bg4;

    bool selectZCoord;
    bool selectColor;
};

#endif
