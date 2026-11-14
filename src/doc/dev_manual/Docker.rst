Using Docker
============

Overview
--------

The release container scripts build VisIt's third-party libraries, build and
package VisIt, install the package, and run a smoke test. They can use Docker
or Podman on Linux, macOS, and Windows hosts.

The recommended interface is ``scripts/docker/create_release.py``. The driver
uses the Dockerfiles and helper scripts directly from the VisIt checkout and
creates a temporary build context. Do not copy the Docker scripts into a
directory containing the release inputs.

The driver requires two release inputs:

* A source archive named ``visitMAJOR_MINOR_PATCH.src.tar.gz``.
* The matching ``build_visitMAJOR_MINOR_PATCH`` script.

The VisIt version is inferred from those filenames, and the driver verifies
that they agree. No release version is embedded in a Dockerfile or helper
script.

Install Docker using the instructions at
https://docs.docker.com/get-docker/. Podman may be used by passing
``--engine podman``.

Supported Platforms
-------------------

``create_release.py`` supports the following platform names and version
arguments.

.. list-table::
   :header-rows: 1

   * - Platform
     - Dockerfile
     - Default version
     - Other supported versions
   * - ``ubuntu``
     - ``Dockerfile-ubuntu``
     - ``24.04``
     - ``22.04``, ``26.04``
   * - ``rocky``
     - ``Dockerfile-rocky``
     - ``9.3``
     - ``8.9``
   * - ``debian``
     - ``Dockerfile-debian``
     - ``12``
     - Set with ``--os-version``
   * - ``fedora``
     - ``Dockerfile-fedora``
     - ``44``
     - Set with ``--os-version``
   * - ``manylinux``
     - ``Dockerfile-manylinux``
     - ``2_28``
     - Set with ``--os-version``

The manylinux default selects
``quay.io/pypa/manylinux_2_28_x86_64``. Manylinux versions use an underscore,
as in ``2_28``.

Creating a Release
------------------

The source archive and ``build_visit`` script may be anywhere on the host.
For example:

.. code-block:: console

   $ python3 /path/to/visit/scripts/docker/create_release.py \
       --source /releases/visit3_6_0.src.tar.gz \
       --build-visit /releases/build_visit3_6_0 \
       --platform ubuntu --os-version 24.04

If the inputs are in the current directory, relative paths may be used:

.. code-block:: console

   $ python3 /path/to/visit/scripts/docker/create_release.py \
       --source visit3_6_0.src.tar.gz \
       --build-visit build_visit3_6_0 \
       --platform fedora --os-version 44

Examples for the other platforms include:

.. code-block:: console

   $ python3 create_release.py --source "$src" --build-visit "$bv" \
       --platform rocky --os-version 8.9
   $ python3 create_release.py --source "$src" --build-visit "$bv" \
       --platform debian --os-version 12
   $ python3 create_release.py --source "$src" --build-visit "$bv" \
       --platform manylinux --os-version 2_28

The temporary context is removed automatically, leaving the checkout and
input directory unchanged. Use ``--temp-dir`` to select the parent directory
for that context.

By default, a successful VisIt 3.6.0 Ubuntu 24.04 build creates these host
artifacts in the current directory:

* ``visit3_6_0.linux-x86_64-ubuntu24.tar.gz``
* ``visit0000-ubuntu24.png``

The platform suffix prevents builds for different operating systems from
overwriting one another. Use ``--output-dir`` to select another artifact
directory or ``--label`` to override the suffix. Existing artifacts are not
overwritten. Use ``--build-only`` to create the container image without
extracting host artifacts.

The default image tag is derived from the version and platform, for example
``visitdev:3.6.0-ubuntu24``. It may be overridden with ``--tag``.

Podman on LC
------------

Enable Podman in the calling shell before running the driver on LC. Preserve
the user namespace mappings used by the release environment:

.. code-block:: console

   $ /collab/usr/gapps/lcweg/containers/scripts/enable-podman.sh
   $ python3 /path/to/visit/scripts/docker/create_release.py \
       --source "$src" --build-visit "$bv" \
       --engine podman --platform ubuntu --os-version 24.04 \
       --userns-uid-map 0:0:1 \
       --userns-uid-map 1:1:1999 \
       --userns-uid-map 65534:2047:1

How the Build Is Layered
------------------------

Each Dockerfile defines a ``visit-build-environment`` stage that installs the
operating-system packages and copies the version-independent build helpers.
The normal ``visit-release`` stage then performs these operations:

#. Run ``build_visit`` with the core build options.
#. Run the same command again with ``--optional`` added.
#. Build and package VisIt, install the package, and run a smoke test.
#. Remove downloaded third-party tarballs after all VisIt steps succeed.

