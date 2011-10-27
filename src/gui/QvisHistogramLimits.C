/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

#include <QvisHistogramLimits.h>

#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QTimer>

#include <QvisHistogram.h>

#include <float.h>
#define MIN_VALUE -FLT_MAX
#define MAX_VALUE  FLT_MAX
#define EPSILON (100 * FLT_MIN)

// ****************************************************************************
// Method: QvisHistogramLimits::QvisHistogramLimits
//
// Purpose: 
//   Constructor
//
// Arguments:
//   parent : The widget's parent.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QvisHistogramLimits::QvisHistogramLimits(QWidget *parent) :
    QGroupBox(parent), hitRect()
{
    closeEnabled = false;
    setTitle(" ");

    QGridLayout *gLayout = new QGridLayout(this);
    gLayout->setMargin(5);
    gLayout->setSpacing(5);

    QLabel *minLabel = new QLabel(tr("Minimum"), this);
    gLayout->addWidget(minLabel, 0, 0);
    selectedRange[0] = new QLineEdit(this);
    connect(selectedRange[0], SIGNAL(editingFinished()),
            this, SLOT(minChanged()));
    gLayout->addWidget(selectedRange[0], 0, 1);

    QLabel *maxLabel = new QLabel(tr("Maximum"), this);
    gLayout->addWidget(maxLabel, 0, 2);
    selectedRange[1] = new QLineEdit(this);
    connect(selectedRange[1], SIGNAL(editingFinished()),
            this, SLOT(maxChanged()));
    gLayout->addWidget(selectedRange[1], 0, 3);

    histogram = new QvisHistogram(this);
    histogram->setEnabled(false);
    connect(histogram, SIGNAL(selectedRangeChanged(float,float)),
            this, SLOT(rangeChanged(float,float)));
    gLayout->addWidget(histogram, 1, 0, 1, 4);

    totalRange[0] = new QLabel(this);
    gLayout->addWidget(totalRange[0], 2, 0);

    totalRange[1] = new QLabel(this);
    gLayout->addWidget(totalRange[1], 2, 3, Qt::AlignRight);

    gLayout->setRowStretch(1, 10);
}

// ****************************************************************************
// Method: QvisHistogramLimits::~QvisHistogramLimits
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QvisHistogramLimits::~QvisHistogramLimits()
{
}

// ****************************************************************************
// Method: QvisHistogramLimits::setHistogram
//
// Purpose: 
//   Set the histogram that we'll display.
//
// Arguments:
//   hist : The histogram texture data.
//   nhist : The number of elements in the histogram array.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramLimits::setHistogram(const float *hist, int nhist)
{
    histogram->setEnabled(true);
    histogram->setHistogramTexture(hist, nhist);
}

// ****************************************************************************
// Method: QvisHistogramLimits::getTotalRange
//
// Purpose: 
//   Return the total range used in the histogram.
//
// Arguments:
//   valid : Whether the histogram range is valid.
//   r0    : The min value.
//   r1    : The max value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramLimits::getTotalRange(bool &valid, float &r0, float &r1) const
{
    histogram->getTotalRange(valid, r0, r1);
}

// ****************************************************************************
// Method: QvisHistogramLimits::GetRangeText
//
// Purpose: 
//   Get the range text.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 26 17:08:25 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramLimits::GetRangeText(float r0, float r1, QString &r0Text, QString &r1Text) const
{
    if(r0 <= (MIN_VALUE + EPSILON))
        r0Text = tr("min").toLower();
    else
        r0Text = QString().setNum(r0);
    if(r1 >= (MAX_VALUE - EPSILON))
        r1Text = tr("max").toLower();
    else 
        r1Text = QString().setNum(r1);
}

// ****************************************************************************
// Method: QvisHistogramLimits::setTotalRange
//
// Purpose: 
//   Set the total range used in the histogram.
//
// Arguments:
//   r0    : The min value.
//   r1    : The max value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Oct 26 16:43:01 PDT 2011
//   Handle min and max.
//
// ****************************************************************************

