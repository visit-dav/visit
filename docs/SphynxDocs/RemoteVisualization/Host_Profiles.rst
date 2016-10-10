Host Profiles
-------------

When VisIt launches a component on a remote computer, it looks for something called a
*host profile*
. A host profile contains information that VisIt uses to launch components on a remote computer. Host profiles allow you to specify information like: remote username, number of processors, parallel launch method, etc. You can have multiple host profiles for any given computer. Host profiles are most often used to specify options for running VisIt's parallel compute engine on remote computers and often the same host will have a serial host profile and several other host profiles with different parallel options.

Host Profile Window
~~~~~~~~~~~~~~~~~~~

VisIt provides a
**Host Profile Window**
, shown in
, that you can use to manage your host profiles. You can open the
**Host Profile Window**
by choosing
**Host profiles**
from the
**Main Window's File**
menu. The
**Host Profiles Window**
is divided vertically into two main areas. The top area contains a group of tabs where each tab contains the host profiles for a given computer. The top area also contains controls to create and delete host profiles. The bottom area of the window displays all attributes for the selected host profile.

Creating a new host profile
"""""""""""""""""""""""""""

You click the
**New profile**
button to create a new host profile based on the selected host profile. If no host profile exists or if no host profile is selected, the new host profile contains reasonable
default values. In any case, the host profile has a default name that you can change to make the new host profile more memorable. If the host profile is for a computer for which there are no other host profiles, a tab is created with the name of the new computer and the new host profile is added to the new tab.

Deleting a host profile
"""""""""""""""""""""""

If a host profile is no longer useful, you can click on it in the tabbed list to select it and then click the
**Delete**
button to delete it.

Activating a host profile
"""""""""""""""""""""""""

Only one host profile can be active for any given computer. When VisIt launches a remote component, it looks for the active host profile for the computer where the component is to be launched. To activate a different host profile, click the
**Active profile for host**
toggle button in the
**Selected profile**
settings. The VCL and the database server use the active host profile but VisIt will prompt you for a host profile to use before launching a compute engine if you have more than one host profile or your only host profile has parallel options set for the compute engine.

Setting the host name
"""""""""""""""""""""

The host name is the name of the computer for which the host profile is created. You can change the host name by typing a new host name into the
**Remote host name**
combo box. You can also select an existing host name, if there are any, from the
**Remote host name**
combo box. If you provide a host name, you can provide the fully qualified computer name (hostname.domain.net) or just the host name. Some systems require the fully qualified name.

Some clustered systems have one overall host name but also have names for the individual compute nodes that comprise the system. The compute nodes are often named by appending the node number to the host name. For example, if the clustered system is called:
*cluster*
, you might be logged into node
*cluster023*
. When you launch a remote component, VisIt will not find any host profiles if the host name in the host profiles is:
*cluster*
.

To ensure that VisIt correctly matches a computer's node name to one of VisIt's host profiles, you should include host name aliases in the host profile for a clustered system. Host name aliases typically consist of the host name with different wildcard characters appended to it. Three wildcards are supported. The
*'?'*
wildcard character lets any one character replace it while the '
**'*
wildcard character lets any character or group of characters replace it and the
*`#'*
wildcard character lets any numeric digit replace it. Appropriate host aliases for the previous example would be:
*cluster#*
,
*cluster##*
,
*cluster###*
, etc. If you need to enter host name aliases for the host profile, type them into the
**Host name aliases**
text field.

Setting the remote user name
""""""""""""""""""""""""""""

The remote user name is the name of the account that you want to use when you access the remote computer. The remote user name does not have to match your local user name and it is often the case that your desktop user name will not match your remote user name. To change the remote user name, type a new user name into the
**Username**
text field in the
**Selected profile**
settings.

Setting the timeout
"""""""""""""""""""

The compute engine and database server have a
timeout
mechanism that causes them to exit if no requests have been made of them for a certain period of time so they do not run indefinitely if their connection to VisIt's viewer is severed. You can set this period of time, or timeout, by typing in a new number of minutes into the
**Timeout**
text field. You can also increase or decrease the timeout by clicking on the up and down arrows next to the
**Timeout**
text field.

