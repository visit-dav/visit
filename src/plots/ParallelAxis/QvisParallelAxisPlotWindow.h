#ifndef QVIS_PARALLEL_AXIS_PLOT_WINDOW_H
#define QVIS_PARALLEL_AXIS_PLOT_WINDOW_H

#include <QvisPostableWindowObserver.h>
#include <ObserverToCallback.h>

#include <AttributeSubject.h>

#include <vectortypes.h>

#include <vector>
#include <string>

class ParallelAxisAttributes;
class QButtonGroup;
class QPushButton;
class QvisVariableButton;
class QLabel;


// ****************************************************************************
// Class: QvisParallelAxisPlotWindow
//
// Purpose: GUI window for the ParallelAxis (parallel coordinate) plot.
//
// Programmer: Mark Blair
// Creation:   Mon Mar 27 18:24:00 PST 2006
//
// Modifications:
//
//      Mark Blair, Wed Aug 16 17:12:00 PDT 2006
//      Removed widgets that display axis extents and extents selected by
//      Extents tool.  These were considered unnecessary.
//
// ****************************************************************************

class QvisParallelAxisPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisParallelAxisPlotWindow(const int type,
                                ParallelAxisAttributes *parAxisAtts_,
                                const char *caption = 0,
                                const char *shortName = 0,
                                QvisNotepadArea *notepad = 0);
    virtual ~QvisParallelAxisPlotWindow();
    virtual void CreateWindowContents();

public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();

protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);

private slots:
    void prevAxisClicked();
    void nextAxisClicked();
    void axisAdded(const QString &axisToAdd);
    void axisDeleted(const QString &axisToDelete);
    void leftAxisSelected(const QString &axisToSelect);

private:
    void UpdateShownFields(bool applyvalues);

    int                         plotType;

    ParallelAxisAttributes     *parAxisAtts;

    QLabel                      *axisVariable;
    QLabel                      *axisPosition;
    QPushButton                 *showPrevAxis;
    QPushButton                 *showNextAxis;
    QvisVariableButton          *addAxis;
    QvisVariableButton          *deleteAxis;
    QvisVariableButton          *leftAxis;

    int                          latestGUIShownPos;
};


#endif
