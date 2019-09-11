// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_VARIABLE_LIST_LIMITER_H
#define QVIS_VARIABLE_LIST_LIMITER_H
#include <gui_exports.h>
#include <QScrollArea>

class QVBoxLayout;
class QvisHistogramLimits;

// ****************************************************************************
// Class: QvisVariableListLimiter
//
// Purpose:
//   Manage a list of QvisHistogramLimits objects so we can manage limits and
//   histograms for several variables at once.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:32:28 PST 2010
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisVariableListLimiter : public QScrollArea
{
    Q_OBJECT
public:
    QvisVariableListLimiter(QWidget *parent);
    virtual ~QvisVariableListLimiter();

    void setNumVariables(int n);
    int getNumVariables() const;

    QvisHistogramLimits *getVariable(int i);

signals:
    void deleteVariable(const QString &);
    void selectedRangeChanged(const QString &, float, float);
private slots:
    void closeVariable();
    void handleObjectDeletion();
private:
    QWidget                     *central;
    QVBoxLayout                 *vLayout;
    QList<QvisHistogramLimits *> widgets;
    QvisHistogramLimits         *deleteObject;
};

#endif
