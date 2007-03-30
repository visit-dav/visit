#ifndef QVIS_PLOT_LISTBOX_ITEM_H
#define QVIS_PLOT_LISTBOX_ITEM_H
#include <gui_exports.h>
#include <qlistbox.h>
#include <Plot.h>
#include <vector>

// ****************************************************************************
// Class: QvisPlotListBoxItem
//
// Purpose:
//   This is a list item that can be inserted into a QListBox. The
//   PlotList that is displayed in the GUI is made of these.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 12:18:41 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 8 11:16:36 PDT 2003
//   I renamed the class to QvisPlotListBoxItem and made big changes to it
//   to allow expanded display of plots.
//
// ****************************************************************************
    
class GUI_API QvisPlotListBoxItem : public QListBoxItem
{
public:
    typedef enum {
        Expand,
        Subset,
        PlotAttributes,
        OperatorAttributes,
        Promote,
        Demote,
        Delete,
        ActiveOperator
    } ClickObjectType;

protected:
    class GUI_API ClickableRectangle
    {
    public:
        ClickableRectangle();
        ClickableRectangle(int, const QRect &, ClickObjectType);
        ClickableRectangle(const ClickableRectangle &obj);
        ~ClickableRectangle();
        void operator = (const ClickableRectangle &obj);

        QRect           r;
        ClickObjectType type;
        int             id;
    };

    typedef std::vector<ClickableRectangle> ClickableRectangleVector;
public:
    QvisPlotListBoxItem(const QString &prefix, const Plot &plot);
   ~QvisPlotListBoxItem();

    int    height(const QListBox *) const;
    int    width(const QListBox *)  const;

    int    clicked(const QPoint &p, bool dc, int &id);
    bool   isExpanded() const;
    int    activeOperatorIndex() const;

    const Plot &GetPlot() const { return plot; };
protected:
    void paint(QPainter *);
private:
    QString GetDisplayString(const Plot &plot, const QString &prefix);
    void GetOperatorPixmap(int operatorType, QPixmap &pm);
    void GetPlotPixmap(int plotType, QPixmap &pm);
    void drawButtonSurface(QPainter *painter, const QRect &r) const;
    void drawUpDownButton(QPainter *painter, const QRect &r, bool up) const;
    void drawDeleteButton(QPainter *painter, const QRect &r) const;
    void AddClickableRectangle(int id, const QRect &r, ClickObjectType type);
    void setTextPen(QPainter *painter, bool highlightText) const;
private:
    Plot                     plot;

    QString                  prefix;
    QPixmap                  plotIcon;
    QString                  plotName;
    QPixmap                 *operatorIcons;
    QString                 *operatorNames;
    int                      maxIconWidth, maxIconHeight;
    ClickableRectangleVector clickable;
    bool                     addClickableRects;

    static QPixmap          *subsetIcon;
};

#endif
