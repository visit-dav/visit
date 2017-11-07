.. _Installing and Starting VisIt:

Installing and Starting VisIt
------------------------------------------

VisIt runs on the following platforms:

* Linux (including Ubuntu, RedHat, SUSE, CHAOS)
* Mac OSX
* Microsoft Windows

A new version of VisIt is usually released every 2-3 months, you can 
find VisIt release executables at: 
https://wci.llnl.gov/simulation/computer-codes/visit/executables.

Download a binary release compatible with the machine you want to install
VisIt on. If you are installing VisIt on Linux, also download the 
``visit-install`` script.

.. _MacOSX Installation Instructions:

Installing on Mac OSX
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

VisIt releases include an app-bundle for Mac OSX packaged in a DMG image.
Download and open the DMG file and copy the VisIt app-bundle to your
applications directory or any other path. To run VisIt
double click on the VisIt app-bundle. The ``visit-install`` script can 
also be used to install tarball packaged OSX binaries. For this case
follow the Linux installation instructions. 

.. _Linux Installation Instructions:

Installing on Linux
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Installing VisIt on Linux (and optionally on Mac OSX) is done using the
``visit-install`` script. Make sure that the ``visit-install`` script
is executable by entering the following command at the command line prompt:

.. code:: bash

  chmod +x visit-install

The visit-install script has the following usage:

.. code:: bash

  visit-install version platform directory

The **version** argument is the version of VisIt being installed. 
The **platform** argument depends on the type platform VisIt is being
installed for. The platform argument can be one of the following:
linux, linux-x86_64, darwin. The **directory** argument specifies the directory
to install VisIt into. If the specified directory does not exist
then VisIt will create it.

For example, to install an x86_64 version of VisIt 2.12.0, use:

.. code:: bash
  
  visit-install 2.12.0 linux-x86_64 /usr/local/visit
  

This command will install the 2.12.0 version of VisIt into the 
``/usr/local/visit`` directory. Note that when you enter the above
command, the file ``visit2_12_0_linux-x86_64.tar.gz`` must be present in the
current working directory.

The ``visit-install`` script will prompt you to choose a network configuration.
A network configuration is a set of VisIt preferences that provide
information to enable VisIt to identify and connect to remote
computers and run VisIt in client/server mode.  VisIt includes network 
configuration files for several computing centers with VisIt users.

After running ``visit-install``, you can launch VisIt using ``bin/visit``
. For example, if you installed to ``/usr/local/visit``, you can 
run using:

.. code:: bash
  
  /usr/local/visit/bin/visit

We also recommend adding ``visit`` to your shell's path. 
For bash users this can usually be accomplished by modifying the
``PATH`` environment variable in ``~/.bash_profile``, and for
c-shell users accomplished by modifying the
``path`` environment variable in ``~/.cshrc``. 
 
The exact procedure for this varies with each shell and may
be customized at each computing center, so please refer to your
shell and computing center documentation. 

 
.. _Windows Installation Instructions:

Installing on Windows
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

VisIt release binaries for Windows are packaged in an executable installer.
To install on Windows run the install and follow its prompts.

The VisIt installation program adds a VisIt program group to the Windows 
Start menu and it adds a VisIt shortcut to the desktop.
You can double-click on the desktop shortcut or use the Start menu's VisIt
program group to launch VisIt. In addition to creating shortcuts, the VisIt
installation program creates file associations for ``.silo``, ``.visit``, and
``.session/.vses`` files so double-clicking on files with those extensions
opens them with VisIt.

.. danger:: link to Command Line Options?



