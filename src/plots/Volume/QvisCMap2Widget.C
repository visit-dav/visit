#include <QvisCMap2Widget.h>

#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTimer>

#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>

// ****************************************************************************
// Method: QvisCMap2Widget::QvisCMap2Widget
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   parent : The widget's parent.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 25 13:55:08 PDT 2008
//
// Modifications:
//   Brad Whitlock, Tue Sep 30 10:36:48 PDT 2008
//   Qt 4, added tr()'s.
//
// ****************************************************************************

QvisCMap2Widget::QvisCMap2Widget(QWidget *parent) : QWidget(parent)
{
    nameIndex = 0;

    QGridLayout *topLayout = new QGridLayout(this);
    topLayout->setSpacing(5);
    topLayout->setMargin(5); 
    cmap2 = new QvisCMap2Display(this);
    topLayout->addWidget(cmap2, 0, 0, 1, 2);
    topLayout->setRowStretch(0, 5);
    connect(cmap2, SIGNAL(widgetListChanged()),
            this, SIGNAL(widgetListChanged()));
    connect(cmap2, SIGNAL(widgetListChanged()),
            this, SLOT(updateList()));
    connect(cmap2, SIGNAL(selectWidget(WidgetID)),
            this, SIGNAL(selectWidget(WidgetID)));
    connect(cmap2, SIGNAL(widgetChanged(WidgetID)),
            this, SIGNAL(widgetChanged(WidgetID)));
    connect(cmap2, SIGNAL(widgetChanged(WidgetID)),
            this, SLOT(updateWidget(WidgetID)));

    QLabel *nameLabel = new QLabel(tr("Widgets"), this);
    topLayout->addWidget(nameLabel, 1, 0);

    names = new QListWidget(this);
    connect(names, SIGNAL(currentRowChanged(int)),
            this, SLOT(showProperties(int)));
    topLayout->addWidget(names, 2, 0);

    QHBoxLayout *btnLayout = new QHBoxLayout(0);
    topLayout->addLayout(btnLayout, 3, 0, 1, 2);
    btnLayout->setSpacing(5);

    QPushButton *addTriButton = new QPushButton(tr("New triangle"), this);
    btnLayout->addWidget(addTriButton);
    connect(addTriButton, SIGNAL(clicked()),
            this, SLOT(addNewTriangle()));

    QPushButton *addRectButton = new QPushButton(tr("New rectangle"), this);
    btnLayout->addWidget(addRectButton);
    connect(addRectButton, SIGNAL(clicked()),
            this, SLOT(addNewRectangle()));

    QPushButton *addEllipseButton = new QPushButton(tr("New ellipsoid"), this);
    btnLayout->addWidget(addEllipseButton);
    connect(addEllipseButton, SIGNAL(clicked()),
            this, SLOT(addNewEllipsoid()));

    QPushButton *addParaboloidButton = new QPushButton(tr("New paraboloid"), this);
    btnLayout->addWidget(addParaboloidButton);
    connect(addParaboloidButton, SIGNAL(clicked()),
            this, SLOT(addNewParaboloid()));

    deleteButton = new QPushButton(tr("Delete"), this);
    btnLayout->addStretch(1);
    btnLayout->addWidget(deleteButton);
    connect(deleteButton, SIGNAL(clicked()),
            this, SLOT(deleteWidget()));

    QGroupBox *attsGroup = new QGroupBox(this);
    attsGroup->setTitle(tr("Widget attributes"));
    topLayout->addWidget(attsGroup, 1, 1, 2, 1);
    topLayout->setColumnStretch(1, 10);
    QVBoxLayout *attsInnerTopLayout = new QVBoxLayout(attsGroup);
    attsInnerTopLayout->setMargin(10);
    attsInnerTopLayout->setSpacing(10);
    QGridLayout *attsLayout = new QGridLayout(0);
    attsInnerTopLayout->addLayout(attsLayout);
    attsInnerTopLayout->addStretch(5);
    attsLayout->setSpacing(5);

    wName = new QLineEdit(attsGroup);
    wName->setEnabled(false); // allow editing name sometime.
    attsLayout->addWidget(new QLabel(tr("Name"), attsGroup), 0, 0);
    attsLayout->addWidget(wName, 0, 1);

    sizeLoc = new QLineEdit(attsGroup);
    connect(sizeLoc, SIGNAL(returnPressed()),
            this, SLOT(setSizeLoc()));
    attsLayout->addWidget(new QLabel(tr("Size/Location"), attsGroup), 1, 0);
    attsLayout->addWidget(sizeLoc, 1, 1);
  
    color = new QvisColorButton(attsGroup);
    connect(color, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(setWidgetColor(const QColor &)));
    attsLayout->addWidget(new QLabel(tr("Color"), attsGroup), 2, 0);
    attsLayout->addWidget(color, 2, 1);

    opacity = new QvisOpacitySlider(attsGroup);
    connect(opacity, SIGNAL(valueChanged(int)),
            this, SLOT(setWidgetOpacity(int)));
    attsLayout->addWidget(opacity, 3, 1);
}

