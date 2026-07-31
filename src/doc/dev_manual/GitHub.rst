Developing at GitHub
====================

Overview
--------

The VisIt_ project has a number of repositories located at the GitHub visit-dav organization.

https://github.com/visit-dav/

The primary repository for doing VisIt_ development is the visit repository.

https://github.com/visit-dav/visit/

The following top level directories exist in the visit repository.

* `data <https://github.com/visit-dav/visit/tree/develop/data/>`_ - Data files used by the test suite.
* `docs <https://github.com/visit-dav/visit/tree/develop/docs/>`_ - Legacy documentation including design documents and presentations.
* `scripts <https://github.com/visit-dav/visit/tree/develop/scripts/>`_ - Various scripts used for doing VisIt_ developement including scripts for managing docker containers and doing continuous integration.
* `src <https://github.com/visit-dav/visit/tree/develop/src/>`_ - The VisIt_ source code. It includes the Read the Docs documentation and the regression test suite.
* `test/baseline <https://github.com/visit-dav/visit/tree/develop/test/baseline/>`_ - The baseline results for the regression test suite.


Setting Up Git LFS
------------------

Git LFS (Large File Storage) is a mechanism to help revision control large files efficiently with git. Instead of storing large files in the repo, LFS provides an extension that stores small text files with metadata in the repo and the actual files on another server. These metadata files are called "pointer" files. We use LFS for binary data including our test data tar files, source code for third party libraries, and regression test baseline images.

Git LFS is not part of the standard git client. See https://git-lfs.github.com/ for how to obtain Git LFS.

When installing, use the following option::

    git lfs install --force --skip-smudge

The "skip smudge" command sets up LFS in a way that skips automatically pulling our large files on clone. We do this to conserve bandwidth.

To obtain these files you will need to do some extra incantations followed by an explicit::

    git lfs pull
    
For more details about using Git LFS, read :ref:`our additional notes <git_lfs_note>`.

Accessing GitHub
----------------

The following link points to a page for creating a personal access token to use for the password when accessing GitHub through the command line. Use the following scopes for the token:

repo:status

repo_deployment

public_repo

https://help.github.com/articles/creating-a-personal-access-token-for-the-command-line/

The following link describes how to add your ssh key to your GitHub account.

https://help.github.com/articles/adding-a-new-ssh-key-to-your-github-account/

Cloning the Repository and Setting Up Hooks
-------------------------------------------

You can access GitHub either through https or ssh. If you use https you will be prompted for your password whenever you push to GitHub. There are ways you can have your password cached for a period of time to reduce the frequency of entering your password. However, if you have two-factor authentication set up you will need to create a personal access token to use in place of the password. If you use the ssh protocol you can set things up so that you never have to enter a password by adding your ssh key to your GitHub account.

To clone the repository::

    git clone --recursive https://github.com/visit-dav/visit.git

or::

    git clone  --recursive ssh://git@github.com/visit-dav/visit.git

If for some reason the ``--recursive`` flag was overlooked when the repository was originally cloned, this can be easily remedied by::

    cd visit
    git submodule init
    git submodule update

To setup our hooks::

    cd visit
    ./scripts/git-hooks/install-hooks.sh 


Sparse Checkouts
----------------

If your workflow includes multiple clones of the repository in order to work on different VisIt_ tasks without having to constantly swap branches in a single clone, sparse checkouts might be a good fit.
Sparse checkouts allow you to clone the repo, but only download specific directories of interest.
For instance, if you don't have a need to regularly run the regression tests, you might only want the ``src`` and ``scripts`` directories.

Here's how to clone and checkout only those directories::

    git clone --sparse --recursive https://github.com/visit-dav/visit.git
    git sparse-checkout set 'src/*' 'scripts/*'
    git checkout

The use of the asterisk in the set command above  isn't technically necessary, but does guard against other subdirectories with the same name from being part of the checkout.
For instance, to get only the ``data`` directory where the test data lives, if ``git sparse-checkout set 'data'`` is used, then both the ``data`` and the ``src`` directories will be downloaded because there is a ``data`` subdirectory in ``src/tools/data``.

You can use the same commands if you prefer ``ssh`` over ``https`` for cloning.
Branching and development in sparse checkouts works the same as with the full repository.

If later you want to run regressions, you can create a new full clone or a separte sparse clone of just the ``data`` or ``test`` directory, or both.
If the test data directory is not located peer to ``src``, you can tell VisIt_'s CMake build system where to find it by setting ``VISIT_DATA_DIR``  to the location of ``data`` before you configure with CMake.
The location of the test baselines can be passed to the regression test script via ``-b /path/to/test/baseline`` on the command line when running tests.


Creating a Branch
-----------------

Development for VisIt_ is done off of two main branches, the ``develop`` branch and the current release candidate branch, which was ``3.2RC`` when this content was written. The ``develop`` branch is used for development that will go into the next major or minor release. Major releases are releases where the first digit of the release number is incremented, Minor releases are releases where the second digit of the release number is incremented. The release candidate branch is used for development that will go into the next patch release. Patch releases are releases where the third digit of the release number is incremented.

There is no convention on the names of a branch. One commonly used convention is ``task\Username\YYYY_MM_DD_Description`` where ``Username`` is your GitHub user name, ``YYYY`` is the current year, ``MM`` is the current month, ``DD`` is the current day, and ``Description`` is a short description of the task to be performed. Since branches only exist while you are doing the development, the name isn't critical, but it should be sufficiently descriptive so that someone can have some idea what the development on the branch is about.

To create a branch off of the ``develop`` branch::

    git checkout develop
    git pull
    git checkout -b task/user/2021_05_07_bug_fix

