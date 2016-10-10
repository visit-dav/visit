On the Remote Machine
---------------------

If you already have a
**$HOME/.ssh/authorized_keys**
file, append the contents of the local machine's
**$HOME/.ssh/identity.pub**
file to this file. If this isn't the case then create a
**$HOME/.ssh**
directory, if one does not exist, with
**r-w-x**
permission for the owner only.

**cd**

**mkdir .ssh**

**chmod 700 .ssh**

Check that the directory does not allow world or group access. SSH will not work if world or group access is allowed.

**ls -ld ~**

Copy the contents of the local machine's
**$HOME/.ssh/identity.pub**
file to the remote machine into the file
**$HOME/.ssh/authorized_keys**
. This provides the information for the remote machine to validate you. Remember that
**identity.pub**
contains a single long line. Either
**ftp**
the
**identity.pub**
file to the remote machine or edit it, being careful to avoid introducing carriage returns.