Providing additional command line options
"""""""""""""""""""""""""""""""""""""""""

The
**Host Profile Window**
allows you to provide additional command line options to the compute engine and database server through the
**Additional options**
text field. When you provide additional command line options, you should type them, separated by spaces, into the
**Additional options**
text field. Command line options influence how the compute engine and database server are executed. For more information on VisIt's command line options, see
**Appendix A**
. Most of the host profiles that are installed with VisIt specify the expected installation directory for VisIt so VisIt does not have to be in your path on remote computes.

Setting parallel options
~~~~~~~~~~~~~~~~~~~~~~~~

The chief purpose of host profiles is to make launching compute engines easier. This is even more the case when host profiles are used to launch parallel compute engines on large computers that often have complex scheduling programs that determine when parallel jobs can be executed. It is easy to forget how to use the scheduling programs on a large computer because each scheduling program requires different arguments. In order to make launching compute engines easy, VisIt hides the details of the scheduling program used to launch parallel compute engines. VisIt instead allows you to set some common parallel options and then figures out how to launch the parallel compute engine on the specified computer using the parallel options specified in the host profile. Furthermore, once you create a host profile that works for a computer, you rarely need to modify it.


Setting the parallel launch method
""""""""""""""""""""""""""""""""""

The parallel launch method option allows you to specify which launch program should be used to execute the parallel compute engine. This setting depends on the computer where you plan to run the compute engine and how the computer is configured. Some computers have multiple launch programs depending on which part of the parallel machine you want to use. The table below lists some common Operating System/Launch program pairs.

+------------------+---------------------------------------+
| Operating System | Launch program                        |
|                  |                                       |
+------------------+---------------------------------------+
| IRIX             | mpirun                                |
|                  |                                       |
+------------------+---------------------------------------+
| AIX              | poe, interactive partition            |
|                  |                                       |
+------------------+---------------------------------------+
| AIX              | psub, batch partition                 |
|                  |                                       |
+------------------+---------------------------------------+
| Linux CHAOS      | psub/srun, both interactive and batch |
|                  |                                       |
+------------------+---------------------------------------+
| Tru64            | dmpirun                               |
|                  |                                       |
+------------------+---------------------------------------+

In addition to choosing a launch program, you can also elect to give it additional command line options to influence how it launches your compute engine. To give additional command line options to the launch program, click the
**Additional launcher arguments**
check box and type command line options into the text field to the right of that check box.

Setting the partition/pool
""""""""""""""""""""""""""

Some parallel computers are divided into partitions so that batch processes might be executed on one part of the computer while interactive processes are executed on another part of the computer. You can use host profiles tell VisIt which partition to use when launching the compute engine on systems that have multiple partitions. To set the partition, check the
**Partition/Pool**
check box and type a partition name into the
**Partition/Pool**
text field.

Setting the number of processors
""""""""""""""""""""""""""""""""

You can set the number of processors by typing a new number of processors into the
**Default number of processors**
text field. When the number of processors is greater than 1, VisIt will attempt to run the parallel version of the compute engine. You can also click on the up and down arrows next to the text field to increase or decrease the number of processors. If VisIt finds a parallel host profile, you will have the option of changing the number of processors before the compute engine is actually launched.

