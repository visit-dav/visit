import re

try:
    import visit
except:
    pass

#
# Return the list of names of all methods where the specified regex
# matches either in the method name or its doc string or a stringified
# version of the object. Use a set to accumulate only unique results
# but return the result as a list. The matching is performed all in
# lower case.
#
def apropos(key):
    retval = set()
    for v in dir(visit):
        if re.search(key, v, re.IGNORECASE):
            retval.add(v)
            continue
        meth = getattr(visit, v) 
        if 'Attributes' in v:
            try:
                if re.search(key, str(meth()), re.IGNORECASE):
                    retval.add(v)
                    continue
            except:
                pass
        if hasattr(meth,'__doc__'): 
             if meth.__doc__ is not None:
                 if re.search(key, meth.__doc__, re.IGNORECASE):
                     retval.add(v)
    return list(retval)
