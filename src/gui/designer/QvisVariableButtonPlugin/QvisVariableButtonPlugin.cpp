#include "QvisVariableButton.h"
#include "QvisVariableButtonPlugin.h"

#include <QtPlugin>

QvisVariableButtonPlugin::QvisVariableButtonPlugin(QObject *parent)
    : QObject(parent)
{
    initialized = false;
}

void QvisVariableButtonPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool QvisVariableButtonPlugin::isInitialized() const
{
    return initialized;
}

QWidget *QvisVariableButtonPlugin::createWidget(QWidget *parent)
{
    return new QvisVariableButton(parent);
}

QString QvisVariableButtonPlugin::name() const
{
    return "QvisVariableButton";
}

QString QvisVariableButtonPlugin::group() const
{
    return "VisIt";
}

QIcon QvisVariableButtonPlugin::icon() const
{
    return QIcon();
}

QString QvisVariableButtonPlugin::toolTip() const
{
    return "variable button";
}

QString QvisVariableButtonPlugin::whatsThis() const
{
    return "Lets the user select a variable from a set of menus.";
}

bool QvisVariableButtonPlugin::isContainer() const
{
    return false;
}

QString QvisVariableButtonPlugin::domXml() const
{
    return "<widget class=\"QvisVariableButton\" name=\"QvisVariableButton\">\n"
           " <property name=\"geometry\">\n"
           "  <rect>\n"
           "   <x>0</x>\n"
           "   <y>0</y>\n"
           "   <width>100</width>\n"
           "   <height>22</height>\n"
           "  </rect>\n"
           " </property>\n"
           "</widget>\n";
}

QString QvisVariableButtonPlugin::includeFile() const
{
    return "QvisVariableButton.h";
}

Q_EXPORT_PLUGIN2(QvisVariableButtonPlugin, QvisVariableButtonPlugin)
