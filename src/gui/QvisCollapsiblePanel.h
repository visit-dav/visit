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

#ifndef QVISCOLLAPSIBLEPANEL_H_
#define QVISCOLLAPSIBLEPANEL_H_

#include <QWidget>
#include <QLabel>

#include <gui_exports.h>

class QVBoxLayout;

// ****************************************************************************
// Class: QvisCaptionLabel
//
// Purpose:
//   This class is a caption label for the collapsable panel
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
class GUI_API QvisCaptionLabel: public QLabel
{
  Q_OBJECT

public:
  QvisCaptionLabel(const QString& caption,QWidget* parent);

  void mousePressEvent(QMouseEvent* e);
  void paintEvent(QPaintEvent *event);
  bool isCollapsed()const{return m_collapsed;}
  void collapse();
  void expand();

signals:
  void collapseOrExpand(bool);

private:
  bool m_collapsed;
};

// ****************************************************************************
// Class: QvisCollapsiblePanel
//
// Purpose:
//   This class is a collapsable panel
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
class GUI_API QvisCollapsiblePanel: public QWidget
{
  Q_OBJECT

public:
  QvisCollapsiblePanel(const QString& caption,QWidget* parent);

  void setWidget(QWidget* widget);
  void setCaption(const QString& caption);
  bool isCollapsed()const;

signals:
  void collapsed();
  void expanded();

public slots:
  void collapse();
  void expand();

private slots:
  void collapseOrExpand(bool);

private:
  QWidget *m_widget;
  QVBoxLayout* m_layout;
  QvisCaptionLabel* m_label;
};

// ****************************************************************************
// Class: QvisCollapsibleStack
//
// Purpose:
//   This class manages the collapsable panels
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
class GUI_API QvisCollapsibleStack: public QWidget
{
  Q_OBJECT
 public:
  QvisCollapsibleStack(QWidget* parent);
  QvisCollapsiblePanel* addPanel(const QString& caption,QWidget* widget);

private slots:
  void updateStretch();
  void shrink();

private:
  bool allCollapsed()const;
  QVBoxLayout* m_layout;
};

#endif /*QVISCOLLAPSIBLEPANEL_H_*/
