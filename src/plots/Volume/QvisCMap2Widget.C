#include <QvisCMap2Widget.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qtimer.h>

#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>

QvisCMap2Widget::QvisCMap2Widget(QWidget *parent, const char *name) : QGroupBox(parent,name)
{
    nameIndex = 0;

    QGridLayout *topLayout = new QGridLayout(this, 3, 2);
    topLayout->setSpacing(5);
    topLayout->setMargin(5); 
    cmap2 = new QvisCMap2Display(this, "cmap2");
    topLayout->addMultiCellWidget(cmap2, 0, 0, 0, 1);
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

    QLabel *nameLabel = new QLabel("Widgets", this, "nameLabel");
    topLayout->addWidget(nameLabel, 1, 0);

    names = new QListBox(this, "names");
    connect(names, SIGNAL(highlighted(int)),
            this, SLOT(showProperties(int)));
    topLayout->addWidget(names, 2, 0);

    QHBoxLayout *btnLayout = new QHBoxLayout(0);
    topLayout->addMultiCellLayout(btnLayout, 3, 3, 0, 1);
    btnLayout->setSpacing(5);

    QPushButton *addTriButton = new QPushButton("New triangle", this, "addTriButton");
    btnLayout->addWidget(addTriButton);
    connect(addTriButton, SIGNAL(clicked()),
            this, SLOT(addNewTriangle()));

    QPushButton *addRectButton = new QPushButton("New rectangle", this, "addRectButton");
    btnLayout->addWidget(addRectButton);
    connect(addRectButton, SIGNAL(clicked()),
            this, SLOT(addNewRectangle()));

    QPushButton *addEllipseButton = new QPushButton("New ellipsoid", this, "addEllipseButton");
    btnLayout->addWidget(addEllipseButton);
    connect(addEllipseButton, SIGNAL(clicked()),
            this, SLOT(addNewEllipsoid()));

    QPushButton *addParaboloidButton = new QPushButton("New paraboloid", this, "addParaboloidButton");
    btnLayout->addWidget(addParaboloidButton);
    connect(addParaboloidButton, SIGNAL(clicked()),
            this, SLOT(addNewParaboloid()));

    deleteButton = new QPushButton("Delete", this, "deleteButton");
    btnLayout->addStretch(1);
    btnLayout->addWidget(deleteButton);
    connect(deleteButton, SIGNAL(clicked()),
            this, SLOT(deleteWidget()));

    QGroupBox *attsGroup = new QGroupBox(this, "attsGroup");
    attsGroup->setTitle("Widget attributes");
    topLayout->addMultiCellWidget(attsGroup, 1, 2, 1, 1);
    QVBoxLayout *attsInnerTopLayout = new QVBoxLayout(attsGroup);
    attsInnerTopLayout->setMargin(10);
    attsInnerTopLayout->addSpacing(15);
    attsInnerTopLayout->setSpacing(10);
    QGridLayout *attsLayout = new QGridLayout(attsInnerTopLayout, 4, 2);
    attsLayout->setSpacing(5);

    wName = new QLineEdit(attsGroup, "wName");
    wName->setEnabled(false); // allow editing name sometime.
    attsLayout->addWidget(new QLabel(wName, "Name", attsGroup, "wNameLabel"), 0, 0);
    attsLayout->addWidget(wName, 0, 1);

    sizeLoc = new QLineEdit(attsGroup, "sizeLoc");
    connect(sizeLoc, SIGNAL(returnPressed()),
            this, SLOT(setSizeLoc()));
    attsLayout->addWidget(new QLabel(sizeLoc, "Size/Location", attsGroup, "sizeLocLabel"), 1, 0);
    attsLayout->addWidget(sizeLoc, 1, 1);
  
    color = new QvisColorButton(attsGroup, "color");
    connect(color, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(setWidgetColor(const QColor &)));
    attsLayout->addWidget(new QLabel(color, "Color", attsGroup, "colorLabel"), 2, 0);
    attsLayout->addWidget(color, 2, 1);

    opacity = new QvisOpacitySlider(attsGroup, "opacity");
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
    s.sprintf("%s%04d", base.latin1(), nameIndex);
    return s;
}

WidgetID
QvisCMap2Widget::getSelectedWidget() const
{
    int index = names->index(names->selectedItem());
    WidgetID id = cmap2->getID(index);
    return id;
}

//
// Qt slots
//
void
QvisCMap2Widget::addNewTriangle()
{
    WidgetID w = cmap2->addTriangleWidget(newName("tri"),
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
    WidgetID w = cmap2->addRectangleWidget(newName("rect"),
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
  WidgetID w = cmap2->addEllipsoidWidget(newName("ellipse"),
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
  WidgetID w = cmap2->addParaboloidWidget(newName("paraboloid"),
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
            names->insertItem(n);
        }
    }

    deleteButton->setEnabled(numWidgets() > 0);
}

void
QvisCMap2Widget::selectLastItem()
{
    names->setCurrentItem(names->count()-1);
}

void
QvisCMap2Widget::deleteWidget()
{
    if(names->selectedItem() != 0)
    {
        int index = names->index(names->selectedItem());
        WidgetID id = cmap2->getID(index);
        if(id != QvisCMap2Display::WIDGET_NOT_FOUND)
        {
            removeWidget(id);
        }
    }
    else
        qDebug("No widget selected");

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
                names->setCurrentItem(i);
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
    opacity->setValue(100. * cmap2->getAlpha(id));
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
