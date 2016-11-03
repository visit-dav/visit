.. _Preferences_How_to_save_your_settings:

How to save your settings
-------------------------

To save preferences in VisIt, you need only to select **Save settings**
from the **Main Window's Options** menu. When you choose this option,
VisIt saves the current settings to your preferences file so you can
use them the next time you run VisIt. VisIt does not automatically save
your settings when you make changes to default attributes for plots,
operators, or various controls windows. If a VisIt window provides a
**Make default** button, the settings for that window are divided
into current settings and default settings. You must click the
**Make default** button to ensure that VisIt will copy the current
settings over the default settings. Once you have set your default
settings appropriately, save your settings.

If you want to save more than just the default attributes such as the
plots in your plot list and where your windows are located, you should
save a session file in addition to saving your settings. To save a
session file, select the **Save session** option from the
main window's **File** menu, and select a session file name. Once
you select a name for your session file, VisIt saves out an XML
description of your VisIt session that includes all of the information
required to reproduce it.

VisIt saves two preference files, the first of which stores preferences
for VisIt's GUI while the second file stores preferences for VisIt's
viewer. When running VisIt on UNIX, MacOS X systems, the preference files
are called: ``guiconfig`` and ``config`` and they are saved in the ``.visit``
directory in your home directory. The Windows version of the ``.visit`` 
directory is ``%USERPROFILE%\Documents\VisIt``, which may be something like: 
``C:\Users\<your-user-name>\Documents\VisIt``.

If you prefer to run VisIt without reading your saved settings, you can
provide the ``-noconfig`` option on the command line when you run VisIt.
The ``-noconfig`` argument is often useful when you run the UNIX
version and you get an updated version of VisIt that is incompatible with
your saved settings. VisIt settings are usually compatible between
different versions but this is not always the case and some users have
had trouble on occasion when transitioning to a newer version. If you
find that VisIt has stability problems when it starts up, provide the
``-noconfig`` option and then save your settings to write over any older
preference files.
