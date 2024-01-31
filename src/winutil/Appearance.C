// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "Appearance.h"
#include <string>

#include <QApplication>
#include <QFont>
#include <QStyle>
#include <QWidget>

#include <AppearanceAttributes.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: QvisGUIApplication::ExtractSystemDefaultAppearance
//
// Purpose: 
//   Obtains Qt's current appearance values (style, font, colors) and stores
//   the values in the "default" section of the input appearance attributes.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Nov 24 13:49:38 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Nov 26 11:14:14 PDT 2008
//   I changed the function so we pass in the application and the appearance
//   attributes that we want to populate.
//
//   Kathleen Biagas, Fri Jan 22 14:06:33 PST 2016
//   We are using styles directly from Qt' Style factory, so no need to 
//   compare against our list of styles, simply remove 'Q' from the beginning
//   and 'Style' from the end of the style name.
// 
// ****************************************************************************

void
GetAppearance(QApplication *a, AppearanceAttributes *aa)
{
    const char *mName = "GetAppearance: ";
    
    // get system default style
    QStyle *style = a->style();
    QString style_class(style->metaObject()->className());

    std::string styleName = style_class.toStdString().substr(1, style_class.length()-6);
    
    debug1 << mName << "Default System Style is: " << styleName << endl;
    aa->SetDefaultStyle(styleName);
    
    // get system default font
    QFont font = a->font();
    std::string font_name = font.key().toStdString();
    debug1 << mName << "Default system font is:" << font_name << endl;
    aa->SetDefaultFontName(font_name);
        
    //set aa colors from defaults
    QColor bg = a->palette().window().color();
    QColor fg = a->palette().windowText().color();
        
    char tmp[20];
    snprintf(tmp, 20, "#%02x%02x%02x", bg.red(), bg.green(), bg.blue());
    debug1 << mName << "Default background color is:" << tmp << endl;
    aa->SetDefaultBackground(tmp);
    snprintf(tmp, 20, "#%02x%02x%02x", fg.red(), fg.green(), fg.blue());
    debug1 << mName << "Default foreground color is:" << tmp << endl;
    aa->SetDefaultForeground(tmp);
    
    debug1 << mName << "Default orientation:" << 0 << endl;
    aa->SetDefaultOrientation(0);
}

