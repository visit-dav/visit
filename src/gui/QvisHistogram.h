/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
