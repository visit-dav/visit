.. _file_locations:

Configuration File Locations
----------------------------

VisIt_ reads and writes information about aspects of its *configuration*
to various files. In most cases where VisIt_ provides controls to either save
or load some aspect of its configuration, the user is presented with a file
browser dialog and can explicitly choose arbitrary locations on the file system
to look for or store files. However, this is not universally true and VisIt_
does have *prescribed* locations for some configuration files. In this section
we provide some additional details about these various files.

The locations of prescribed files may depend on the platform upon which VisIt_
is running. Typically, on UNIX and OSX systems, prescribed configuration files
are stored in ``~/.visit`` whereas on Windows systems, they are, by default, in
``%USERPROFILE%\Documents\VisIt``, which may be something like
``C:\Users\<user-name>\Documents\VisIt``. Furthermore, on Windows, Visit_ honors
the ``CSIDL_PERSONAL`` and ``CSIDL_MYDOCUMENTS``
`CSIDL environment variables <https://docs.microsoft.com/en-us/windows/win32/shell/csidl>`_.
Depending on the how the system is configured, these might actually resolve to a
networked drive, but most commonly, to the values described previously. Finally,
Windows users can also set the ``VISITUSERHOME`` environment variable to point
to whatever location they desire, and VisIt_ will use that location instead.
In the descriptions below, we use the symbol ``VUSER_HOME`` as a way to refer to
whatever this location happens to be on whatever platform the user is running.

In addition, when running VisIt in :ref:`client/server mode <Client-Server Mode>`,
the user may need to be aware of the location of these files on either the client
or the server or both.

Files in ``VUSER_HOME``
~~~~~~~~~~~~~~~~~~~~~~~
Most of the files associated with VisIt_ configuration have their prescribed home
in ``VUSER_HOME``. When running in client/server, it is the configuration files
on the *local client* that effect behavior. This means they are always the
files on the *local* machine and not the *remote* system. Any configuration
files that might also be on the remote server do not play a role in effecting
behavior in client/server mode.

Settings/Preferences File
"""""""""""""""""""""""""
* Location and file name: ``VUSER_HOME/config`` 
* Purpose: Holds user settings from :ref:`preferences window <Preferences Window>`
  plus numerous other settings such as default attributes for operators and plots,
  default database read options, default color tables, as well as the
  enabled/disabled state of various plot, operator and database plugins.
* Written: when user :ref:`saves settings <How to Save Settings>`.
* Read: on VisIt_ startup but this can be overridden by the ``-noconfig``
  command-line :ref:`startup option <StartupOptions>`.
* Format: ASCII `XML <https://en.wikipedia.org/wiki/XML>`_

GUI Configuration File
""""""""""""""""""""""
* Location and file name: ``VUSER_HOME/guiconfig``
* Purpose: Holds positions and sizes of various GUI windows. Also holds the list of
  recently used paths to open databases.
* Otherwise operates identically to ``VUSER_HOME/config``.

Host Profile Files
""""""""""""""""""
* Location and file name(s): ``VUSER_HOME/host/host_<site-name>_<resource-name>.xml``
* Purpose: Stores information on how to connect to and launch jobs on various
  compute resources. In many cases, there are separate sets of host profile files
  for all the compute resources at a commonly used site such as LLNL CZ or RZ,
  ANL, ORNL, etc. Often sites will *post* VisIt_ host profile files in places for
  users to easily find and *install* them. Installing them is just a matter of
  copying them to ``VUSER_HOME``.
* Written: when user :ref:`saves settings <How to Save Settings>`.
* Read: on VisIt_ startup. All host profiles in ``VUSER_HOME/host*.xml`` are read
  on VisIt_ startup but this can be overridden by ``-noconfig``. Users should be
  aware of this behavior. If the user passes ``-noconfig`` for the purposes of
  avoiding the loading of preferences, s/he will also be without any host profiles.
* Format: ASCII `XML <https://en.wikipedia.org/wiki/XML>`_

.. _file_locations_visitrc:

VisIt Run Commands (rc) File
""""""""""""""""""""""""""""
* Location and file name: ``VUSER_HOME/visitrc``
* Purpose: Holds Python code to be executed each time VisIt_ is launched.
* Written: Whenever user hits the **Update Macros** button in the
  :ref:`**Command Window** <Command_Line_Window>`.
* Read: on VisIt_ startup of the CLI.
* Format: Python source code. However, there is no ``.py`` file extension in the
  file name.

Command Window Tabs Script Files
""""""""""""""""""""""""""""""""
* Location and file name(s): ``VUSER_HOME/script<K>.py`` where ``K`` is an
  integer in the range [1...8].
