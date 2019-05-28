.. _How VisIt Uses Preferences:

How VisIt_ Uses Preferences
---------------------------

VisIt_'s preferences are saved into two levels of XML files that are stored
in the user's home directory and in the global VisIt_ installation directory.
The global preferences are read first and they allow the system administrator
to set global preferences for all users. After VisIt_ reads the global
preferences, it reads the preferences file for the current user. These
settings include things like the color of the GUI and the initial directory
from which to read files. Most of the attributes that are settable in VisIt_
can be saved to the preferences files for future VisIt_ sessions.
