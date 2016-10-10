On the Local Machine
--------------------

If you already have a
**~/.ssh/identity.pub**
file, you can skip these configuration steps. (Go to Section B.2)

**cd**

**ssh-keygen**

Accept default values by entering
*<return>*
. When asked for a passphrase enter a passphrase to gain a greater level of security. List the contents of the
**.ssh**
directory.

**ls -l .ssh**

The file
**identity.pub **
contains your public key in one very long line of text. The file identity contains your private key, which is non-readable data. The subdirectory
**$HOME/.ssh**
subdirectory must remain
**r-w-x**
permissions for the owner only. Check that this is the case.

**ls -ld .ssh**