QvisCMap2Widget::~QvisCMap2Widget()
{
}

WidgetID
QvisCMap2Widget::addTriangleWidget(const QString &wName,
  float base,  // X-coordinate of bottom point
  float top_x, // Offset in X from top left point
  float top_y, // Y-coordinate of top left point
  float w, // Width of top of triangle.
  float bottom // Distance along triangle median
  )
{
    return cmap2->addTriangleWidget(wName, base, top_x, top_y, w, bottom);
}

WidgetID
QvisCMap2Widget::addRectangleWidget(const QString &wName,
   float left_x,
   float left_y,
   float w,
   float h,
   float offset
   )
{
    return cmap2->addRectangleWidget(wName, left_x, left_y, w, h, offset);
}

int
QvisCMap2Widget::numWidgets() const
{
    return cmap2->numWidgets();
}

WidgetID
QvisCMap2Widget::getID(int index) const
{
  return cmap2->getID(index);
}

void
QvisCMap2Widget::removeWidget(WidgetID id)
{
    cmap2->removeWidget(id);
}


void  
QvisCMap2Widget::setDefaultColor(const QColor &c)
{
    cmap2->setDefaultColor(c);
}

QColor
QvisCMap2Widget::getDefaultColor() const
{
    return cmap2->getDefaultColor();
}


void  
QvisCMap2Widget::setDefaultAlpha(float f)
{
    cmap2->setDefaultAlpha(f);
}

float 
QvisCMap2Widget::getDefaultAlpha() const
{
    return cmap2->getDefaultAlpha();
}


void  
QvisCMap2Widget::setColor(WidgetID id, const QColor &c)
{
    cmap2->setColor(id, c);
}

QColor
QvisCMap2Widget::getColor(WidgetID id) const
{
    return cmap2->getColor(id);
}


void  
QvisCMap2Widget::setAlpha(WidgetID id, float a)
{
    cmap2->setAlpha(id, a);
}

float 
QvisCMap2Widget::getAlpha(WidgetID id) const
{
    return cmap2->getAlpha(id);
}


void  
QvisCMap2Widget::setName(WidgetID id, const QString &name)
{
    cmap2->setName(id, name);
}

QString 
QvisCMap2Widget::getName(WidgetID id) const
{
    return cmap2->getName(id);
}

QString
QvisCMap2Widget::getString(WidgetID id) const
{
    return cmap2->getString(id);
}

QString
QvisCMap2Widget::newName(const QString &base)
{
    nameIndex++;
    QString s;
    s.sprintf("%04d", nameIndex);
    return base + s;
}

WidgetID
QvisCMap2Widget::getSelectedWidget() const
{
    int index = names->currentRow();
    WidgetID id = cmap2->getID(index);
    return id;
}

//
// Qt slots
//
void
QvisCMap2Widget::addNewTriangle()
{
    WidgetID w = cmap2->addTriangleWidget(newName(tr("tri")),
        0.3, // X-coordinate of bottom point
        0.25, // Offset in X from top left point
        0.5, // Y-coordinate of top left point
        0.7, // Width of top of triangle.
        0.5 );

    QTimer::singleShot(100, this, SLOT(selectLastItem()));
}