* Purpose: Hold the python code associated with each tab in the
  :ref:`**Command Window** <Command_Line_Window>`.
* Written: when user :ref:`saves settings <How to Save Settings>`.
* Read: on VisIt_ startup but this can be overridden by the ``-noconfig``
* Format: Python source code.

Color Table Files
"""""""""""""""""
* Location and file name(s): ``VUSER_HOME/<color-table-name>.ct``
* Purpose: Store a single color table for easy sharing with other users.
* Written when the user hits the **Export** button in the
  :ref:`color table window <fig-MakingItPretty-ColorTables>` from
  :menuselection:`Controls -> Color table...`.
* Read: on VisIt_ startup. All color table files in ``VUSER_HOME/*.ct``
  are read and loaded into VisIt_. However, this behavior is overridden
  by ``-noconfig``.
* Format: ASCII `XML <https://en.wikipedia.org/wiki/XML>`_ specifying the
  :ref:`colors and color control points <Color_tables>` for the color table.

Custom Plugins
""""""""""""""
* Location and file name(s): There are separate directories in ``VUSER_HOME``
  for operator, database and plot plugins. These are

  * ``VUSER_HOME/<visit-version>/<visit-arch>/plugins/operators/``
  * ``VUSER_HOME/<visit-version>/<visit-arch>/plugins/databases/``
  * ``VUSER_HOME/<visit-version>/<visit-arch>/plugins/plots/``

  where ``<visit-version>`` and  ``<visit-arch>`` are the VisIt_ version number
  and VisIt_ architecture moniker. A single plugin involves a set of related
  files for the mdserver, engine and those common to both.
* Purpose: Directories to hold custom (e.g. user-compiled and installed)
  plugins.
* Written: When the user
  `makes and installs a custom plugin <http://visitusers.org/index.php?title=Building_plugins_using_CMake>`_
* Read: On VisIt_ startup, all plugin *info* files are read. The remaining
  parts of the plugin are read only when the plugin is actually used.
* Format: Binary shared library files in the machine format of the host
  architecture.

State Tracking Files
""""""""""""""""""""
* Location and file name(s): ``VUSER_HOME/stateA.B.C.txt`` where ``A``,
  ``B`` and ``C`` form a VisIt_ version number.
* Purpose: Holds a single ASCII integer indicating the number of times the
  associated VisIt_ version has been run. This is to facilitate suppression of
  the release notes and help upon the *first* run of a new version of VisIt_.
* Written: each time VisIt_ is started, the integer value in the associated
  state tracking file is updated.
* Read: each time VisIt_ is started, the value in the associated state tracking
  file is read.
* Format: ASCII text

Crash Recovery Files
""""""""""""""""""""
* Location and file name(s): ``VUSER_HOME/crash_recovery.$pid.session`` and
  ``VUSER_HOME/crash_recovery.$pid.session.gui`` where ``$pid`` is the process
  id of the VisIt_ viewer component.
* Purpose: Hold the most recently saved last good state of VisIt_ prior
  to a crash. Disabled if the preference
  ``Periodically save a crash recovery file`` is unchecked in the
  **Preferences Window**.
* Written: periodically from VisIt_ automatically.
* Read: when user starts VisIt_ and answers ``yes`` when queried whether to
  start up from the most recent crash recovery file or when user explicitly
  specifies the crash recovery file as an argument to the ``-sessionfile``
  command-line :ref:`startup option <StartupOptions>`.
* Format: ASCII `XML <https://en.wikipedia.org/wiki/XML>`_, same as any
  other VisIt_ :ref:`session files <Session files>`.

Files In Other Locations
~~~~~~~~~~~~~~~~~~~~~~~~

There are several other kinds of files VisIt_ reads and writes to locations
other than ``VUSER_HOME``. These are breifly described here.

VisIt Debug Log (``.vlog``) Files
"""""""""""""""""""""""""""""""
* Location and file name(s): The location of these files depends on whether
  VisIt_ is being run in :ref:`client/server mode <Client-Server Mode>`.
  When running client/server, some logs are written on the client and some on
  the server. The logs on the client are written to whatever the current working
  directory was when VisIt_ was started. If started from an icon, this is most
  likely the the user's home directory. If started from a command-line, it is
  whatever the shell's current working directory for that command-line was. On
  the server, the logs are written to the user's login (home) directory. In a
  typical client/server scenario, the user gets gui and viewer logs locally in
  the current working directory and mdserver and engine logs on the remote
  system in their login (home) directory. In a purley local scenario, all logs
  are written to the current working directory.

  The names of the log files are of the form
  ``<letter>.<component-name>.<-mpi-rank-or-$pid>.<debug-level>.vlog`` where
  ``<letter>`` is one of ``A`` through ``E``, ``<component-name>`` is one of
  ``gui``, ``mdserver``, ``viewer``, ``engine_ser``, ``engine_par``,
  ``<mpi-rank-or-$pid>`` is the MPI rank for a prallel engine (``engine_par``)
  or, optionally if ``-pid`` is given as a command-line
  :ref:`startup option <StartupOptions>`) the component's process id,
  and ``<debug-level>`` is the integer argument for the ``-debug``
  command-line :ref:`startup option <StartupOptions>`. For example the file
  names are ``A.mdserver.5.vlog`` or ``C.engine_par.123.2.vlog``.
* Purpose: Capture streaming debugging messages from various VisIt_ components.
* Written continuously by VisIt if ``-debug L`` where ``L`` is the debug *level*
  and is an integer in the range ``[1...5]`` is given on the command-line that
  starts VisIt_ or buffered if a ``b`` is given immediately afte the debug level
  integer. In addition, VisIt_ maintains the 5 most recently written logs from
  the 5 most recent component executions each beginning with the letters ``A``
  through ``E``, ``A`` being the most recent.
* Format: Various, ad-hoc ASCII, mostly human readable.

Session Files
"""""""""""""
* Location and file name(s): User is prompted with a file browser to select
  the name and location of these files. The ``-sessionfile`` 
  command-line :ref:`startup option <StartupOptions>` can be used to select
  a session file to open at startup.
* Purpose: :ref:`Session files <Session files>` are used to save and restore the
  entire state of a VisIt_ session.
* Written: On demand when user selects :menuselection:`File --> Save session...`
* Read: On demand when user selects :menuselection:`File --> Restor session...`
* Format: ASCII `XML <https://en.wikipedia.org/wiki/XML>`_.

Database Files
""""""""""""""
* Location and file name(s): User uses the :menuselection:`File --> Open...`
  file browser to select the location of these files. The ``-o`` 
  command-line :ref:`startup option <StartupOptions>` can be used to select
  a database file to open at startup.
* Purpose: Database files store the data that VisIt_ is used to analyze and
  visualize.
* Written: VisIt_ *reads* over 130 different types of databases. Only about
  20 of those types can be *written* by VisIt_. And some of those output
  types support only limited kinds of data. A database can be written using
  :menuselection:`File --> Export database...`
* Read: On demand when user selects :menuselection:`File --> Open...`
* Format: Varies by database type.

Save Window Files
"""""""""""""""""
* Location and file name(s): User uses the :menuselection:`File --> Save Window...`
  file browser to select the location of these files.
  and :menuselection:`File -> Set save options...` to specify other properties.
