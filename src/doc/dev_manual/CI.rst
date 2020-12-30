Docker Containers For CI Testing
================================

We use Azure Pipelines for CI testing VisIt_'s Pull Requests, located at
`VisIt Azure DevOps Space <https://dev.azure.com/visit-dav/VisIt/>`_.


To speed up our CI testing we use Docker containers with pre-built third party
libraries (TPLs). These containers leverage our ``build_visit`` third party
build process. The Docker files and build scripts used to create 
these containers are in ``scripts/ci/docker``. The process to create the
container varies somewhat if you have Docker installed on the same
system as the git checkout of your branch.

Creating the container with Docker on the same system as the git checkout
-------------------------------------------------------------------------

Create the container using ``build_docker_visit_ci.py``. ::

    cd scripts/ci/docker
    python build_docker_visit_ci.py
 
This creates the container with a tag that will include today's date
and a short substring of the current git hash. 

Example Tag: ``visitdav/visit-ci-develop:2020-11-11-sha433ef0``

This will typically take several hours to complete.

Creating the container without Docker on the same system as the git checkout
----------------------------------------------------------------------------

Create two tar files and the Docker command to create the container using
``build_docker_visit_ci.py``. ::

    cd scripts/ci/docker
    python build_docker_visit_ci.py

This will create the files ::

    visit.build_visit.docker.src.tar
    visit.masonry.docker.src.tar

The command will also fail with output similar to this ::

    [exe: git rev-parse HEAD]
    [exe: docker build -t visitdav/visit-ci-develop:2020-11-11-sha433ef0 . --squash]
    Cannot connect to the Docker daemon at unix:///var/run/docker.sock. Is the docker daemon running?

You will use the ``docker build ...`` command to build the container on
the system you have Docker installed.

You can now move over to the system where you have Docker installed.
Bring up up a shell window and create a new directory or folder to contain
the files necessary to create the container. ::

    mkdir docker_ci
    cd docker_ci

Now copy all the files in ``scripts/ci/docker`` to your new folder. Now
you can run the ``docker build ...`` command to create the container. For
example: ::

    docker build -t visitdav/visit-ci-develop:2020-11-11-sha433ef0 . --squash

This will typically take several hours to complete.

Push the container to Dockerhub
-------------------------------

Now that you have created your Docker container image, you are ready to push
it to `VisIt's DockerHub Registry <https://hub.docker.com/orgs/visitdav>`_
using ``docker push <container-name>``.

If you do not already have a DockerHub account, go
`here <https://hub.docker.com/signup>`_  and sign up for one. Then
contact another member of `visitdav <https://hub.docker.com/orgs/visitdav>`_
and ask to be added to the organization.

You will need to be logged into DockerHub to successfully push. Here is
an example push command: ::

    docker login docker.io
    docker push visitdav/visit-ci-develop:2020-11-11-sha433ef0

Update VisIt_ to use the new Docker image
-----------------------------------------

To change which Docker Image is used by Azure, edit ``azure-pipelines.yml``
and change the `container_tag` variable. ::

    #####
    # TO USE A NEW CONTAINER, UPDATE TAG NAME HERE AS PART OF YOUR PR!
    #####
    variables:
      container_tag: visitdav/visit-ci-develop:2020-12-09-shaf6ef22


If you change the operating system, you will need to update the `vmImage`
variable. It is specified in two locations. ::

        pool:
          vmImage: 'ubuntu-18.04'

When the PR is merged, the Azure changes will be merged and PRs to develop 
will now use the new container.