// ****************************************************************************
// Method: SetAppearance
//
// Purpose: 
//   Sets the application's style, colors, font based on the values in the
//   appearance attributes.
//
// Arguments:
//   app : The Qt application.
//   aa  : The appearance attributes to use.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 26 11:10:13 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
SetAppearance(QApplication *app, AppearanceAttributes *aa)
{
    const char *mName = "SetAppearance: ";

    // Get the current appearance attributes.
    AppearanceAttributes currentAtts;
    GetAppearance(app, &currentAtts);

    //
    // Set the style
    //
    std::string newStyle(aa->GetStyle());
    if(aa->GetUseSystemDefault())
        newStyle = aa->GetDefaultStyle();
    bool changeStyle = newStyle != currentAtts.GetDefaultStyle();
    if(changeStyle)
    {
        debug1 << mName << "Setting style to: " << newStyle << endl;
        // Set the style via the style name.
        app->setStyle(newStyle.c_str());
    }
     
    //
    // Set the font.
    //
    std::string newFont(aa->GetFontName());
    if(aa->GetUseSystemDefault())
        newFont = aa->GetDefaultFontName();
    bool changeFont = (newFont != currentAtts.GetDefaultFontName()) || changeStyle;
    if(changeFont)
    {
        QFont font;
        bool okay = true;

        if(newFont.size() > 0 &&
           newFont[0] == '-')
        {
            // It's probably an XLFD
            font = QFont(newFont.c_str());
            debug1 << mName << "The font looks like XLFD: "
                   << newFont << endl;
        }
        else
            okay = font.fromString(newFont.c_str());
        
        if(okay)
        {
            
            debug1 << mName << "Font okay. name=" << font.toString().toStdString() << endl;
            app->setFont(font);

            // Force the font change on all top level widgets.
            // for each top level widget...
            foreach(QWidget *w, QApplication::topLevelWidgets())
                w->setFont(font);
        }
        else
            debug1 << mName << "Font NOT okay. name=" << font.toString().toStdString() << endl;
    }

    //
    // Set the application colors
    //
    std::string newBG(aa->GetBackground());
    std::string newFG(aa->GetForeground());
    if(aa->GetUseSystemDefault())
    {
        newBG = aa->GetDefaultBackground();
        newFG = aa->GetDefaultForeground();
    }
    bool changeColors = newBG != currentAtts.GetDefaultBackground() ||
                        newFG != currentAtts.GetDefaultForeground();
    bool colorStyle = newStyle != "macintosh";
    if(changeColors && colorStyle)
    {       
        QColor bg(newBG.c_str());
        QColor fg(newFG.c_str());
        QColor btn(bg);

        // Put the converted RGB format color into the appearance attributes
        // in case the colors were given as color names.
        if(!aa->GetUseSystemDefault())
        {
            char tmp[20];
            sprintf(tmp, "#%02x%02x%02x", bg.red(), bg.green(), bg.blue());
            aa->SetBackground(tmp);
            sprintf(tmp, "#%02x%02x%02x", fg.red(), fg.green(), fg.blue());
            aa->SetForeground(tmp);
        }

        debug1 <<mName << "Setting BG color to: " << newBG << endl;
        debug1 <<mName << "Setting FG color to: " << newFG << endl;

        int h,s,v;
        fg.getHsv(&h,&s,&v);
        QColor base = Qt::white;
        bool bright_mode = false;
        if(v >= 255 - 50)
        {
            base = btn.darker(150);
            bright_mode = true;
        }

        QPalette pal(fg, btn, btn.lighter(),
                     btn.darker(), btn.darker(150), fg, Qt::white, base, bg);
//        pal.setCurrentColorGroup(QPalette::Normal);
        
        if (bright_mode)
        {
//            pal.setColor(QPalette::Text, fg);
//            pal.setColor(QPalette::WindowText, fg);
            pal.setColor(QPalette::Normal, QPalette::HighlightedText, base );
            pal.setColor(QPalette::Normal, QPalette::Highlight, Qt::white );
        }
        else
        {
//            pal.setColor(QPalette::Text, fg);
//            pal.setColor(QPalette::WindowText, fg);
            pal.setColor(QPalette::Normal, QPalette::HighlightedText, Qt::white );
            pal.setColor(QPalette::Normal, QPalette::Highlight, Qt::darkBlue );
        }
        
        // Set inactive colors
//        pal.setCurrentColorGroup(QPalette::Inactive);
        if (bright_mode)
        {
//            pal.setColor(QPalette::Inactive, QPalette::Text, fg);
//            pal.setColor(QPalette::Inactive, QPalette::WindowText, fg);
            pal.setColor(QPalette::Inactive, QPalette::HighlightedText, base );
            pal.setColor(QPalette::Inactive, QPalette::Highlight, Qt::white );
        }
        else
        {
//            pal.setColor(QPalette::Inactive, QPalette::Text, fg);
//            pal.setColor(QPalette::Inactive, QPalette::WindowText, fg);
            pal.setColor(QPalette::Inactive, QPalette::HighlightedText, Qt::white );
            pal.setColor(QPalette::Inactive, QPalette::Highlight, Qt::darkBlue );
        }

        // Set disabled colors
        QColor disabled((fg.red()+btn.red())/2,
                        (fg.green()+btn.green())/2,
                        (fg.blue()+btn.blue())/2);
        
//        pal.setCurrentColorGroup(QPalette::Disabled);
        pal.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
        pal.setColor(QPalette::Disabled, QPalette::Light, btn.lighter( 125 ));
        pal.setColor(QPalette::Disabled, QPalette::Text, disabled);
        pal.setColor(QPalette::Disabled, QPalette::Base, Qt::white);
        if (bright_mode)
        {
            pal.setColor(QPalette::Disabled, QPalette::HighlightedText, base);
            pal.setColor(QPalette::Disabled, QPalette::Highlight, Qt::white);
        }
        else
        {
            pal.setColor(QPalette::Disabled, QPalette::HighlightedText, Qt::white);
            pal.setColor(QPalette::Disabled, QPalette::Highlight, Qt::darkBlue);
        }

        app->setPalette(pal);
    }
}
