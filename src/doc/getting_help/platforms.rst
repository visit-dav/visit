.. _supported_platforms:

Supported Platforms
~~~~~~~~~~~~~~~~~~~

.. note::

   Supported platforms periodically change due to any of a number of factors outside the control of the VisIt_ project including demand, developer access, technology obsolescence, and majority stakeholder requirements.

The platforms upon which the majority of contributors regularly develop and run VisIt_ and for which the resources exist to provide *full support* are determined primarily by our `host organization <https://wci.llnl.gov>`_.
For these platforms, we are able to reproduce bugs, test bug-fixes, develop work-arounds and in general provide the highest quality support services.
These platforms are...

* |fs*nix|
* Windows 7, 8 and 10
* macOS 10.15

We do make an effort to provide pre-compiled binaries as well as perform minimal testing for |ps*nix|.
However, because we do not yet routinely develop or test on these platforms, we are not able to provide the same level of support as we do for |fs*nix|.

Finally, for other platforms, :ref:`building <Building>` VisIt_ from sources is an option.
We do try to be responsive to issues building VisIt_ from sources.

Supported Versions
~~~~~~~~~~~~~~~~~~

A VisIt_ version number is composed of three digits, ``A.B.C``, where ``A`` is the major version, ``B`` is the minor version and ``C`` is the patch version.
Patch releases are made approximately 2-4 times a year and are designed to be compatible.
Minor releases are made approximately 1-2 times a year and are not compatible with any prior versions.
Major releases are made infrequently.
Major releases are not compatible with any prior versions and might also not be compatible with older operating systems, older compilers, and/or older primary dependencies (e.g. VTK or GL).

.. tip:: Use VisIt_'s `GitHub milestones <https://github.com/visit-dav/visit/milestones>`__ page or reach out to the VisIt_ team on our `GitHub discussions <https://github.com/visit-dav/visit/discussions>`__ page if you need help planning for upcoming releases of VisIt_.

Only in very rare circumstances does the VisIt_ team have the resources to update a previously released version of VisIt_.
This does occasionally happen but it is very rare.
Instead, we ask that users please try to keep up to date with the most current minor release of VisIt.

.. tip:: Wherever possible, its best if users can keep a version or two *behind* the current minor release around as a fall back in case the current release introduces any show-stopper behavior.

What this means is that the only *supported* version of VisIt_ is the *current* minor version.
When users report issues which are reproducible *only* in versions of VisIt_ older than the *current* minor version, the team may be able to suggest work-arounds but will otherwise ask users to please upgrade to the current version.
On the other hand, when issues reported in older versions are reproducible in the current version as well, the VisIt_ team will try to provide additional support.