Setting the number of nodes
"""""""""""""""""""""""""""

The number of nodes refers to the number of compute nodes that you want to reserve for your parallel job. Each compute node typically contains more than one processor (often 2, 4, 16) and the number of nodes required is usually the ceiling of the number of processors divided by the number of processors per node. It is only necessary to set the number of nodes if you want to use fewer processors than the number of processors that exist on a compute node. This option is not available on some computers as it is meant primarily for compute clusters. To set the number of nodes, check the
**Default number of nodes**
check box and type a number of processors per node into the
**Default number of nodes**
text field.

Load balancing
""""""""""""""

Load balancing refers to how well tasks are distributed among computer processors. The goal is to make each computer processor have roughly the same amount of work so they all finish at the same time. VisIt's compute engine supports two forms of load balancing. The first form is static load balancing where the entire problem is distributed among processors and that distribution of work never changes. The second form of load balancing is dynamic load balancing. In dynamic load balancing, the work is redistributed as needed each time work is done. Idle processors independently ask for work until the entire task is complete. VisIt allows you to specify the form of load balancing that you want to use. You can choose to use static or dynamic load balancing by clicking the
**Static**
or
**Dynamic**
radio buttons. There is also a default setting that uses the most appropriate form of load balancing.

Setting the default bank
""""""""""""""""""""""""

Some computers, if they are large enough, have scheduling systems that break up the number of processors into banks, which are usually reserved for particular projects. Users who contribute to a project take processors from their default bank of processors. By default, VisIt uses environment variables to get your default bank when submitting a parallel job to the batch system. If you want to override those settings, you can click the
**Default Bank**
check box to turn it on and then type your desired bank into the text field next to the check box.

Setting the parallel time limit
"""""""""""""""""""""""""""""""

The parallel time limit is the amount of time given to the scheduling program to tell it the maximum amount of time, usually in minutes, that your program will be allowed to run. The parallel time limit is one of the factors that determines when your compute engine will be run and smaller time limits often have a greater likelihood of running before jobs with large time limits. To specify a parallel time limit, click the
**Default Time Limit**
check box and enter a number of minutes or hours into the
**Default Time Limit**
text field. If you want to specify minutes, be sure to append "m" to the number or append an "h" for hours. If you want to specify a timeout of 30 minutes, you would type:
*30m*
.

Specifying a machine file
"""""""""""""""""""""""""

When using VisIt with MPICH on some clustered computers, it is necessary to specify a machine file, which is a file containing a list of the compute nodes where the VisIt compute engine should be executed. If you want to specify a machine file when you execute VisIt in parallel on a cluster that requires a machine file, click on the
**Default Machine File**
check box and type the name of the machine file that you want to associate with your host profile into the
**Default Machine File**
text field.

Advanced host profile options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Host profiles contain the information that VisIt needs in order to successfully launch VisIt components on remote computers and to make sure that they are launched with enough computing resources. Most of those properties can be set using the
**Selected profile**
and
**Parallel options**
tabs of the
**Host Profile Window**
but there is also an
**Advanced options**
tab (see
) in the Host Profile Window that lets you specify advanced networking options to ensure that the VisIt components running on the remote computer use resources correctly and can connect back to the viewer running on your local workstation.

Sharing a compute job
"""""""""""""""""""""

Some computers, notably Los Alamos National Laboratory's Q machine, place restrictions on the number of interactive sessions that a single user can have on the computer. To allow VisIt to run on computer systems that enforce these kinds of restrictions, VisIt can optionally force the database server and parallel compute engine to share the same job in the batch system. If you want to make the database server and parallel compute engine share the same batch job, you can click the
**Share batch job with Metadata Server**
check box on the
**Host profiles Window's Advanced options tab**
.

