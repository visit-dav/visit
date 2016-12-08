.. _Appendix_B:

Appendix B: Setting up password-less ssh
========================================

The following instructions describe how to set up **ssh** to allow password-less
authentication among a collection of machines.

On the Local Machine
--------------------

If you do not already have a **~/.ssh/id_rsa.pub** file, generate the key::

    cd

    ssh-keygen -t rsa

Accept default values by pressing *<Enter>*.  This will generate two files, 
**~/.ssh/id_rsa** and **~/.ssh/id_rsa.pub**.  The **~/.ssh/id_rsa.pub** file 
contains your public key in one very long line of text.  This information needs 
to be concatenated to the **authorized_keys** file on the remote machine, so 
copy it to a temp file on the remote machine::

     scp ~/.ssh/id_rsa.pub <your-user-name>@<the.remote.machine>:tmp


On the Remote Machine
---------------------

If you do not already have a **~/.ssh** directory, create one with **r-w-x** 
permission for the owner only::

    cd

    mkdir .ssh

    chmod 700 .ssh

If you do not already have a **~/.ssh/authorized_keys** file, create an empty 
one with permission for the owner only::

    cd ~/.ssh

    touch authorized_keys

    chmod 600 authorized_keys 

Concatenate the temporary file you copied into authorized_keys::

    cd ~/.ssh

    cat authorized_keys ~/tmp > authorized_keys

    rm ~/tmp

Completing the Process
----------------------

If you have more remote machines you want to access from the same local machine
using passwordless ssh, repeat the process starting with copying the 
**~/.ssh/id_rsa.pub** file from the local machine to the remote, and 
continuing from there.

You can also repeat the above sections, reversing the local and remote 
machines, in order to allow passwordless ssh to the local machine from the 
remote machine.

