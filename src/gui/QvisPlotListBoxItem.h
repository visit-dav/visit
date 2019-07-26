// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_PLOT_LISTBOX_ITEM_H
#define QVIS_PLOT_LISTBOX_ITEM_H
#include <gui_exports.h>
#include <QListWidget>
#include <GUIBase.h>
#include <Plot.h>
#include <vector>

// ****************************************************************************
// Class: QvisPlotListBoxItem
//
// Purpose:
//   This is a list item that can be inserted into a QListWidget. The
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
//   Brad Whitlock, Wed Jul 28 17:37:15 PST 2004
//   I added a method to return the prefix.
//
//   Brad Whitlock, Fri Feb 23 14:15:12 PST 2007
//   Added non const GetPlot method.
//
//   Brad Whitlock, Tue Jun 24 12:13:41 PDT 2008
//   Added GUIBase as a private base class so we can access the plugin managers.
//
//   Brad Whitlock, Fri Jul 23 14:55:01 PDT 2010
//   I added selectionName.
//
//   Brad Whitlock, Thu Mar 14 16:21:47 PDT 2013
//   Added applyOperators.
//
// ****************************************************************************
    
class GUI_API QvisPlotListBoxItem : public QListWidgetItem, private GUIBase
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
        ActiveOperator,
        Selection
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
    QvisPlotListBoxItem(const Plot &plot, const QString &prefix,  
                        const QString &selName);
   ~QvisPlotListBoxItem();

    int    height(const QListWidget *) const;
    int    width(const QListWidget *)  const;

    int    clicked(const QPoint &p, bool dc, int &id);
    bool   isExpanded() const;
    int    activeOperatorIndex() const;
    int    textX() const;

    void   setApplyOperators(bool);

    const Plot &GetPlot() const { return plot; };
    Plot &GetPlot() { return plot; };
    const QString &GetPrefix() const { return prefix; };
    const QString &GetSelectionName() const { return selectionName; }

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
    QString                  selectionName;

    QPixmap                  plotIcon;
    QString                  plotName;
    QPixmap                 *operatorIcons;
    QString                 *operatorNames;
    int                      maxIconWidth, maxIconHeight;
    ClickableRectangleVector clickable;
    bool                     addClickableRects;
    bool                     applyOperators;

    static QPixmap          *subsetIcon; 
    static QPixmap          *selectionIcon;
};

#endif