Setting up the parallel environment
"""""""""""""""""""""""""""""""""""

VisIt is usually executed by a script called: visit, which sets up the environment variables required for VisIt to execute. When the visit script is told to launch a parallel compute engine, it sets up the environment variables as it usually does and then invokes an appropriate parallel launch program that takes care of either spawning the VisIt parallel compute engine processes or scheduling them to run in a batch system. When VisIt is used with MPICH on some clusters, the parallel launch program does not replicate the environment variables that the visit script set up, preventing the VisIt parallel compute engine from running. On clusters where the parallel launch program does not replicate the VisIt environment variables, VisIt provides an option to start each process of the VisIt compute engine under the visit script. This ensures that the environment variables that VisIt requires in order to run are indeed set up before the parallel compute engine processes are started. To enable this feature, click on the
**Use VisIt script to set up parallel environment **
check box on the
**Host profiles Window's Advanced options tab**
.

Determining the host name
"""""""""""""""""""""""""

There are many different network naming schemes and each major operating system type seems to have its own variant. While being largely compatible, the network naming schemes sometimes present problems when you attempt to use a computer that has one idea of what its name is with another computer that may use a somewhat different network naming scheme. Since VisIt users are encouraged to use distributed mode because it provides fast local graphics hardware without sacrificing computing power, VisIt must provide a way to reconcile the network naming schemes when 2 different computer types are used.

Workstations often have a host name that was arbitrarily set when the computer was installed and that host name has nothing to do with the computer's network name, which ultimately resolves to an IP address. This condition is common on computers running MS Windows though other operating systems can also exhibit this behavior. When VisIt launches a component on a remote computer, it passes information that includes the host name of the local computer so the remote component will know how to connect back to the local computer. If the local computer did not supply a valid network name then the remote component will not be able to connect back to the local computer and VisIt will wait for the connection until you click the
**Cancel**
button in the
**Launch progress window**
.

By default, VisIt relies on the name obtained from the local computer but if you want to specify a name instead of using the name reported by the local computer then you can use the controls on the
**Advanced options**
tab. To use a host name other than what the local computer returns, you can click the
**Parse from SSH_CLIENT environment variable**
or
**Specify manually**
radio buttons. If you choose the
**Parse from SSH_CLIENT environment variable**
option then VisIt will not pass a host name for the local computer but will instead tell the remote computer to inspect the
*SSH_CLIENT*
environment variable to determine the IP address of the local computer that initiated the connection. This option usually works if you have a local computer that does not accurately report its host name. If you don't trust the output of any implicit scheme for getting the local computer's name, you can provide the name of the local computer by typing its name or IP address into the text field next to the
**Specify manually**
radio button.

VisIt's ports
"""""""""""""

VisIt uses secure shell (ssh) to launch its components on remote computers. Secure shell often uses port 22 but if you are attempting to communicate with a computer that does not use port 22 for ssh then you can specify a port for ssh by clicking the
**Specify port**
check box and then typing a new port number into the adjacent text field.

One common case of when you might want to use a different port is if you are running VisIt on your home computer and connecting to an LLNL computer via Internet Port Allow (IPA), which allows your computer through the Lab firewall so it is possible to connect to some computers from offsite. In this type of connection, the port for ssh is often 822 so in order to successfully
connect VisIt from your home desktop to an LLNL computer, you would specify port 822 in the
**Advanced options**
tab.

In addition to relying on remote computers' ssh port, VisIt listens on its own ports (5600-5605) while launching components. If your desktop computer is running a firewall that blocks ports 5600-5605 then any remote components that you launch will be unable to connect back to the viewer running on your local computer. If you are not able to successfully launch VisIt components on remote computers, be sure that you make sure your firewall does not block VisIt's ports. Windows XP's and MacOS X's default software firewall configurations block VisIt's ports so if you run those software firewall programs, you will have to unblock VisIt's ports if you want to run VisIt in distributed mode.

Engine Option Window
~~~~~~~~~~~~~~~~~~~~

You can use
**Engine Option Window**
, shown in
, to pick a host profile to use when there are multiple host profiles for a computer or if there are any parallel host profiles. When there is a single serial host profile or no host profiles, the window is not activated when VisIt launches a compute engine. The window's primary purpose is to select a host profile and set some parallel options such as the number of processors. This window is provided as a convenience so host profiles do not have to be modified each time you want to launch a parallel engine is run with a different number of processors.

The
**Engine Option Window**
has a list of host profiles from which to choose. The active profile for the host is selected by default though the another can be profile used instead. Once a host profile is selected, the parallel options such as the number of processors/nodes, processor count, can be changed to fine-tune how the compute engine is launched. After making any changes, click the window's
**OK**
button to launch the compute engine. Clicking the
**Cancel**
button prevents the compute engine from being launched.

Setting the number of processors
""""""""""""""""""""""""""""""""

The number of processors determines how many processors are used by VisIt's compute engine. Generally, a higher number of processors yields higher performance but it depends on the host
platform and the database being visualized. The
**Num procs**
text field initially contains the number of processors used in the active host profile but you can change it by typing a new number of processors. The number of processors can also be incremented or decremented by clicking the up/down buttons next to the text field.

Setting batch queue options
"""""""""""""""""""""""""""

Some compute environments such as large IBM computers schedule parallel jobs in batch queues. The
**Engine option window**

provides a few controls that are useful for batch queued systems. The first option is the number of nodes which determines the number of smaller portions of the computer that are allocated to a particular task. Typically the number of processors is evenly divisible by the number of nodes but the window allows you to specify the number of nodes such that not all processors within a node need be active. You can set the number of nodes, by typing a new number into the
**Num nodes**
text field or you can increment or decrement the number by clicking on the arrow buttons to the right of the text field. The second option is the bank which is a large collection of nodes from which nodes can be allocated. To change the bank, you can type a new bank name into the
**Bank**
text field. The final option that the window allows to be changed is the time limit. The time limit is an important piece of information to set because it can help to determine when the compute engine is scheduled to run. A smaller time limit can increase the likelihood that a task will be scheduled to run sooner than a longer running task. You can change the time limit by typing a new number of minutes into the
**Time limit**
text field.

Setting the machine file
""""""""""""""""""""""""

Some compute environments use machine files, text files that contain the names of the nodes to use for executing a parallel job, when running a parallel job. If you are running VisIt in such an environment, the
**Engine option window**
provides a text field called
**Machine file**
. The
**Machine file**
text field allows you to enter the name of a new machine file if you want to override which machine file is used for the selected host profile. The
**Machine file**
text field is only enabled when the
**Default Machine File**
check box is enabled in the
**Host profile Window's**
parallel options.
