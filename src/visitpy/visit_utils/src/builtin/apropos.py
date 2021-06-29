import re, sys
from pydoc import help as python_help

try:
    import visit
except:
    pass

#
# Define an override for standard python `help` method
# It calls normal python help but also returns an apropos
# result if it would be useful.
#
def visit_help(thing):
    try:
        python_help(thing)
    except:
        pass
    apresult = apropos(thing)
    if apresult:
        sys.stdout.write("NOTE: The following VisIt functions and objects also mention '%s'...\n"%str(thing))
        sys.stdout.write("%s\n"%apropos(thing))

#
# Re-define python's main help method
#
help = visit_help

#
# Return the list of names of all methods where the specified regex
# matches either in the method name or its doc string or a strinified
# version of the object. Use a set to accumulate only unique results
# but return the result as a list. The matching is performed all in
# lower case.
#
def apropos(key):
    retval = set()

    if not isinstance(key, str):
        key = str(key)

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

    if not retval:
        sys.stdout.write("Nothing found in VisIt for '%s'\n"%key)
        return None

    return list(retval)
