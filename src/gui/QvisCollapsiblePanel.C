/*****************************************************************************
*
* This file is part of Mantid. For details, see http://www.mantidproject.org/.
* The full copyright notice is contained in:
* http://www.mantidproject.org/Mantid:About#License_and_Distribution.
*
* Mantid is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.

* Mantid is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details: http://www.gnu.org/licenses.
*
*****************************************************************************/

#include "QvisCollapsiblePanel.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QFontMetrics>
#include <QPolygon>
#include <QTimer>

#include <stdexcept>
#include <iostream>

// ****************************************************************************
// Class: QvisCaptionLabel::QvisCaptionLabel
//
// Purpose:
//   Creates a label that goes across the frame
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
QvisCaptionLabel::QvisCaptionLabel(const QString& caption,
                                   QWidget* parent) :
  QLabel(caption,parent), m_collapsed(false)
{
  setFrameStyle(QFrame::WinPanel);
  setFrameShadow(QFrame::Raised);
}

// ****************************************************************************
// Class: QvisCaptionLabel::mousePressEvent
//
// Purpose:
//   Captures the mouse events for collapsing or expanding the panel.
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//

void QvisCaptionLabel::mousePressEvent(QMouseEvent* e)
{
  if (e->buttons() & Qt::LeftButton)
  {
    e->accept();
    m_collapsed = !m_collapsed;
    emit collapseOrExpand(m_collapsed);
  }
  else
  {
    e->ignore();
  }
}

// ****************************************************************************
// Class: QvisCaptionLabel::paintEvent
//
// Purpose:
//   Creates arrow indicating whether the panel is collapsed or expanded.
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
void QvisCaptionLabel::paintEvent(QPaintEvent *e)
{
  QLabel::paintEvent(e);

  QPainter painter(this);
  painter.setBrush(QBrush(QColor(Qt::black)));

  QFontMetrics fm(this->font());

  int h = fm.height() - 4;
  h = h / 2 * 2; // h is even

  if (h > 0)
  {
    int w = h / 2;
    int x = this->width() - 2*h;
    int y = (this->height() - h) / 2;

    QPolygon tri(3);
 
    if (m_collapsed)
    {
      // tri.setPoint(0, x, y + h);
      // tri.setPoint(1, x + w + w, y + h);
      // tri.setPoint(2, x + w, y);

      tri.setPoint(0, x, y + h);
      tri.setPoint(1, x + w + w, y + h/2);
      tri.setPoint(2, x, y);

      painter.drawText(x - w - w - w, y + h, "+" );
    }
    else
    {
      tri.setPoint(0, x, y);
      tri.setPoint(1, x + w + w ,y);
      tri.setPoint(2, x + w, y + h);

      painter.drawText(x - w - w - w, y + h, "-" );
    }

    painter.drawPolygon(tri);
  }
}

// ****************************************************************************
// Class: QvisCaptionLabel::collapsed
//
// Purpose:
//   Sets the flag for collapsed
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
void QvisCaptionLabel::collapse()
{
    m_collapsed = true;
    emit collapseOrExpand(m_collapsed);
}

// ****************************************************************************
// Class: QvisCaptionLabel::expand
//
// Purpose:
//   Set the flag for expanded (collapsed = false)
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//

void QvisCaptionLabel::expand()
{
    m_collapsed = false;
    emit collapseOrExpand(m_collapsed);
}

// ****************************************************************************
// Class: QvisCollapsiblePanel::QvisCollapsiblePanel
//
// Purpose:
//   This class is a label and a layout (panel) for widgets
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
QvisCollapsiblePanel::QvisCollapsiblePanel(const QString& caption,
                                           QWidget* parent) :
  QWidget(parent),m_widget(NULL)
{
  m_layout = new QVBoxLayout(this);
  m_label = new QvisCaptionLabel(caption,this);
  m_layout->addWidget(m_label);
  m_layout->setMargin(0);

  connect(m_label, SIGNAL(collapseOrExpand(bool)), this,
          SLOT(collapseOrExpand(bool)));
}

// ****************************************************************************
// Class: QvisCollapsiblePanel::setWidget
//
// Purpose: Sets the widget that will be contained in the layout. Only
//   one widget per layout (panel).
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
void QvisCollapsiblePanel::setWidget(QWidget* widget)
{
  if (m_widget)
  {
    throw std::runtime_error("CollapsiblePanel already has a widget");
  }
  m_widget = widget;
  m_widget->setParent(this);
  m_layout->addWidget(m_widget);
}

