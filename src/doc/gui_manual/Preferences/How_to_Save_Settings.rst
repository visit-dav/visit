.. _How to Save Settings:

How to Save Settings
--------------------

To save preferences in VisIt_, select **Save settings** from the **Main**
window's **Options** menu. When VisIt_ saves the current settings to the
users preferences file they are used to set the initial state the next
time the user runs VisIt_. VisIt_ does not automatically save settings
when changes are made to the default attributes for plots, operators, or
various control windows. For windows that only have current attributes
(windows without a **Make default** button), the current attributes are
saved. For windows that have current and default attributes (windows with
a **Make default** button), the default attributes are saved.

To save the entire state of VisIt_, which includes things such as the plots
in the window and the operators applied to the plots for each visualization
window, select either **Save session** or **Save session as** from the **Main**
window's **File** menu. When using **Save session**, if a session has already
been restored or saved, VisIt_ will overwrite the existing session file. If
a session has not already been restored or saved, VisIt_ will bring up a
dialog window that will allow the user to specify the location and name of
the session file. When using **Save session as** VisIt_ will always bring
up a dialog window that will allow the user to specify the location and name
of the session file and prompt the user to confirm before overwriting an
existing session file.

VisIt_ saves two preference files, the first of which stores preferences
for VisIt_'s GUI while the second file stores preferences for VisIt_'s
state. When running VisIt_ on UNIX and MacOS X systems, the preference files
are called: ``guiconfig`` and ``config`` and they are saved in the ``.visit``
directory in the users home directory. The Windows version of the ``.visit`` 
directory is ``%USERPROFILE%\Documents\VisIt``, which may be something like: 
``C:\Users\<your-user-name>\Documents\VisIt``.

To run VisIt_ without reading the saved settings, add ``-noconfig`` to the
command line when running VisIt_.  The ``-noconfig`` argument is often
useful when running an updated version of VisIt_ that is incompatible with
the saved settings. VisIt_ settings are usually compatible between different
versions but this is not always the case and some users have had trouble
on occasion when transitioning to a newer version. If VisIt_ has stability
problems when it starts up after upgrading to a newer version, add the
``-noconfig`` option to the command line and save the settings to write over
any older preference files.
