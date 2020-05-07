###############################################################################
from spack import *

import os
from os.path import join as pjoin

from .visit import Visit

class UberenvVisit(Visit):
    """Spack Based Uberenv Build for Visit Thirdparty Libs """

    homepage = "https://github.com/alpine-DAV/ascent"

    def url_for_version(self, version):
        dummy_tar_path =  os.path.abspath(pjoin(os.path.split(__file__)[0]))
        dummy_tar_path = pjoin(dummy_tar_path,"uberenv-visit.tar.gz")
        url      = "file://" + dummy_tar_path
        return url

    def install(self, spec, prefix):
        """
        Build and install VisIt Deps.
        """
        pass