// ****************************************************************************
// Class: QvisCollapsiblePanel::SetCaption
//
// Purpose:
//   Sets the caption label for the collapsable panel
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
void QvisCollapsiblePanel::setCaption(const QString& caption)
{
  m_label->setText(caption);
}

// ****************************************************************************
// Class: QvisCollapsiblePanel::collapseOrExpand
//
// Purpose:
//   Collapses (hides) or expands (shows) the layout (panel)
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
void QvisCollapsiblePanel::collapseOrExpand(bool collapse)
{
  if (!m_widget)
    return;

  if (collapse)
  {
    m_widget->hide();
    emit collapsed();
  }
  else
  {
    m_widget->show();
    emit expanded();
  }
}

// ****************************************************************************
// Class: QvisCollapsiblePanel::isCollapsed
//
// Purpose:
//   Returns the panel's status:
//   collapsed (collapsed = true) or expanded (collapsed = false)
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
bool QvisCollapsiblePanel::isCollapsed()const
{
  return m_label->isCollapsed();
}

// ****************************************************************************
// Class: QvisCollapsiblePanel::collapsed
//
// Purpose:
//   Collapses (hides) a panel
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
void QvisCollapsiblePanel::collapse()
{
  m_label->collapse();
  collapseOrExpand(true);
}

// ****************************************************************************
// Class: QvisCollapsiblePanel::expand
//
// Purpose:
//   Expands (shows) a panel
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
void QvisCollapsiblePanel::expand()
{
  m_label->expand();
  collapseOrExpand(false);
}

// ****************************************************************************
// Class: QvisCollapsibleStack::QvisCollapsibleStack
//
// Purpose:
//   This class creates layout for managing collapsable panels.
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
QvisCollapsibleStack::QvisCollapsibleStack(QWidget* parent):
QWidget(parent)
{
  m_layout = new QVBoxLayout(this);
  m_layout->setMargin(0);
  setLayout(m_layout);
}

// ****************************************************************************
// Class: QvisCollapsibleStack::addPanel
//
// Purpose:
//   Adds a new collapsable panel to the layout
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
QvisCollapsiblePanel* QvisCollapsibleStack::addPanel(const QString& caption,
                                                     QWidget* widget)
{
  QvisCollapsiblePanel *panel = new QvisCollapsiblePanel(caption,this);
  panel->setWidget(widget);
  m_layout->addWidget(panel);

  connect(panel,SIGNAL(collapsed()),this,SLOT(updateStretch()));
  connect(panel,SIGNAL(expanded()),this,SLOT(updateStretch()));

  return panel;
}

// ****************************************************************************
// Class: QvisCollapsibleStack::allCollapsed
//
// Purpose:
//   Checks to see if all children are collapsed.
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
bool QvisCollapsibleStack::allCollapsed()const
{
  int n = m_layout->count();

  for(int i = 0; i < n; ++i)
  {
    QvisCollapsiblePanel* panel =
      dynamic_cast<QvisCollapsiblePanel*>(m_layout->itemAt(i)->widget());

    if (panel && !panel->isCollapsed())
    {
      return false;
    }
  }
  return true;
}

// ****************************************************************************
// Class: QvisCollapsibleStack::updateStretch
//
// Purpose:
//   Adds stretch to the controlling panel if needed.
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
void QvisCollapsibleStack::updateStretch()
{
  if (m_layout->count() == 0) return;

  // Point to the last item
  int i = m_layout->count() - 1;

  QvisCollapsiblePanel* panel =
    dynamic_cast<QvisCollapsiblePanel*>(m_layout->itemAt(i)->widget());

  if (allCollapsed())
  {// make sure that the last item is a stretch
    if (panel) // if it's a panel there is no stretch
    {
      m_layout->addStretch();
    }
  }
  else
  {
    if (!panel) // then it must be a stretch
    {
      m_layout->removeItem(m_layout->itemAt(i));
    }
  }

  QTimer::singleShot(0, this, SLOT(shrink()));

  emit shrink();
}

// ****************************************************************************
// Class: QvisCollapsibleStack::shrink
//
// Purpose:
//   Forces the window to resize after hiding or showing a layout.
//
// Notes:      
//
// Programmer: MANTiD Project
// Creation:   
//
// Modifications: 
//
// Allen Sanderson : The original class name has been changed to
// relfect the VisIt name style.
//
void QvisCollapsibleStack::shrink()
{
  m_layout->invalidate();

  QWidget *parent = m_layout->parentWidget();
 
  while (parent) {
    parent->adjustSize();
    parent = parent->parentWidget();
  }
}
