.. _Session files:

Session files
-------------

A session file is an XML file that contains all of the necessary information to
recreate the plots and visualization windows used in a VisIt session. You can
set up complex visualizations, save a session file, and then run a new VisIt
session later and be able to pick up exactly where you left off when you saved
the session file. If you often look at the same types of plots with the same
complex setup then you should save a session file for your visualization once
it is set up so you don't have to do any manual setup in the future.

Saving session
~~~~~~~~~~~~~~

Once you have set up your plots, you can select **Save session** option in the
**Main Window's File** menu to open up a **Save file** dialog. Once the
**Save file** dialog is opened, select the location and filename that you want
to use to store the session file. By default, VisIt stores all session files
in your .visit directory on UNIX and MacOS X computers and in the directory
where VisIt was installed on Windows computers. Once you select the location
and filename to use when saving the session file, VisIt writes an XML
description of the complete state of all vis windows, plots, and GUI windows
into the session file so the next time you come into VisIt, you can completely
restore your VisIt session.

Restoring session
~~~~~~~~~~~~~~~~~

Restoring a VisIt session file deletes all plots, closes all databases,
etc before VisIt reads the session file to get back to the state described
in the session file. After restoring a session file, VisIt will look exactly
like it did when the session file was saved. To restore a session file, click
the **Restore session** option from the **Main Window's File** menu to open
an **Open file** dialog. Choose a session file to open using the **Open file**
dialog. Once a file is chosen, VisIt restores the session using the
selected session file. If you are on the Windows platform, you can
double-click session files (.vses files) stored on your computer in order to
directly open them with VisIt.
