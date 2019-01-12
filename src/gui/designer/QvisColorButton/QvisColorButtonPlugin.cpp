 #include "QvisColorButton.h"
 #include "QvisColorButtonPlugin.h"

 #include <QtPlugin>

 QvisColorButtonPlugin::QvisColorButtonPlugin(QObject *parent)
     : QObject(parent)
 {
     initialized = false;
 }

 void QvisColorButtonPlugin::initialize(QDesignerFormEditorInterface * /* core */)
 {
     if (initialized)
         return;

     initialized = true;
 }

 bool QvisColorButtonPlugin::isInitialized() const
 {
     return initialized;
 }

 QWidget *QvisColorButtonPlugin::createWidget(QWidget *parent)
 {
     return new QvisColorButton(parent);
 }

 QString QvisColorButtonPlugin::name() const
 {
     return "QvisColorButton";
 }

 QString QvisColorButtonPlugin::group() const
 {
     return "VisIt";
 }

 QIcon QvisColorButtonPlugin::icon() const
 {
     return QIcon();
 }

 QString QvisColorButtonPlugin::toolTip() const
 {
     return "color button";
 }

 QString QvisColorButtonPlugin::whatsThis() const
 {
     return "A button that lets the user pick a color from a menu";
 }

 bool QvisColorButtonPlugin::isContainer() const
 {
     return false;
 }

 QString QvisColorButtonPlugin::domXml() const
 {
     return "<widget class=\"QvisColorButton\" name=\"QvisColorButton\">\n"
            " <property name=\"geometry\">\n"
            "  <rect>\n"
            "   <x>0</x>\n"
            "   <y>0</y>\n"
            "   <width>50</width>\n"
            "   <height>20</height>\n"
            "  </rect>\n"
            " </property>\n"
            "</widget>\n";
 }

 QString QvisColorButtonPlugin::includeFile() const
 {
     return "QvisColorButton.h";
 }

 Q_EXPORT_PLUGIN2(VisItCustom, QvisColorButtonPlugin)
