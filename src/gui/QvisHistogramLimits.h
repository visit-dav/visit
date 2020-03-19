// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_HISTOGRAM_LIMITS_H
#define QVIS_HISTOGRAM_LIMITS_H
#include <gui_exports.h>
#include <QGroupBox>

class QLabel;
class QLineEdit;
class QvisHistogram;

// ****************************************************************************
// Class: QvisHistogramLimits
//
// Purpose:
//   This widget includes min/max text boxes coupled with a histogram widget
//   and you can use any of them to set the min/max limits.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   Brad Whitlock, Thu Oct 27 14:10:23 PDT 2011
//   Added GetRangeText.
//
// ****************************************************************************

class GUI_API QvisHistogramLimits : public QGroupBox
{
    Q_OBJECT
public:
    QvisHistogramLimits(QWidget *parent);
    virtual ~QvisHistogramLimits();

    void setHistogram(const float *hist, int nhist);

    void getTotalRange(bool &valid, float &r0, float &r1) const;
    void setTotalRange(float r0, float r1);
    void invalidateTotalRange();

    void getSelectedRange(float &r0, float &r1);
    void setSelectedRange(float r0, float r1);

    void setVariable(const QString &);
    QString getVariable() const;

    void setCloseEnabled(bool);
    bool getCloseEnabled() const;

signals:
    void selectedRangeChanged(float, float);
    void selectedRangeChanged(QString, float, float);
    void closeClicked();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    void GetRangeText(float r0, float r1, QString &r0Text, QString &r1Text) const;

private slots:
    void minChanged();
    void maxChanged();
    void rangeChanged(float, float);
    void updateSelectedText();
private:
    bool           closeEnabled;
    QRect          hitRect;
    QLineEdit     *selectedRange[2];
    QvisHistogram *histogram;
    QLabel        *totalRange[2];
};

#endif
