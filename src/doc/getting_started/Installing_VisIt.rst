.. _Installing and Starting VisIt:

Installing and Starting VisIt_
------------------------------

Pre-built binaries for VisIt_ are provided on the following platforms:

* |fs*nix| (fully supported)
* |ps*nix| (partially supported)
* macOS
* Microsoft Windows

For an explanation of what *fully* and *partially* supported mean, see our section on :ref:`supported platforms <supported_platforms>`.

New versions are usually released every 2-3 months.
Users can find releases at the `VisIt releases page <https://visit-dav.github.io/visit-website/releases-as-tables/>`_. 
See our section on :ref:`managing GitHub notifications <managing_github_notifications>` to get notified of new releases.

Download a binary release compatible with the machine on which you want to run VisIt_.
If you are installing VisIt_ on Linux, you will also need to download and use the ``visit-install`` script.

Installing VisIt_ on platforms other than those listed here requires :ref:`building VisIt from sources <Building>`.

.. _MacOSX Installation Instructions:

Installing on macOS
~~~~~~~~~~~~~~~~~~~

VisIt_ releases include an app-bundle for macOS packaged in a DMG image.
Download and open the DMG file and copy the VisIt_ app-bundle to your applications directory or any other path.

.. note::
   If you have a previous version of VisIt already installed, you may be prompted by macOS to decide if you want to **Keep both** versions or **Replace** the old version with the new version.
   If you choose **Keep both**, macOS will automatically adjust the name of the new version to something like ``VisIt 2``.
   The space will cause problems and it will have to be removed as in ``VisIt2`` or, ``VisIt-x.y.z`` where ``x.y.z`` is the version number of the installation.
   If you do not have the necessary privileges to change the name, a system administrator's help may be needed.

To run VisIt_ double click on the VisIt_ app-bundle.
Alternatively, VisIt_ can be run on macOS from the ``Terminal`` using a command of the form:

.. code:: bash

  /Applications/VisIt.app/Contents/Resources/bin/visit

.. note::

   Running from the ``Terminal`` may work around issues accessing some folders on local storage or code signing and notarization.

The ``visit-install`` script can also be used to install tarball packaged macOS binaries.
For this case follow the Linux installation instructions. 

.. _Linux Installation Instructions:

Installing on Linux
~~~~~~~~~~~~~~~~~~~

Installing VisIt_ on Linux (and optionally on macOS) is done using the ``visit-install`` script.
Make sure that the ``visit-install`` script is executable by entering the following command at the command line prompt:

.. code:: bash

  chmod +x visit-install

The visit-install script has the following usage:

.. code:: bash

  ./visit-install version platform directory

The **version** argument is the version of VisIt_ being installed.
The **platform** argument depends on the type platform VisIt_ is being installed for.
The platform argument can be one of the following: linux, linux-x86_64, darwin.
The **directory** argument specifies the directory to install VisIt_ into.
If the specified directory does not exist then VisIt_ will create it.

For example, to install an x86_64 version of VisIt_ 3.2.2, use:

.. code:: bash
  
  ./visit-install 3.2.2 linux-x86_64 /usr/local/visit
  

This command will install the 3.2.2 version of VisIt_ into the ``/usr/local/visit`` directory.
Note that when you enter the above command, the file ``visit3_2_2.linux-x86_64.tar.gz`` must be present in the current working directory.

The ``visit-install`` script will prompt you to choose a network configuration.
A network configuration is a set of VisIt_ preferences that provide information to enable VisIt_ to identify and connect to remote computers and run VisIt_ in client/server mode.
VisIt_ includes network configuration files for several computing centers with VisIt_ users.

After running ``visit-install``, you can launch VisIt_ using ``bin/visit``.
For example, if you installed to ``/usr/local/visit``, you can run using:

.. code:: bash
  
  /usr/local/visit/bin/visit

We also recommend adding ``visit`` to your shell's path.
For bash users this can usually be accomplished by modifying the ``PATH`` environment variable in ``~/.bash_profile``, and for c-shell users accomplished by modifying the ``path`` environment variable in ``~/.cshrc``.
 
The exact procedure for this varies with each shell and may be customized at each computing center, so please refer to your shell and computing center documentation. 
 
.. _Windows Installation Instructions:

Installing on Windows
~~~~~~~~~~~~~~~~~~~~~

VisIt_ release binaries for Windows are packaged in an executable installer.
To install on Windows run the installer and follow its prompts.

The VisIt_ installation program adds a VisIt_ program group to the Windows Start menu and it adds a VisIt_ shortcut to the desktop.
You can double-click on the desktop shortcut or use the Start menu's VisIt_ program group to launch VisIt_.
In addition to creating shortcuts, the VisIt_ installation program creates file associations for ``.silo``, ``.visit``, and ``.session/.vses`` files so double-clicking on files with those extensions opens them with VisIt_.

You can also run the installer from a command prompt, and pass it certain parameters to override defaults including running in silent mode.
Available options are as follows::


     -ALLUSERS             Install for all users.
                           Must be in admin-mode.
                           Default : install for current user
 
     -SITE <site-name>     Specifies host profiles to be installed (eg llnl).
                           Default: None
 
     -PB <parallel bank>   Specifies parallel bank (FOR LLNL host-profiles).
                           Default: wbronze
 
     -DB <database reader> Specifies a default database reader for VisIt.
                           (eg Silo, FLASH, etc).
                           Default: None
 
     -DEV                  Install plugin development tools.
                           Default: no plugin dev tools
 
     -LIBSIM               Install libsim tools.
                           Default: no libsim tools
 
     -AssociatePython      Associate python files with VisIt.
                           Default: don't set up association
 
     -AssociateCurves      Associate curve files with VisIt.
                           Default: don't set up association
 
     /S                    Make install silent.
 
     /D <installationdir>  Change install directory to <installationdir>.
                           Default is %PROGRAM FILES% for ALL USERS and
                           %HOMEPATH% for single user.
                           MUST BE THE LAST PARAMETER!

Startup Options
~~~~~~~~~~~~~~~

VisIt_ has many startup options that affect its behavior (see the :ref:`Startup Options <StartupOptions>` for complete documentation).
