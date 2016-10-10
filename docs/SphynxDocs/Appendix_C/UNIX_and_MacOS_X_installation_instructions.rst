UNIX and MacOS X installation instructions
------------------------------------------

Installing VisIt on UNIX
TM
and MacOS X systems is done using the
**visit-install**
script. In order to run the visit-install script, make sure that you have an available shell window. On MacOS X, open the Terminal application to obtain a command line shell. Also make sure that the visit-install script is executable by entering the following command at the command line prompt:
**chmod 750 visit-install**
. The visit-install script has the following usage:

**visit-install version platform directory**

The
**version**
argument is the version of VisIt being installed. The
**platform**
argument
depends on the platform where VisIt is being installed. The platform argument can be one of the following: aix, darwin, irix6, linux, linux-x86_64, osf1, or sunos5. The
**directory**
argument specifies the directory in which you want to install VisIt. If the specified directory does not exist then VisIt will create it.

To install an Intel/ Linux version of VisIt, the command entered at the shell prompt would look like the following:

**visit-install 1.9.0 linux-intel /usr/local/visit**

This command will install the 1.9.0 version of VisIt in the /usr/local/visit directory. Note that when you enter the above command, the file visit1_9_0_linux-intel.tar.gz must be present in the current working directory.

Once you start running the visit-install script, it will prompt you to choose a network configuration. You can elect to choose no network configuration if you are installing VisIt at an institution for which the visit installer does not provide a network configuration option.
A network configuration is a VisIt preferences file that includes information that VisIt needs to identify and connect to remote computers in distributed mode and is included as a convenience for users of several sites that are heavy VisIt users.

It you want to install VisIt for different platforms in the same directory, you can use the
**-a**
argument when running the visit-install script. For example, if you want to also install a MacOS X version of VisIt in the same visit directory as the Intel/Linux distribution, you could enter the following command into the command shell:

**visit-install -a 1.9.0 darwin-intel /usr/local/visit**

The final step in installing VisIt is adding it to your path. The best way to do this is to add it to your command line shell initialization file. If you are using C-Shell, the file to edit is called .cshrc and it exists in your home directory.
It is important to add VisIt to your path because VisIt's bin directory contains the visit script that is used to run VisIt. The visit script can be run on many machines. The script determines the type machine it is running on and starts the appropriate visit component. This is crucial if you plan to run VisIt in distributed mode and do remote visualization. If VisIt was installed in /usr/local/visit, as in the above examples, the following commands will add VisIt to your search path:

**cd**

**echo "set path = ($path /usr/local/visit/bin)" >>.cshrc**

You are now finished installing VisIt.
