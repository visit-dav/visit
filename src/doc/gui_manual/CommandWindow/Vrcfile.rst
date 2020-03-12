.. _visitrc_file:

VisIt Run Commands (RC) File
----------------------------

VisIt_ supports a `run commands <https://en.wikipedia.org/wiki/Run_commands>`_
or an *rc* file called the ``visitrc`` file which is typically
:ref:`located <file_locations_visitrc>` in ``~/.visit``. The ``visitrc`` file
is a Python source code file that contains Python scripting commands that VisIt_
executes whenever the CLI is started either from the shell or from within the
GUI through the :ref:`Command Window <Command_Line_Window>`.

The ``visitrc`` file is most often used to define Python functions associated
with VisIt_ :ref:`macros <command_macros>`. However, users can use the file to
run whatever Python code they wish during VisIt_ CLI startup. This could include
opening a frequently used database, defining a set of frequently used expressions,
etc. See the :ref:`Python command-line interface <PythonManual>` manual for more
information about the commands available in VisIt_'s Python interface.
