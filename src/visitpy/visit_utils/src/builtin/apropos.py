import inspect, re, sys
from pydoc import help as python_help
from contextlib import suppress

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
    if not isinstance(thing, str):
        try:
            python_help(thing)
        except:
            pass
        # Return early w/o adding apropos if thing is a basic type
        if isinstance(thing, (int,float,bool,complex,list,type)):
            return
    elif hasattr(visit, thing):
        try:
            python_help(getattr(visit, thing))
        except:
            pass
    try:
        if hasattr(thing, "__name__"):
            thingstr = thing.__name__
        else:
            thingstr = str(thing)
    except:
        thingstr = str(thing)
    trunc = 50
    apresult = set(apropos(key=thingstr,trunc=trunc))
    apresult = apresult - set([thingstr]) 
    apresult = sorted(list(apresult))
    if apresult:
        sys.stdout.write("Using apropos(), here are first %d VisIt items that also mention '%s'...\n"%(trunc,thingstr))
        sys.stdout.write("%s\n"%apresult)

#
# Re-define python's main help method
#
help = visit_help

#
#
def apropos(key, match='all', trunc=50):
    """Search function, object names and/or doc strings for matching text

       - key: is search text (can be a regex; help(re) for more info)
       - match: is a catentated string of words controlling behavior...
          - items: search VisIt CLI item names
          - attrs: search attributes of VisIt CLI items
          - docs: search doc strings of VisIt CLI items
          - all: search all of the above (this is default behavior)
          - case: honor case in searches. By default, case is ignored.
       - trunc: truncate response list (30 is default; 0 disables).

       Return the list of names of all items where the specified text
       (or regular expression) matches either in the item name, the
       item's attributes, if any, or the item's docstring, if any.
    """
    called_from_help = True if inspect.stack()[1].function == "visit_help" else False

    ignore_case = False if re.search(r'case', match, re.IGNORECASE) else True
    search_items = True if re.search(r'items|all', match, re.IGNORECASE) else False
    search_attrs = True if re.search(r'attrs|all', match, re.IGNORECASE) else False
    search_docs = True if re.search(r'docs|all', match, re.IGNORECASE) else False

    reflags = re.IGNORECASE if ignore_case else re.RegexFlag(0)
    result = set()

    #
    # We wanna search for a string
    #
    if not isinstance(key, str):
        key = str(key)

    #
    # Loop over everything known to the visit module
    #
    for v in dir(visit):

        # Stop loop if we're at truncated size
        if trunc > 0 and len(result) > trunc:
            break

        # First, search the name of thing itself.
        if search_items and re.search(key, v, reflags):
            result.add(v)

        # If we're not searching anything else, continue.
        if not search_attrs and not search_docs:
            continue

        # Get the actual thing handle from its name.
        thing = None
        try:
            thing = getattr(visit, v) 
        except:
            pass

        # Nothing to do if we can't get a python handle for thing.
        if not thing:
            continue

        if search_attrs:
            members = [m for m in dir(thing) \
                       if not (m.startswith('__') and m.endswith('__'))]
            if re.search(key, str(members), reflags):
                result.add(v)
                continue

        if search_docs and hasattr(thing, '__doc__') and thing.__doc__ is not None:
            if re.search(key, thing.__doc__, reflags):
                result.add(v)
                continue

    if not result and not called_from_help:
        sys.stdout.write("Nothing found in VisIt for '%s'\n"%key)
        return None

    retval = sorted(list(result))
    if trunc > 0 and len(retval) > trunc and not called_from_help:
        sys.stdout.write("More than %d VisIt items mention '%s'.\n"%(trunc,key))
        sys.stdout.write("To see a complete list, enter `apropos('%s',trunc=0)`.\n"%key)
        sys.stdout.write("To narrow results, try a more restrictve regular expression.\n")
        sys.stdout.write("To learn more about regular expressions type `help(re)`.\n")
        retval = retval[:trunc]
    return retval