* Purpose: Save the data displayed in the currently active window usually but
  not always to an image file.
* Written: On demand user selects :menuselection:`File --> Save Window...` or
  hits the **Save** button in the :ref:`Set save options <saving_viz_window>`
  window.
* Read: Yes, saved images can be read into VisIt_ like any other database.
  On demand when user selects :menuselection:`File --> Open...`
* Format: Various, see :ref:`Set save options <saving_viz_window>` window.

Save Window vs. Export Database Files
"""""""""""""""""""""""""""""""""""""
As far as file locations are concerned, the key issue for users to keep in 
mind regarding **Save Window** operations and **Export Database** operations
has to do with client/server operation. In client/server mode, **Save Window**
produces files always on the client whereas **Export Database** produces files
always on the server.

The **Save Window** and **Export Database** operations can in some circumstances
be highly similar and confusing to decide which to use. In general, the
**Save Window** operation is to save an *image* file whereas the **Export Database**
operation is to output a wholly new VisIt_ *database* file. The cases where
these two operations are blurred is when non-image formats are used by
**Save Window** such as STL, VTK, OBJ, PLY (3D formats) and Curve or Ultra
(2D, xy curve formats) formats.

Temporarily Adjusting Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sometimes, users need to temporarily change their configuration either to work
around or diagnose an issue. Since the majority of content in these files is
ASCII, it is possible to manually edit files without having to start VisIt_.

The user can also move (or rename) files so that VisIt_ will either find or not
find them. For example, a common trick is for users to change the name of
``VUSER_HOME/config`` to ``VUSER_CONFIG/config.orig`` so that the majority of
*settings/preferences* are not seen during VisIt_ startup. The most dramatic
variation of this approach is to move the whole ``VUSER_HOME`` directory which
on UNIX platforms would be a command like ``mv ~/.visit ~/.visit.old``.
