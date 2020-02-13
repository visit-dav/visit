.. _file_locations:

Locations of various files
--------------------------

VisIt_ reads and writes information about various aspects of its *configuration*
to various files. In most cases where VisIt_ provides controls to either save
or load some aspect of its configuration, the user is presented with a file
browser dialog and can explicitly choose where to look for or store files.
However, this is not universally true and VisIt_ does have prescribed locations
for some configuration files. The locations of prescribed files may depend on
the platform upon which VisIt_ is running.

Typically, on UNIX and OSX systems, prescribed configuration files are stored
in ``~/.visit`` whereas on Windows systems, they are, by default, in
``%USERPROFILE%\Documents\VisIt``, which may be something like
``C:\Users\<user-name>\Documents\VisIt``. Furthermore, on Windows, Visit_ reads
the ``CSIDL_PERSONAL`` and ``CSIDL_MYDOCUMENTS`` environment variables
(aka *known folder*). Depending on the how the system is configured, that might
actually resolve to a networked drive, but most commonly, to above.
Finally, Windows users can set ``VISITUSERHOME`` environment variable to point
to whatever location they desire, and VisIt_ will use that instead.

In the
descriptions below, we use the symbol ``VUSER_HOME`` as a platform-indepenent
way to refer to whatever this location happens to be.

Finally, when running VisIt in client/server mode, the user may need to be aware
of the location of these files on either the client or the server or both.

Below, for each type of configuration file VisIt_ manages, we provide some more
details about the associated locations.

Files in ``VUSER_HOME``
~~~~~~~~~~~~~~~~~~~~~~~
In all cases for files in ``VUSER_HOME``, these files are managed only by the
VisIt_ client when running in client/server mode. This means they are always
in the *local* machine and not the *remote* system.

Settings/Preferences File
"""""""""""""""""""""""""
* Location: ``VUSER_HOME/config`` 
* Purpose: Holds user settings from :ref:`preferences window <Preferences Window>`
  plus numerous other settings such as default attributes for operators and plots,
  default database read options, default color tables, enabled/disabled state of
  various plot, operator and database plugins
* Written when user takes the GUI action: :menuselection:`Options --> Save Settings`
* Read on VisIt_ startup but this can be overridden by ``-noconfig``
  command-line :ref:`startup option <StartupOptions>`
* Format: ASCII `XML <https://en.wikipedia.org/wiki/XML>`_ which means that often
  the file can be manually edited with a text editor to make quick adjustments.

GUI Configuration File
""""""""""""""""""""""
* Location: ``VUSER_HOME/guiconfig``
* Purpose: Holds position and sizes of various GUI windows. Also holds the list of
  recently used paths to open databases.
* Otherwise managed identically to ``config``.

State Tracking Files
""""""""""""""""""""
* Location: ``VUSER_HOME/stateA.B.C.txt`` where ``A``, ``B`` and ``C`` form a VisIt_
  version number
* Purpose: Holds a single ASCII integer indicating the number of times the
  associated VisIt_ version has been run to facilitate suppression of the release
  notes and help upon the *first* run of new version of VisIt_.
* Updated each time VisIt_ is run

Host Profile Files
""""""""""""""""""
* Location(s): ``VUSER_HOME/host/host_<site-name>_<resource-name>.xml``
* Purpose: Stores information on how to connect to and launch jobs on remote
  resources
* All available host profiles are read on VisIt_ startup but this is overridden
  by ``-noconfig``. Users should be aware of this behavior. If the user passes
  ``-noconfig`` to avoid loading preferences, s/he will also be without any
  host profiles.
* Format: ASCII `XML <https://en.wikipedia.org/wiki/XML>`_

VisIt Run Commands (rc) File
""""""""""""""""""""""""""""
* Location: ``VUSER_HOME/visitrc``
* Purpose: Holds Python code to be executed each time VisIt_ is launched.
  Existence of ``VUSER_HOME/visitrc`` means that even when running the GUI,
  VisIt_ will also launch the CLI as a second client. Users must then exit
  from *both* the GUI and CLI in order for the VisIt_ session to completely
  exit.