The separate core and optional invocations create useful Docker cache layers.
Both use ``build_visit --cleanup``, which removes a third-party library's
source and build directories only after that library builds and installs
successfully. Downloaded tarballs remain available until VisIt itself has
been built, installed, and tested successfully.

XCB and xkbcommon are included in the core options so Qt cannot silently
configure without the X11 platform support required by VisIt.

Recovering a Failed Build
-------------------------

Docker build layers are not sufficient when a long-running command fails:
the failed command does not create a reusable image. Add ``--recoverable`` to
run the core, optional, and VisIt operations in named containers instead:

.. code-block:: console

   $ python3 create_release.py --recoverable \
       --source "$src" --build-visit "$bv" \
       --platform rocky --os-version 8.9

Each successful operation creates a checkpoint image. If an operation fails,
the driver:

* Commits the partial filesystem to an image ending in ``-core-failed``,
  ``-optional-failed``, or ``-visit-failed``.
* Retains the stopped container.
* Copies ``build_visitMAJOR_MINOR_PATCH_log`` into ``--output-dir`` with the
  platform label appended, for example
  ``build_visit3_6_0_log_rocky8``.
* Prints the exact image, container, and resume command.

Recovery images contain the source archive, ``build_visit`` script, platform
metadata, release tag, and version. Resume a failed operation without
repeating the original platform and input arguments:

.. code-block:: console

   $ python3 create_release.py --recoverable \
       --resume-from RECOVERY_IMAGE --resume-step optional

The valid resume steps are ``core``, ``optional``, and ``visit``. The VisIt
build also records extraction, configuration, packaging, installation, and
testing milestones. A resumed VisIt compilation is incremental and reuses
completed milestones.

Recovery images made by older versions of the driver may not contain the
source archive. Supply it when resuming such an image:

.. code-block:: console

   $ python3 create_release.py --recoverable \
       --resume-from OLD_RECOVERY_IMAGE --resume-step visit \
       --source /releases/visit3_6_0.src.tar.gz

If a third-party source archive was missing, copy it into the retained
container, update the recovery image, and resume:

.. code-block:: console

   $ docker cp conduit.tar.gz STOPPED_CONTAINER:/home/visit/
   $ docker commit STOPPED_CONTAINER RECOVERY_IMAGE
   $ python3 create_release.py --recoverable \
       --resume-from RECOVERY_IMAGE --resume-step optional

Interactive Debugging
---------------------

Start a shell from a failed recovery image without ``--rm`` so that debugging
changes are retained:

.. code-block:: console

   $ docker run -it --name visit-debug \
       --entrypoint /bin/bash RECOVERY_IMAGE -l

The partial build is in ``/home/visit``. Reproduce an operation with one of
the following commands:

.. code-block:: console

   $ python3 run_build_visit.py
   $ python3 run_build_visit.py --optional
   $ python3 build_test_visit.py

After exiting, reopen the same container with
``docker start -ai visit-debug``. Commit useful fixes or added files and resume
through the driver:

.. code-block:: console

   $ docker commit visit-debug visitdev:3.6.0-debug
   $ python3 create_release.py --recoverable \
       --resume-from visitdev:3.6.0-debug --resume-step optional

Replace ``docker`` with ``podman`` in these examples when using Podman.

Adding a Platform
-----------------

A platform Dockerfile belongs in ``scripts/docker`` and should follow the
same two-stage interface as the existing files:

* Name the dependency stage ``visit-build-environment``.
* Name the completed stage ``visit-release``.
* Use a build argument for the base operating-system version.
* Copy ``build_visit[0-9]*``, ``visit_version.py``, and
  ``run_build_visit.py`` into the environment stage.
* Run ``run_build_visit.py`` once for core libraries and once with
  ``--optional`` in the release stage.
* Copy ``visit*.src.tar.gz`` and the shared Python build/test helpers without
  embedding a VisIt release number.
* Run ``build_visit_docker_cleanup.py`` only after
  ``build_test_visit.py`` succeeds.

Register the platform's Dockerfile, build argument, and default operating
system version in ``PLATFORMS`` in ``create_release.py``. This registration
also adds the platform to the command-line choices and to recoverable builds.

Useful Container Commands
-------------------------

The following commands list and remove local images and containers:

.. code-block:: console

   $ docker image ls
   $ docker container ls --all
   $ docker image rm IMAGE
   $ docker container rm CONTAINER

Recovery mode deliberately retains failed containers and images. Remove them
after the release succeeds and they are no longer needed.
