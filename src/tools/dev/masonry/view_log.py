import sys
import json
import os

from os.path import join as pjoin

sys.path.append(pjoin(os.path.split(os.path.abspath(__file__))[0],"build/lib"))

from masonry import *

if __name__ == "__main__":
    log.view(sys.argv[1])