To create a branch off of the current release candidate::

    git checkout 3.2RC
    git pull
    git checkout -b task/user/2021_05_07_bug_fix


When you switch branches, you may also need to update submodules so they match your branch::

    git submodule update 


Doing Development
-----------------

Doing development using the Git version control system can be complex and take considerable time and effort to master. The primer below is just meant to get you started in modifying files and then pushing the changes to GitHub so that they can be integrated into VisIt_.

To add a new file or modify an existing file, edit the file with your favorite text editor and then use the ``add`` command so that git knows you want the file to be part of your next commit. To add a file::

    git add src/myfile

To delete an existing file use the ``rm`` command::

    git rm src/myfile

Once you have modified one or more files you can commit the change to git. You will typically do a commit after having modified one or more files that completes a logical unit of change. To commit the added files with a comment::

    git commit -m "Description of my change."

It is recommended that you make commits frequently so that you can better track individual changes. The commit descriptions are typically brief. The record of the individual commits will not go into the final record of the commit, since we do "Squash and Merge" commits that merge all the commits into a single commit at GitHub. The individual commits will be helpful to you as a developer if you need to go back and understand when making many changes over a period of time. It may also potentially make it easier for reviewers to understand your commits.

Once you have finished all your changes you can push the change to GitHub. To push your changes to GitHub::

    git push --set-upstream origin task/user/2021_05_07_bug_fix

Once you have pushed your changes to GitHub, you can submit a :ref:`pull request <Creating a Pull Request>`.

Simultaneous Development on Multiple Branches
---------------------------------------------

Doing VisIt_ development on multiple branches *simultaneously* can be challenging.
Frequently switching between branches in a single clone can involve significant productivity hits especially if the branches diverge substantially from one another.
Maintaining multiple clones is possible but requires a lot of disk space, especially given all of the LFS content in VisIt_.

Another approach is to use git `worktrees <https://git-scm.com/docs/git-worktree?utm_source=chatgpt.com>`__.
Worktrees are like multiple clones except they all share a common, single ``.git`` repository database.

To maintain two independent branches of development for ``develop`` and ``3.5RC`` in two directory peers named ``visit`` and ``3.5RC``, do the following::

    git clone --recursive https://github.com/visit-dav/visit.git
    cd visit
    git worktree add ../3.5RC 3.5RC

The clone is given the *default* directory of ``develop``.
The worktree command will create a worktree directory that is peer to ``visit`` and will check out that worktree to ``3.5RC``.
Work can proceed in either ``visit``  or ``3.5RC`` totally independently.

If instead, you want to create a worktree for a new feature, then while in the ``visit`` directory::

    git worktree add ../feature-xyz -b feature-xyz origin/feature-xyz

This will create a worktree directory, ``feature-xyz``, which is peer to ``visit``.

Alternatively, one can just maintain worktrees representing a *second* and a *third* copy of VisIt_ and do whatever development is desired in either copy::

    cd visit
    git worktree add --detach ../visit2
    cd ../visit2
    git checkout exisiting-branch
    cd ../visit
    git worktree add --detach ../visit3
    cd ../visit3
    git checkout -b new-branch

The ``--detach`` argument will create a worktree directory in a detached head state.
A normal ``git checkout`` command can be used within that worktree directory to set it to whatever branch is needed.

.. note::
   Two worktrees cannot be checked out to the same branch.

To remove a worktree::

    cd visit
    git worktree remove ../visit2

Read more about the advantages of worktrees on `bssw.io <https://bssw.io/items/working-within-multiple-git-branches-simultaneously>`__.

CMake Build System 
-------------------

VisIt's build system uses `BLT <https://github.com/llnl/blt/>`_ CMake helpers.
BLT is included in VisIt's git repo as a git submodule.
To obtain the submodule, use ``git clone --recursive`` when cloning, or manually setup the submodule after cloning using::

    git submodule init
    git submodule update

When you switch branches, you may also need to update submodules so they match your branch::

    git submodule update 

Branch development with git submodules can lead to unintended submodule commits.
To avoid this, we have an CI check that ensures the active submodule commits match
a version explicitly listed in a ``hashes.txt`` file at the root of the git repo.

GitHub Administration
---------------------

The VisIt_ project uses some additional, paid, services on GitHub.
For example, we use additional *data packs*, for higher storage and bandwidth thresholds than the free tier.
These are handled under the VisIt_ organization's **Billing and Plans** section of the organization **Settings**.
These are currently billed annually and renew in mid-January (January 19th).
In the past, someone in LLNL's SD IT department would feed GitHub a credit card number to fund the services for another year.
More recently, LLNL requires a more formal *paper trail* for anything billed in this manner.
So, to ensure someone in LLNL'S SD IT department actually takes care of it, a **ServiceNow** ticket needs to get generated and it needs to be one that specifically goes to the SD IT *procurement* team.
As of this writing, **SD IT Procurement** can be found in **Service Now** by going to `Home->Business Applications Catalog-> Strategic Deterrence (SD)->SD Procurement <https://llnl.servicenowservices.com/ess?id=sc_cat_item&sys_id=34ad842fdb8e3f801a9efd0e0f961942&u_service=ceb29f25dbe622001a9ebd0e0f9619f1&request_type=0224a769dbe622001a9ebd0e0f96191a&sysparm_category=3e260246db98b70027d2f81d0f9619d1>`__.
Be sure to set **Urgency** to ``1-High``, provide VisIt_'s project/task number and then indicate in the description it is... *"For the yearly renewal of '5 Git LFS data packs', additional storage and bandwidth needed by our project on GitHub (github.com)*.
Tiffany Taylor (taylor253) has been the SD IT department member to take care of it.
