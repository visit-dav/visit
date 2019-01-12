#include "QvisSpectrumBar.h"
#include "QvisSpectrumBarPlugin.h"

#include <QtPlugin>

QvisSpectrumBarPlugin::QvisSpectrumBarPlugin(QObject *parent)
    : QObject(parent)
{
    initialized = false;
}

void QvisSpectrumBarPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool QvisSpectrumBarPlugin::isInitialized() const
{
    return initialized;
}

QWidget *QvisSpectrumBarPlugin::createWidget(QWidget *parent)
{
    return new QvisSpectrumBar(parent);
}

QString QvisSpectrumBarPlugin::name() const
{
    return "QvisSpectrumBar";
}

QString QvisSpectrumBarPlugin::group() const
{
    return "VisIt";
}

QIcon QvisSpectrumBarPlugin::icon() const
{
    return QIcon();
}

QString QvisSpectrumBarPlugin::toolTip() const
{
    return "spectrum widget";
}

QString QvisSpectrumBarPlugin::whatsThis() const
{
    return "Lets the user design piecewise linear color tables using color control points";
}

bool QvisSpectrumBarPlugin::isContainer() const
{
    return false;
}

QString QvisSpectrumBarPlugin::domXml() const
{
    return "<widget class=\"QvisSpectrumBar\" name=\"QvisSpectrumBar\">\n"
           " <property name=\"geometry\">\n"
           "  <rect>\n"
           "   <x>0</x>\n"
           "   <y>0</y>\n"
           "   <width>200</width>\n"
           "   <height>60</height>\n"
           "  </rect>\n"
           " </property>\n"
           "</widget>\n";
}

QString QvisSpectrumBarPlugin::includeFile() const
{
    return "QvisSpectrumBar.h";
}

Q_EXPORT_PLUGIN2(QvisSpectrumBarPlugin, QvisSpectrumBarPlugin)
