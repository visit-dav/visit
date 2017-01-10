.. _Linux and MacOSX installation instructions:

Linux and MacOSX installation instructions
------------------------------------------

.. danger::
   on osx we also can use app bundles

Installing VisIt on Linux and MacOSX systems is done using the
``visit-install`` script. Make sure that the ``visit-install`` script
is executable by entering the following command at the command line prompt:

.. code:: bash

  chmod +x visit-install

The visit-install script has the following usage:

.. code:: bash

  visit-install version platform directory

The **version** argument is the version of VisIt being installed. 
The **platform** argument depends on the type platform VisIt is being
installed for. The platform argument can be one of the following: darwin,
linux, linux-x86_64. The **directory** argument specifies the directory
in which you want to install VisIt. If the specified directory does not exist
then VisIt will create it.

To install an x86_64 version of VisIt, use:

.. code:: bash
  
  visit-install 2.12.0 linux-x86_64 /usr/local/visit
  

This command will install the 2.12.0 version of VisIt in the 
``/usr/local/visit`` directory. Note that when you enter the above
command, the file ``visit2_12_0_linux-x86_64.tar.gz`` must be present in the
current working directory.

The visit-install script will prompt you to choose a network configuration.
You can elect to choose no network configuration if you are installing VisIt
at an institution for which the visit installer does not provide a network
configuration option. A network configuration is a VisIt preferences file that
includes information that VisIt needs to identify and connect to remote
computers in distributed mode and is included as a convenience for users of
several sites that are heavy VisIt users.


.. danger::

  add bash instructions?

The final step in installing VisIt is adding it to your path. The best way to
do this is to add it to your command line shell initialization file. If you
are using C-Shell, the file to edit is called .cshrc and it exists in your
home directory. It is important to add VisIt to your path because VisIt's bin
directory contains the visit script that is used to run VisIt. The visit script
can be run on many machines. The script determines the type machine it is
running on and starts the appropriate visit component. This is crucial if you
plan to run VisIt in distributed mode and do remote visualization. If VisIt
was installed in ``/usr/local/visit``, as in the above examples, the following
commands will add VisIt to your search path:

.. code:: bash
  
  cd
  echo "set path = ($path /usr/local/visit/bin)" >>.cshrc**

