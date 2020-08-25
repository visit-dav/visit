Docker Containers For CI Testing
=================================

We use Azure Pipelines for CI testing Visit's Pull Requests.

* `Ascent Azure DevOps Space <https://dev.azure.com/visit-dav/VisIt/>`_


To speed up our CI testing we use Docker containers with pre-built third party libraries. These containers leverage our ``build_visit`` third party build  process. The Docker files and build scripts used to create these containers are in ``scripts/ci/docker``. To update the containers (assuming you have Docker installed):

 * Run ``python build_docker_visit_ci.py`` to build and tag a new version of our TPL container.

  This tag will include today's day and a short substring of the current git hash.
  Example Tag: ``visitdav/visit-ci-develop:2020-08-24-shac49c21``

 * Run ``docker push <container-name>`` to push the container image to `VisIt's DockerHub Registry <https://hub.docker.com/orgs/visitdav>`_.

  You will need to be logged into DockerHub to successfully push, the process may ask for your DockerHub username and password. Example Push Command: ``docker push visitdav/visit-ci-develop:2020-08-24-shac49c21``

 * To change which Docker Image is used by Azure, edit ``azure-pipelines.yml`` and change `container_tag` variable. ::

    #####
    # TO USE A NEW CONTAINER, UPDATE TAG NAME HERE AS PART OF YOUR PR!
    #####
    variables:
      container_tag: visitdav/visit-ci-develop:2020-08-24-shac49c21

When the PR is merged, the azure changes will be merged and PRs to develop will now use the new container.

