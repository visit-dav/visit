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
    if type(thing) == str:
        apresult = apropos(thing,0,True)
        if apresult:
            sys.stdout.write("NOTE: The following VisIt functions and objects also mention '%s'...\n"%str(thing))
            sys.stdout.write("%s\n"%apresult)
    else:
        try:
            python_help(thing)
        except:
            pass
        try:
            if hasattr(thing, "__name__"):
                thingstr = thing.__name__
            else:
                thingstr = str(thing)
        except:
            thingstr = str(thing)
        try:
            dirthing = [x in dir(thing) and not (x.startswith('__') and x.endswith('__'))]
            if dirthing:
                sys.stdout.write("'%s' defines the following methods...\n"%thingstr)
                sys.stdout.write("%s\n"%dirthing)
        except:
            pass
        apresult = apropos(thingstr,0,True)
        if apresult:
            sys.stdout.write("NOTE: The following VisIt functions and objects also mention '%s'...\n"%thingstr)
            sys.stdout.write("%s\n"%apresult)

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
def apropos(key, truncate=30, called_from_help=False):
    result = set()

    #
    # We want to search attrs only on objects we can safely *instance*
    # and dir() here
    #
    dirableObjectNamesContain = ['Attribute', 'SILRestriction']

    #
    # We wanna search for a string
    #
    if not isinstance(key, str):
        key = str(key)

    #
    # Loop over everything known to the visit module
    #
    for v in dir(visit):

        #
        # First, match the name of the CLI method
        #
        if re.search(key, v, re.IGNORECASE):
            result.add(v)
            continue

        #
        # Get the actual method from its name
        #
        try:
            thing = getattr(visit, v) 
        except:
            continue

        #
        # If a docstring is defined for th method, search it
        #
        if hasattr(thing,'__doc__'): 
             if thing.__doc__ is not None:
                 if re.search(key, thing.__doc__, re.IGNORECASE):
                     result.add(v)
                     continue

        #
        # If the method, when called, instantiates a dirable object,
        # search the methods it defines too.
        #
        for donc in dirableObjectNamesContain:
            if donc in v:
                try:
                    for member in dir(thing()):
                        # ignore what appear to be builtin members
                        if member.startswith('__') and member.endswith('__'):
                            continue
                        if re.search(key, member, re.IGNORECASE):
                            result.add(v)
                            break
                except:
                    pass

    if not result and not called_from_help:
        sys.stdout.write("Nothing found in VisIt for '%s'\n"%key)
        return None

    retval = sorted(list(result))
    if truncate > 0 and len(retval) > truncate:
        sys.stdout.write("More than %d VisIt functions and objects mention '%s'.\n"%(truncate,key))
        sys.stdout.write("To see the complete list, enter `apropos('%s',0)`.\n"%key)
        sys.stdout.write("To narrow the results, consider a more restrictve regular expression.\n")
        sys.stdout.write("To learn more about regular expressions type `help(re)`.\n")
        retval = retval[:truncate]
    return retval
