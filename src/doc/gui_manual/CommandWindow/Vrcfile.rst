.. _visitrc_file:

VisIt Run Commands (RC) File
----------------------------

VisIt_ supports a `run commands <https://en.wikipedia.org/wiki/Run_commands>`_
or *rc* file called the ``visitrc`` file. It is located in the VisIt_ user
directory, which on most platforms is ``~/.visit``. The ``visitrc`` file
is a Python source code file that contains Python scripting commands that VisIt_
executes whenever it is started (either via the GUI or the CLI).

If the VisIt_ GUI detects the presence of the ``visitrc`` file, then the CLI
will also be launched every time VisIt_ starts up, setting up your VisIt_
session so that both the GUI and CLI are connected to the viewer.

The ``visitrc`` file is most often used to define Python functions that will be
called when buttons are clicked in VisIt_'s ``'Macros``' window. However, users
can use the file to run whatever Python code they wish during VisIt_ startup.
This could include opening a frequently used database, defining a set of
frequently used expressions, etc. See the
:ref:`Python command-line interface <PythonManual>` manual for more information
about the commands available in VisIt_'s Python interface.
