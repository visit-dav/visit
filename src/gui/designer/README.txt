This directory contains subdirectories that each contain a Qt Designer plugin.
Each plugin exposes a VisIt custom widget as a Designer plugin so that the
widget is available within designer and can be used to create Forms containing
custom widgets.

Building a plugin
==================
Follow these steps to build a plugin and install it into your Qt Designer:

cd QvisSpectrumBarPlugin
qmake
make
make install