* Format: Python source code. However, there is ``.py`` file extension in
  the file name.

Custom Plugins
""""""""""""""
* Location(s): There are separate directories in ``VUSER_HOME`` for operator,
  database and plot plugins. These are

  * ``VUSER_HOME/<visit-version>/<visit-arch>/plugins/operators/``
  * ``VUSER_HOME/<visit-version>/<visit-arch>/plugins/databases/``
  * ``VUSER_HOME/<visit-version>/<visit-arch>/plugins/plots/``

  where ``<visit-version>`` and  ``<visit-arch>`` are the VisIt_ version number
  and architecture moniker. 
* Purpose: Directories to hold custom (e.g. user-compiled and installed)
  plugins.
* Format: Binary machine format of the host architecture.

Color Table Files
"""""""""""""""""
* Location(s): ``VUSER_HOME/<color-table-name>.ct``
* Purpose: Store a single color table for easy sharing with other users.
* Written when the user hits the **Export** button in the
  :ref:`color table window <fig-MakingItPretty-ColorTables>` from
  :menuselection:`Controls -> Color table...`.
* All color table files in ``VUSER_HOME/*.ct`` are read and loaded into
  VisIt_ upon VisIt_ startup. However, this behavior is disabled if
  ``-noconfig`` is used.
* Format: ASCII `XML <https://en.wikipedia.org/wiki/XML>`_ specifying the
  :ref:`colors and color control points for the color table <Color_tables>`.

Crash Recovery Files
""""""""""""""""""""
* Location(s): ``VUSER_HOME/crash_recovery.$pid.session`` and
  ``VUSER_HOME/crash_recovery.$pid.session.gui``
* Purpose: Hold the most recently saved last good state of VisIt_ prior
  to a crash. Disabled if the preference
  ``Periodically save a crash recovery file`` is unchecked in the
  **Preferences Window**.
* Written periodically from VisIt_ automatically.
* Read when user starts VisIt_ and answers ``yes`` when queried whether to
  start up from the most recent crash recovery file or when user explicitly
  specifies the file as an argument to the ``-sessionfile``
  command-line :ref:`startup option <StartupOptions>`.
* Format: ASCII `XML <https://en.wikipedia.org/wiki/XML>`_, same as any
  other VisIt_ :ref:`session files <Session files>`.

Files In Other Locations
~~~~~~~~~~~~~~~~~~~~~~~~


VisIt Debug Log Files
"""""""""""""""""""""

VisIt_ debug log (``.vlog``) files are written by many different VisIt_
components; the engine, the viewer, the mdserver, the gui, etc. When running
client/server, some logs are written on the client and some on the server.
The logs on the client are written to whatever the current working directory
was when VisIt_ was started. If started from an icon, this is most likely
the ``VUSER_HOME`` directory. If started from a command-line, it is whatever
the shell's current working directory for that command-line was. On the server,
the logs are written to ``VUSER_HOME``. In a typical client/server scenario,
the user gets gui and viewer logs locally in current working directory and
mdserver and engine logs on the remote system in ``VUSER_HOME``.

Session Files
"""""""""""""
When saving or restoring session files, the user is always prompted with a
file browser to allow the user to indicate where the files are.

databases

xml files for attributes

Save Window vs. Export Database
"""""""""""""""""""""""""""""""

As far as file locations are concerned, the key issue for users to keep in 
mind regarding **Save Window** operations and **Export Database** operations
have to do with client/server operation. In client/server **Save Window**
happens always on the client whereas **Export Database** happens always on
the server.

The **Save Window** and **Export Database** operations can in some circumstances
be highly similar. In general, the **Save Window** operation is to save an
*image* whereas the **Export Database** operation is to output a wholly new
*database* file. The cases where these two operations are blurred is when non-image
formats are used by **Save Window**
