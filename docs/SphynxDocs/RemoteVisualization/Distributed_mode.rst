Distributed mode
----------------

When you run VisIt locally, you usually select files, open a database, and create plots using the open database. Fortunately, the procedure for running VisIt in distributed mode is no different than it is for running in single-computer mode. You begin by opening the
**File Selection Window**
and typing the name of the computer where the files are stored into the
**Host**
text field.

Once you have told VisIt which host to use when accessing files, VisIt launches the VisIt Component Launcher (VCL) on the remote computer. The VCL is a VisIt component that runs on remote computers and is responsible for launching other VisIt components such as the database server and compute engine (
). Once you are connected to the remote computer and VCL is running, you won't have to enter a password again for the remote computer because VCL stays active for the life of your VisIt session and it takes care of launching VisIt components on the remote computer.

If VCL was able to launch on the remote computer and if it was able to successfully launch the database server, the files for the remote computer will be listed in the
**File Selection Window**
. Add the files to be visualized to the
**Selected files list**
as you would with local files and dismiss the
**File Selection Window**
. Now that you have files from the remote computer at your disposal, you can create plots as usual.

Passwords
~~~~~~~~~

Sometimes when you try to access files on a remote computer, VisIt prompts you for a password by opening a
**Password Window**
(
) . If you are prompted for a password, type your password into the window and click the
**Ok**
button. If the password window appears and you decide to abort the launch of the remote component, you can click the
**Password Window's Cancel**
button to stop the remote component from being launched.

VisIt uses
*ssh*
for authentication and you can set up ssh so that passwords are not required. This is
called passwordless ssh and once it is set up for a computer, VisIt will no longer need to prompt for a password. More information about setting up passwordless ssh can be found in
**Appendix B**
.

Environment
~~~~~~~~~~~

It is important to have VisIt in your default search path instead of specifying the absolute path to VisIt when starting it. This isn't as important when you run VisIt locally, but VisIt may not run properly in distributed mode if it isn't in your default search path on remote machines. If you regularly run VisIt using the network configurations provided for LLNL computers then VisIt will have host profiles, which are sets of information that tell VisIt how to launch its components on a remote computer. The provided host profiles have special options that tell the remote computer where it can expect to find the installed version of VisIt so it is not required to be in your path. If you did not opt to install the provided network configurations or if you are at a site that requires other network configurations then you will probably not have host profiles by default and it will be necessary for you to add VisIt to your path on the remote computer. You can add VisIt to your default search path on UNIX
TM
systems by editing the initialization file for your command line shell.

Launch progress window
~~~~~~~~~~~~~~~~~~~~~~

When VisIt launches a compute engine or database server, it opens the
**Launch progress window**
when the component cannot be launched in under four seconds. An exception to this rule is that VisIt will always show the
**Launch progress window**
when launching a parallel compute engine or any compute engine on MacOS X. VisIt's components frequently launch fast enough that it is not necessary to show the
**Launch progress window**
but you will often see it if you launch compute engines using a batch system.

The
**Launch progress window**
indicates VisIt is waiting to hear back from the component being launched on the remote computer and gives you some indication that VisIt is still alive by animating a set of moving dots representing the connection from the local computer to the remote computer. The icon used for the remote computer will vary depending on whether a serial or parallel VisIt component is being launched. The
**Launch progress window**
for a parallel compute engine is shown in
. The window is visible until the remote compute engine
connects back to the viewer or the connection is cancelled. If you get tired of waiting for a remote component to launch, you can cancel it by clicking the
**Cancel**
button. Once you cancel the launch of a remote component, you can return to your VisIt session. Note that if the remote compute is a parallel compute engine launched via a batch system, the engine will still run when it is finally scheduled but it will immediately dies since VisIt has stopped listening for it. On heavily saturated batch systems, it might be prudent for you to manually remove your compute engine job from the batch queue.