void
QvisHistogramLimits::setTotalRange(float r0, float r1)
{
    QString r0Text, r1Text;
    GetRangeText(r0, r1, r0Text, r1Text);
    totalRange[0]->setText(r0Text);
    totalRange[1]->setText(r1Text);

    histogram->setTotalRange(r0, r1);
}

// ****************************************************************************
// Method: QvisHistogramLimits::invalidateTotalRange
//
// Purpose: 
//   Invalidate the total range used in the histogram.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramLimits::invalidateTotalRange()
{
    histogram->invalidateTotalRange();
    totalRange[0]->setText("");
    totalRange[1]->setText("");
}

// ****************************************************************************
// Method: QvisHistogramLimits::getSelectedRange
//
// Purpose: 
//   Return the selected range used in the histogram.
//
// Arguments:
//   r0    : The min value.
//   r1    : The max value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Oct 26 15:31:32 PDT 2011
//   Force the min and max to get locked in before we ask for them.
//
// ****************************************************************************

void
QvisHistogramLimits::getSelectedRange(float &r0, float &r1) 
{
    // Make sure the values are up to date with the text fields.
    blockSignals(true);
    minChanged();
    maxChanged();
    blockSignals(false);

    histogram->getSelectedRange(r0, r1);
}

// ****************************************************************************
// Method: QvisHistogramLimits::setSelectedRange
//
// Purpose: 
//   Set the selected range used in the histogram.
//
// Arguments:
//   r0    : The min value.
//   r1    : The max value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Oct 26 16:48:13 PDT 2011
//   Handle min/max.
//
// ****************************************************************************

void
QvisHistogramLimits::setSelectedRange(float r0, float r1)
{
    QString r0Text, r1Text;
    GetRangeText(r0, r1, r0Text, r1Text);

    selectedRange[0]->setText(r0Text);
    selectedRange[1]->setText(r1Text);

    histogram->setSelectedRange(r0, r1);
}

// ****************************************************************************
// Method: QvisHistogramLimits::setVariable
//
// Purpose: 
//   Set the name of the variable used in the histogram.
//
// Arguments:
//   var : The variable name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramLimits::setVariable(const QString &var)
{
    setTitle(var);
}

// ****************************************************************************
// Method: QvisHistogramLimits::getVariable
//
// Purpose: 
//   Return the variable name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

QString
QvisHistogramLimits::getVariable() const
{
    return title();
}

// ****************************************************************************
// Method: QvisHistogramLimits::setCloseEnabled
//
// Purpose: 
//   Set whether the close button is enabled.
//
// Arguments:
//   val : Whether the close button is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramLimits::setCloseEnabled(bool val)
{
    closeEnabled = val;
    update();
}

// ****************************************************************************
// Method: QvisHistogramLimits::getCloseEnabled
//
// Purpose: 
//   Return whether the close button is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

bool
QvisHistogramLimits::getCloseEnabled() const
{
    return closeEnabled;
}

