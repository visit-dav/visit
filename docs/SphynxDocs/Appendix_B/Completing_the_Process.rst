Completing the Process
----------------------

You can then repeat the section "On the Remote Machine" for each remote machine for which you want to set up password-less ssh. You can repeat the above sections, reversing the local and remote machines, in order to allow password-less ssh to the local machine from the remote machine (i.e., copy the remote machine's
**$HOME/.ssh/identity.pub**
into the local machine's
**$HOME/.ssh/authorized_keys**
file).