void
QvisCMap2Widget::addNewRectangle()
{
    WidgetID w = cmap2->addRectangleWidget(newName(tr("rect")),
        0.1, // left_x
        0.4, // left_y
        0.5, // w
        0.4, // h
        0.5 // offset
        );

    QTimer::singleShot(100, this, SLOT(selectLastItem()));
}

void
QvisCMap2Widget::addNewEllipsoid()
{
  WidgetID w = cmap2->addEllipsoidWidget(newName(tr("ellipse")),
                                         .2,
                                         .2,
                                         .1,
                                         .2,
                                         0.2);

  QTimer::singleShot(100, this, SLOT(selectLastItem()));
}

void
QvisCMap2Widget::addNewParaboloid()
{
  WidgetID w = cmap2->addParaboloidWidget(newName(tr("paraboloid")),
                                          .5, .6,
                                          .5, .4,
                                          .3, .5,
                                          .7, .5);

  QTimer::singleShot(100, this, SLOT(selectLastItem()));
}

void
QvisCMap2Widget::updateList()
{
    // The widget list changed so let's update the widget list.
    names->clear();
    for(int i = 0; i < numWidgets(); ++i)
    {
        WidgetID id = cmap2->getID(i);
        if(id != QvisCMap2Display::WIDGET_NOT_FOUND)
        {
            QString n(cmap2->getName(id));
            names->addItem(n);
        }
    }

    deleteButton->setEnabled(numWidgets() > 0);
}

void
QvisCMap2Widget::selectLastItem()
{
    names->setCurrentItem(names->item(names->count()-1));
    names->item(names->count()-1)->setSelected(true);
}

void
QvisCMap2Widget::deleteWidget()
{
    if(names->currentItem() != 0)
    {
        int index = names->currentRow();
        WidgetID id = cmap2->getID(index);
        if(id != QvisCMap2Display::WIDGET_NOT_FOUND)
        {
            removeWidget(id);
        }
    }
//    else
//        qDebug("No widget selected");

    QTimer::singleShot(100, this, SLOT(selectLastItem()));
}

void
QvisCMap2Widget::showProperties(int index)
{
    WidgetID id = cmap2->getID(index);
    updateWidget(id);
}

void
QvisCMap2Widget::updateWidget(WidgetID id)
{
    // If the widget is not selected in the list, select it.
    WidgetID selID = getSelectedWidget();
    if(selID != id)
    {
        names->blockSignals(true);
        for(int i = 0; i < names->count(); ++i)
            if(cmap2->getID(i) == id)
            {
                names->setCurrentItem(names->item(i));
                break;
            }
        names->blockSignals(false);
    }

    // Update the widgets so they show the values for the widget.
    wName->setText(cmap2->getName(id));

    sizeLoc->blockSignals(true);
    sizeLoc->setText(cmap2->getString(id));
    sizeLoc->blockSignals(false);

    color->blockSignals(true);
    color->setButtonColor(cmap2->getColor(id));
    color->blockSignals(false);

    opacity->blockSignals(true);
    opacity->setValue((int)(100. * cmap2->getAlpha(id)));
    opacity->blockSignals(false);
    opacity->setGradientColor(cmap2->getColor(id));
}

void
QvisCMap2Widget::setWidgetColor(const QColor &c)
{
    WidgetID id = getSelectedWidget();
    if(id != QvisCMap2Display::WIDGET_NOT_FOUND)
    {
        cmap2->blockSignals(true);
        cmap2->setColor(id, c);
        cmap2->blockSignals(false);

        opacity->setGradientColor(c);

        emit widgetChanged(id);
    }
}

void
QvisCMap2Widget::setWidgetOpacity(int val)
{
    WidgetID id = getSelectedWidget();
    if(id != QvisCMap2Display::WIDGET_NOT_FOUND)
    {
        cmap2->blockSignals(true);
        cmap2->setAlpha(id, float(val) / 100.f);
        cmap2->blockSignals(false);

        emit widgetChanged(id);
    }
}

void
QvisCMap2Widget::setSizeLoc()
{
    WidgetID id = getSelectedWidget();
    if(id != QvisCMap2Display::WIDGET_NOT_FOUND)
    {
        cmap2->blockSignals(true);
        cmap2->setString(id, sizeLoc->text());
        cmap2->blockSignals(false);
    }
}