// ****************************************************************************
// Method: QvisHistogramLimits::paintEvent
//
// Purpose: 
//   Draw a "close" button over the frame so we can click on it to issue a 
//   closeClicked signal.
//
// Arguments:
//   pe : The paint event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramLimits::paintEvent(QPaintEvent *pe)
{
    QGroupBox::paintEvent(pe);

    // Draw a close button.
    if(closeEnabled)
    {
        QIcon del(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
#if QT_VERSION < 0x040500
        QSize size(del.actualSize(QSize(20,20)));
#else
        QList<QSize> sizes(del.availableSizes());
        QSize size(sizes[0]);
#endif
        QPainter p(this);

        hitRect = QRect(width() - size.width() - 5, 0,
                        size.width(), size.height());

        p.fillRect(hitRect, palette().brush(QPalette::Background));
        del.paint(&p, hitRect, Qt::AlignCenter,
                  QIcon::Active, isEnabled() ? QIcon::On : QIcon::Off);
    }
}

// ****************************************************************************
// Method: QvisHistogramLimits::mousePressEvent
//
// Purpose: 
//   We override this method so we can see if we clicked on the close button
//   that this class provides.
//
// Arguments:
//   e : The mouse event.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
QvisHistogramLimits::mousePressEvent(QMouseEvent *e)
{
    if(closeEnabled && hitRect.contains(e->pos()))
        emit closeClicked();
    else
        QGroupBox::mousePressEvent(e);
}

//
// Qt slots
//

// ****************************************************************************
// Method: QvisHistogramLimits::updateSelectedText
//
// Purpose: 
//   This Qt slot function updates the selected range line edits.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Oct 26 16:56:47 PDT 2011
//   Use GetRangeText.
//
// ****************************************************************************

void
QvisHistogramLimits::updateSelectedText()
{
    float r0, r1;
    histogram->getSelectedRange(r0, r1);

    QString r0Text, r1Text;
    GetRangeText(r0, r1, r0Text, r1Text);
    selectedRange[0]->setText(r0Text);
    selectedRange[1]->setText(r1Text);
}

// ****************************************************************************
// Method: QvisHistogramLimits::minChanged
//
// Purpose: 
//   This Qt slot function gets called when the min gets changed via the 
//   min line edit.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Oct 26 16:51:29 PDT 2011
//   Handle min/max.
//
// ****************************************************************************

void
QvisHistogramLimits::minChanged()
{
    // Convert the text to a float and store the value into the histogram.
    bool okay = false;
    QString minText, maxText;
    GetRangeText(MIN_VALUE, MAX_VALUE, minText, maxText);
    QString txt(selectedRange[0]->text().toLower());
    float value;
    if(txt == minText)
    {
        value = MIN_VALUE;
        okay = true;
    }
    else
        value = txt.toFloat(&okay);

    if(okay)
    {
        float r0, r1;
        histogram->getSelectedRange(r0, r1);
        if(r0 != value)
        {
            if(histogram->setSelectedRange(value, r1))
                QTimer::singleShot(10, this, SLOT(updateSelectedText()));

            histogram->getSelectedRange(r0, r1);
            emit selectedRangeChanged(r0, r1);
            emit selectedRangeChanged(getVariable(), r0, r1);
        }
    }
}

// ****************************************************************************
// Method: QvisHistogramLimits::maxChanged
//
// Purpose: 
//   This Qt slot function gets called when the max gets changed via the 
//   max line edit.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Oct 26 16:52:10 PDT 2011
//   Handle min/max.
//
// ****************************************************************************

void
QvisHistogramLimits::maxChanged()
{
    // Convert the text to a float and store the value into the histogram.
    bool okay = false;
    QString minText, maxText;
    GetRangeText(MIN_VALUE, MAX_VALUE, minText, maxText);
    QString txt(selectedRange[1]->text().toLower());
    float value;
    if(txt == maxText)
    {
        value = MAX_VALUE;
        okay = true;
    }
    else
        value = txt.toFloat(&okay);

    if(okay)
    {
        float r0, r1;
        histogram->getSelectedRange(r0, r1);
        if(r1 != value)
        {
            if(histogram->setSelectedRange(r0, value))
                QTimer::singleShot(10, this, SLOT(updateSelectedText()));

            histogram->getSelectedRange(r0, r1);
            emit selectedRangeChanged(r0, r1);
            emit selectedRangeChanged(getVariable(), r0, r1);
        }
    }
}

// ****************************************************************************
// Method: QvisHistogramLimits::rangeChanged
//
// Purpose: 
//   This Qt slot gets called when we adjust the ranges using the histogram
//   control.
//
// Arguments:
//   r0 : The new minimum range.
//   r1 : The new maximum range.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 28 16:00:40 PST 2010
//
// Modifications:
//   Brad Whitlock, Wed Oct 26 16:52:55 PDT 2011
//   Handle min/max.
//
// ****************************************************************************

void
QvisHistogramLimits::rangeChanged(float r0, float r1)
{
    QString r0Text, r1Text;
    GetRangeText(r0, r1, r0Text, r1Text);
    selectedRange[0]->setText(r0Text);
    selectedRange[1]->setText(r1Text);

    emit selectedRangeChanged(r0, r1);
    emit selectedRangeChanged(getVariable(), r0, r1);
}
