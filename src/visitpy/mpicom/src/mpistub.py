###############################################################################
# Python Module: mpistub
#
# Purpose: Serial stub for mpicom module.
#
# Programmer: Cyrus Harrison
# Creation: Mon Feb  8 15:56:40 PST 2010
#
#
# Modifications:
#
###############################################################################

_sval = None

def init():
    pass

def initied():
    return True;

def serial():
    return True

def parallel():
    return False

def rank():
    return 0

def size():
    return 1

def min(val):
    return val

def max(val):
    return val

def sum(val):
    return val

def mult(val):
    return val

def barrier():
    pass

def broadcast(val):
    pass

def send(val):
    global _sval
    _sval = val

def recv():
    global _sval
    return _sval

def sendrecv(val):
    return val

def gather(val):
    return [val]

def scatter(val):
    return val[0]


def finalize():
    pass