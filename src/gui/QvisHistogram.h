// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_HISTOGRAM_H
#define QVIS_HISTOGRAM_H
#include <gui_exports.h>
#include <QvisAbstractOpacityBar.h>

// ****************************************************************************
// Class: QvisHistogram
//
// Purpose:
//   This class shows a histogram and lets the user select a range of it.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 11:03:09 PST 2010
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisHistogram : public QvisAbstractOpacityBar
{
    Q_OBJECT
public:
    QvisHistogram(QWidget *parent);
    virtual ~QvisHistogram();

    void setDrawBinLines(bool);
    bool getDrawBinLines() const;

    void getTotalRange(bool &valid, float &r0, float &r1) const;
    void setTotalRange(float r0, float r1);
    void invalidateTotalRange();

    void setSelectionEnabled(bool val);
    bool getSelectionEnabled() const;

    void getSelectedRange(float &r0, float &r1) const;
    bool setSelectedRange(float r0, float r1);

signals:
    void selectedRangeChanged(float, float);
protected:
    // Hide these methods
    virtual float *getRawOpacities(int);
    void           setBackgroundColorControlPoints(const ColorControlPointList *ccp);

protected:
    virtual void drawOpacities();

    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

    int   value2Pixel(float value) const;
    float pixel2Value(int pixel) const;

    bool  drawBinLines;
    bool  totalRangeValid;
    float totalRange[2];
    float selectedRange[2];

    bool  selectionEnabled;
    int   selection;
    float tempRange[2];
};

#endif
