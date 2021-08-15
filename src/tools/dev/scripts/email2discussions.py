# Copyright (c) 2021, Lawrence Livermore National Security, LLC
#
# Python3 script using GraphQL interface to GitHub discussions to read
# .txt file mbox files of VisIt User's email archive and import each
# email thread as a GitHub discussion.
#
# 1. Reads a directory of .txt files in mbox format for messages (readAllMboxFiles)
# 2. Removes duplicate messages (removeDateAndSubjectDups) which have
#    identical dates and highly similar subjects.
# 3. Threads messages using filtered email subjects and date proximity
#    (threadMessages) in a dict of threads keyed by subject. Also wholly
#    removes some selection of message topics.
# 4. Removes various bad cases (removeBadMessages).
# 5. Reads your GitHub token from a .txt file `ghToken.txt`
# 6. Iterates over threads using GraphQl interface to create a new
#    discussion for each thread and adding remaining messages in threads
#    as comments.
# 7. Locks each resulting discussion
# 8. Applies a label to resulting discussion
# 9. Throttles (inserts sleeps) to stay within GitHub GraphQl rate
#    limits (throttleRate)
# 10. Maintains knowledge of state to safely restart and pick up
#     where it left off
# 11. Logs failure cases to a message log (text) file
# 12. In addition to GraphQl work, this script will create two files
#      - email2discussions-restart.txt: list of subject keys successfully processed
#      - email2discussions-failures-log.txt: graphql failures
# 13. You will need to modify data on these lines...
#     Params to threadMessages algorithm to control date proximity,
#         subject similarity, subject uniqeness
#     line 39: rootDir containing all mbox .txt files
#     line 46: you may need to add time-zone names
#     line 158-166: Logic to ignore certain email subjects.
#     function filterSubject: which filters email subject lines
#     function filterBody: whith filters email body
#     lines 927-end: GitHub org/user name, repo names, label names,
#     discussion names...
#
# Programmer: Mark C. Miller, Tue Jul 20 10:21:40 PDT 2021
#
import datetime, email.header, glob, mailbox, os, pytz
import re, requests, shutil, sys, textwrap, time
from difflib import SequenceMatcher

# directory containing all the .txt files from the email archive
#rootDir = "/Users/miller86/visit/visit-users-email"
rootDir = "/Users/miller86/visit/visit-developers-email"

#
# Smarter datetime func that culls time-zone name if present and
# normalizes all datetimes to pacific (current) time zone
# CEST and EEST are handled as only first 3 letters
#
tzNames = [ \
    'GMT', 'UTC', 'ECT', 'EET', 'ART', 'EAT', 'MET', 'NET', 'PLT', 'IST', 'BST', 'VST',
    'CTT', 'JST', 'ACT', 'AET', 'SST', 'NST', 'MIT', 'HST', 'AST', 'PST', 'PNT', 'MST',
    'CST', 'EST', 'IET', 'PRT', 'CNT', 'AGT', 'BET', 'CAT', 'CET', 'PDT', 'EDT', 'CES',
    'KST', 'MSK', 'EES', 'MDT', 'CDT', 'SGT', 'AKD', 'US/']
def mydt(d):
    if not d:
        return datetime.datetime.now().astimezone()
    tzn = d.split()[-1][1:4]
    if tzn in tzNames:
        d = " ".join(d.split()[:-1]) # removes the last term (time zone name)
    try: 
        return datetime.datetime.strptime(d,'%a, %d %b %Y %H:%M:%S %z').astimezone()
    except:
        try:
            return datetime.datetime.strptime(d,'%a, %d %b %Y %H:%M %z').astimezone()
        except:
            return datetime.datetime.strptime(d,'%a, %d %b %Y %H:%M:%S').astimezone()

#
# Iterate over all files loading each as a mailbox and catenating the
# items together into one long list. Sort resulting list by date
#
def readAllMboxFiles():

    print("Reading messages...")
    files = glob.glob("%s/*.txt"%rootDir)
    files = sorted(files, key=lambda f: datetime.datetime.strptime(f,rootDir+'/%Y-%B.txt'))
    items = []
    for f in files:
        mb = mailbox.mbox(f)
        items += mb.items()
        print("    read %03d items from file \"%s\"      "%(len(mb.items()),f),end='\r')
    print("\n%d messages read"%len(items))

    print("Sorting messages by date...")
    sitems = sorted(items, key=lambda m: mydt(m[1]['Date']))
    print("Done")

    return sitems

#
# If adjacent items have identical dates and highly similar subjects
# they are probably dups. Remove them now.
#
def removeDateAndSubjectDups(sitems):
    i = 0
    count = len(sitems)-1
    rmcount = 0
    while i < count:
        curdate = mydt(sitems[i][1]['Date'])
        cursubj = filterSubject(sitems[i][1]['Subject'])
        j = i+1
        while (j < count) and (mydt(sitems[j][1]['Date']) == curdate):
            cksubj = filterSubject(sitems[j][1]['Subject'])
            r = SequenceMatcher(None, cursubj, cksubj).ratio()
            if r > 0.6:
                del sitems[j]
                count -= 1
                rmcount += 1
            else:
                j += 1
        i += 1
    print("Removed %d dups with equal datetimes and similar subjects"%rmcount)
    return sitems

#
# Thread messages keyed by filtered subject and within 90 days of
# each other. When an exact subject match fails, try a similarity
# match before giving up and creating a new thread.
#
#    - timeDeltaDays is how wide a window, in days, is used to match
#      similar (not identical) subjects
#    - similarityThreshold is the minimum similarity ratio (defined by
#      sequence-matcher) to consider two different subjects the same.
#      The default value of 0.6 is that recommended by SequenceMatcher docs.
#    - uniqSubjLen is the minimum length of a subject, in characters,
#      that matching instances more than timeDeltaDays apart are still
#      considered part of the *same* thread.
#
def threadMessages(sitems, timeDeltaDays=90, similarityThreshold=0.6, uniqSubjLen=25):
    print("Threading...")

    threads = {}
    recentSubjects = [] 
    recentDates = []
    coincidentalSubjectCounts = {}
    for i in range(len(sitems)):

        # Dereference the current message
        msg = sitems[i][1]

        # Output progress information
        p = int(100*float(i)/len(sitems))
        print("    %02d %% completed, recent subjects count=%03d"%(p,len(recentSubjects)), end='\r')

        # Ignore messgaes that appear to be GitHub related
        # notifications using raw (unfiltered) subject
        sub = msg['Subject']
        if sub and 'visit-dav' in sub:
            continue
        if msg['In-Reply-To'] and 'visit-dav' in msg['In-Reply-To']:
            continue
        if msg['References'] and 'visit-dav' in msg['References']:
            continue

        curdate = mydt(msg['Date'])
        cursubj = filterSubject(msg['Subject'])

        # Ignore messages with subjects that appear to be announcements of releases
        # or subversion commit/update messages or test suite runs
        if re.match('visit [0-9]*.[0-9]*.[0-9]* released', cursubj):
            continue
        if 'svn' in cursubj and 'update' in cursubj: 
            continue
        if 'svn' in cursubj and 'commit' in cursubj: 
            continue
        if 'test suite run' in cursubj and \
            ('passed' in cursubj or 'failed' in cursubj):
            continue

        # Keep recent subjects/dates up to date by deleting entries (at beginning)
        # timeDeltaDays days older than current. This works because the input messages
        # list is already sorted by date.
        while recentDates and \
           (curdate - recentDates[0]) > datetime.timedelta(days=timeDeltaDays):
            del recentDates[0]
            del recentSubjects[0]
 
        # Try exact match first
        if cursubj in recentSubjects:
            threads[cursubj] += [msg]
            idx = recentSubjects.index(cursubj)
            del recentSubjects[idx]
            del recentDates[idx]
            recentSubjects += [cursubj]
            recentDates += [curdate]
            continue

        # Try exact match on modified subject if this a subject
        # for which we have multiple threads
        if cursubj in coincidentalSubjectCounts.keys():
            cursubj1 = cursubj + "~%d"%coincidentalSubjectCounts[cursubj]
            if cursubj1 in recentSubjects:
                threads[cursubj1] += [msg]
                idx = recentSubjects.index(cursubj1)
                del recentSubjects[idx]
                del recentDates[idx]
                recentSubjects += [cursubj1]
                recentDates += [curdate]
                continue

        # Ok, try fuzzy match by taking the *first* maximum-ratio match
        # for which that maximum ratio exceeds our similarity threshold
        ratios = [SequenceMatcher(None, cursubj, recentSubjects[j]).ratio() for j in range(len(recentSubjects))]
        maxr = max(ratios) if ratios else 0
        if maxr > similarityThreshold:
            maxi = ratios.index(maxr)
            cursubj1 = recentSubjects[maxi]
            threads[cursubj1] += [msg]
            del recentSubjects[maxi]
            del recentDates[maxi]
            recentSubjects += [cursubj1]
            recentDates += [curdate]
            continue

        #
        # Looks like a new thread. However, it could coincidentally have a
        # subject identical to a more than timeDeltaDays day old thread. This
        # is common for subjects like "question" or "help" or "no subject", etc.
        # The more characters there are in a thread subject, the more unique
        # it is and so identical matches here for longer subject names are more
        # than likely part of the same thread even if separated in time more
        # than timeDeltaDays days.
        #
        if cursubj in threads.keys():

            if len(cursubj) > uniqSubjLen:

                # add to existing thread
                threads[cursubj] += [msg]
                try:
                    idx = recentSubjects.index(cursubj)
                    del recentSubjects[idx]
                    del recentDates[idx]
                except ValueError:
                    pass
                recentSubjects += [cursubj]
                recentDates += [curdate]
                continue

            else:

                # This logic deals with possible same subject separated in time
                # by more than our timeDeltaDays threshold. Encountering the same
                # subject more than timeDeltaDays days later is treated as a *new*
                # thread of that subject so we append a number/count to the subject.
                if cursubj in coincidentalSubjectCounts.keys():
                    coincidentalSubjectCounts[cursubj] += 1
                else:
                    coincidentalSubjectCounts[cursubj] = 1
                cursubj1 = cursubj + "~%d"%coincidentalSubjectCounts[cursubj]

                # start a new thread on this modified subject
                threads[cursubj1] = [msg]
                recentSubjects += [cursubj1]
                recentDates += [curdate]
                continue

        # start a *new* thread
        threads[cursubj] = [msg]
        recentSubjects += [cursubj]
        recentDates += [curdate]

    print("\nDone")
    print("Subjects with multiple instances in time...")
    for s in coincidentalSubjectCounts.keys():
        print("   %d distinct threads with subject \"%s\""%(coincidentalSubjectCounts[s],s))

    return threads

#
# Remove certain bad cases
#
def removeBadMessages(msgLists):
    if None in msgLists.keys():
        print("There are", len(msgLists[None]), "messages with subject = None")
        del msgLists[None]
    if '' in msgLists.keys():
        print("There are", len(msgLists['']), "messages with subject = ''")
        del msgLists['']
    delItems = []
    for k in msgLists.keys():
        if len(msgLists[k]) <= 1:
            delItems += [k]
    print("Deleting %d threads of size <= 1"%len(delItems))
    for i in delItems:
        del msgLists[i]

#
# Debug: list all the subject lines
#
def debugListAllSubjects(msgLists):
    for k in msgLists.keys():
        mlist = msgLists[k]
        print("%d messages for subject = \"%s\"\n"%(len(mlist),k))

#
# Debug: print some diagnostics info
#
def printDiagnostics(msgLists):
    nummsg = 0
    maxlen = 0
    maxmsglen = 0
    maxth = None
    maxmsgth = None
    for k in msgLists:
        th = msgLists[k] 
        for m in th:
            msglen = len(m.get_payload())
            if msglen > maxmsglen:
                maxmsglen = msglen
                maxmsgth = th
        l = len(th)
        if l > maxlen:
            maxlen = l
            maxth = th
        nummsg += l
    print("Total threaded messages = %d"%nummsg)
    print("Total threads = %d"%len(msgLists))
    print("Max thread length = %d with subject..."%maxlen)
    print("    \"%s\""%filterSubject(maxth[0]['Subject']))
    print("Max message body size = %d"%maxmsglen)
    print("    \"%s\""%filterSubject(maxmsgth[0]['Subject']))


#
# Capture failure details to a continuously appending file
#
def captureGraphQlFailureDetails(gqlQueryName, gqlQueryString, gqlResultString):
    with open("email2discussions-failures-log.txt", 'a') as f:
        f.write("%s - %s\n"%(datetime.datetime.now().strftime('%y%b%d %I:%M:%S'),gqlQueryName))
        f.write("--------------------------------------------------------------------------\n")
        f.write(gqlResultString)
        f.write("\n")
        f.write("--------------------------------------------------------------------------\n")
        f.write(gqlQueryString)
        f.write("\n")
        f.write("--------------------------------------------------------------------------\n\n\n\n")

#
# Read token from 'ghToken.txt'
#
def GetGHToken():
    if not hasattr(GetGHToken, 'ghToken'):
        try:
            with open('ghToken.txt', 'r') as f:
                GetGHToken.ghToken = f.readline().strip()
        except:
            raise RuntimeError('Put a GitHub token in \'ghToken.txt\' readable only by you.')
    return GetGHToken.ghToken

#
# Build standard header for URL queries
#
headers = \
{
    'Content-Type': 'application/json',
    'Authorization': 'bearer %s'%GetGHToken(),
    'GraphQL-Features': 'discussions_api'
}

#
# Workhorse routine for performing a GraphQL query
# # A simple function to use requests.post to make the API call. Note the json= section.
def run_query(query):

    if not hasattr(run_query, 'numSuccessiveFailures'):
        run_query.numSuccessiveFailures = 0;

    # Post the request. Check for possible error return and sleep and retry if so.
    try:
        request = requests.post('https://api.github.com/graphql', json={'query': query}, headers=headers)
        result = request.json()

        i = 0
        while 'errors' in result and i < 100:
            print("....retrying \"%s\" after sleeping 3 seconds"%query[:30])
            time.sleep(3)
            request = requests.post('https://api.github.com/graphql', json={'query': query}, headers=headers)
            result = request.json()
            i = i + 1

        if 'errors' in result and i == 100:
            raise Exception(">100 successive query failures, exiting...")
            sys.exit(1)

        run_query.numSuccessiveFailures = 0

    except:

        captureGraphQlFailureDetails('run_query', query, "")
        run_query.numSuccessiveFailures += 1
        if run_query.numSuccessiveFailures > 3:
            raise Exception(">3 successive query failures, exiting...")
            sys.exit(1)

    if request.status_code == 200:
        return request.json()
    else:
        raise Exception("run_query failed with code of {}. {} {}".format(request.status_code, query, request.json()))

#
# A method to periodically call to ensure we don't
# exceed GitHub's rate limits. It costs us part of our limit to
# call this so we don't want to call it all the time. It is coded
# to do *real* GraphQl work only once per minute, no matter how
# often it is actually called.
#
def throttleRate():

    # set the *last* check 61 seconds in the past to force a check
    # the very *first* time we run this
    if not hasattr(throttleRate, 'lastCheckNow'):
        throttleRate.lastCheckNow = datetime.datetime.now()-datetime.timedelta(seconds=61)

    query = """
        query
        {
            viewer
            {
                login
            }
            rateLimit
            {
                limit
                remaining
                resetAt
            }
        }
    """

    # Perform this check only about once a minute
    now = datetime.datetime.now()
    if (now - throttleRate.lastCheckNow).total_seconds() < 60:
        return
    throttleRate.lastCheckNow = now

    try:
        result = run_query(query)

        zuluOffset = 7 * 3600 # subtract PDT timezone offset from Zulu
        
        if 'errors' in result.keys():
            toSleep = (throttleRate.resetAt-now).total_seconds() - zuluOffset + 1
            print("Reached end of available queries for this cycle. Sleeping %g seconds..."%toSleep)
            time.sleep(toSleep)
            return

        # Gather rate limit info from the query result
        limit = result['data']['rateLimit']['limit']
        remaining = result['data']['rateLimit']['remaining']
        # resetAt is given in Zulu (UTC-Epoch) time
        resetAt = datetime.datetime.strptime(result['data']['rateLimit']['resetAt'],'%Y-%m-%dT%H:%M:%SZ')
        toSleep = (resetAt-now).total_seconds() - zuluOffset
        print("GraphQl Throttle: limit=%d, remaining=%d, resetAt=%g seconds"%(limit, remaining, toSleep))

        # Capture the first valid resetAt point in the future
        throttleRate.resetAt = resetAt

        if remaining < 200:
            print("Reaching end of available queries for this cycle. Sleeping %g seconds..."%toSleep)
            time.sleep(toSleep)

    except:
        captureGraphQlFailureDetails('rateLimit', query, "")

#
# Get various visit-dav org. repo ids. Caches results so that subsequent
# queries don't do any graphql work.
#
def GetRepoID(orgname, reponame):
    query = """
        query
        {
            repository(owner: \"%s\", name: \"%s\")
            {
                id
            }
        }
    """%(orgname, reponame)
    if not hasattr(GetRepoID, reponame):
        result = run_query(query)
        # result = {'data': {'repository': {'id': 'MDEwOlJlcG9zaXRvcnkzMjM0MDQ1OTA='}}}
        setattr(GetRepoID, reponame, result['data']['repository']['id'])
    return getattr(GetRepoID, reponame)


#
# Get object id by name for given repo name and org/user name. 
# Caches reponame/objname pair so that subsequent queries don't do any
# graphql work.
#
def GetObjectIDByName(orgname, reponame, gqlObjname, gqlCount, objname):
    query = """
        query
        {
            repository(owner: \"%s\", name: \"%s\")
            {
                %s(first:%d)
                {
                    edges
                    {  
                        node
                        {
                            description,
                            id,
                            name
                        }
                    }
                } 
            }
        }
    """%(orgname, reponame, gqlObjname, gqlCount)
    if not hasattr(GetObjectIDByName, "%s.%s"%(reponame,objname)):
        result = run_query(query)
        # result = d['data']['repository']['discussionCategories']['edges'][0] =
        edges = result['data']['repository'][gqlObjname]['edges']
        for e in edges:
            if e['node']['name'] == objname:
                setattr(GetObjectIDByName, "%s.%s"%(reponame,objname), e['node']['id'])
                break
    return getattr(GetObjectIDByName, "%s.%s"%(reponame,objname))

#
# Create a discussion and return its id
#
def createDiscussion(repoid, catid, subject, body):
    query = """
        mutation 
        {
            createDiscussion(input:
            {
                repositoryId:\"%s\",
                categoryId:\"%s\"
                title:\"%s\",
                body:\"%s\"
            }) 
            {
                discussion
                {
                    id
                }
            }
        }
    """%(repoid, catid, subject, body)
    try:
        result = run_query(query)
        # {'data': {'createDiscussion': {'discussion': {'id': 'MDEwOkRpc2N1c3Npb24zNDY0NDI1'}}}}
        return result['data']['createDiscussion']['discussion']['id']
    except:
        captureGraphQlFailureDetails('createDiscussion', query,
            repr(result) if 'result' in locals() else "")
    
    return None

#
# Add a comment to a discussion
#
def addDiscussionComment(discid, body):
    query = """
        mutation 
        {
            addDiscussionComment(input:
            {
                discussionId:\"%s\",
                body:\"%s\"
            }) 
            {
                comment
                {
                    id
                }
            }
        }
    """%(discid, body)
    try:
        result = run_query(query)
        # {'data': {'addDiscussionComment': {'comment': {'id': 'MDE3OkRpc2N1c3Npb25Db21tZW50MTAxNTM5Mw=='}}}}
        return result['data']['addDiscussionComment']['comment']['id']
    except:
        captureGraphQlFailureDetails('addDiscussionComment %s'%discid, query,
            repr(result) if 'result' in locals() else "")
    
    return None

#
# lock an object (primarily to lock a discussion)
#
def lockLockable(nodeid):
    query = """
        mutation
        {
            lockLockable(input:
            {
                clientMutationId:\"scratlantis:emai2discussions.py\",
                lockReason:RESOLVED,
                lockableId:\"%s\"
            })
            {
                lockedRecord
                {
                    locked
                }
            }
        }"""%nodeid
    try:
        result = run_query(query)
    except:
        captureGraphQlFailureDetails('lockLockable %s'%nodeid, query,
            repr(result) if 'result' in locals() else "")

#
# Add a convenience label to each discussion
# The label id was captured during startup
#
def addLabelsToLabelable(nodeid, labid):
    query = """
        mutation
        {
            addLabelsToLabelable(input:
            {
                clientMutationId:\"scratlantis:emai2discussions.py\",
                labelIds:[\"%s\"],
                labelableId:\"%s\"
            })
            {
                labelable
                {
                    labels(first:1)
                    {
                        edges
                        {  
                            node
                            {
                                id
                            }
                        }
                    }
                }
            }
        }"""%(labid, nodeid)
    try:
        result = run_query(query)
    except:
        captureGraphQlFailureDetails('addLabelsToLabelable %s'%nodeid, query,
            repr(result) if 'result' in locals() else "")

#
# Method to filter subject lines
#
def filterSubject(su):

    if not su:
        return "no subject"

    # Handle occasional odd-ball encoding
    suparts = email.header.decode_header(su)
    newsu = ''
    for p in suparts:
        if isinstance(p[0],bytes):
            try:
                newsu += p[0].decode('utf-8')
            except UnicodeDecodeError:
                newsu += ''.join([chr(i) if i < 128 else ' ' for i in p[0]])
        else:
            newsu += p[0]
    su = newsu

    # handle line-wraps and other strange whitespace
    su = re.sub('\s+',' ', su)
    su = su.replace('"',"'")
    su = su.lower()

    # Get rid of all these terms
    stringsToRemove = ['visit-users', '[external]', 'visit-dav/live-customer-response', '[bulk]',
        '[ext]', '[github]', '[ieee_vis]', '[sec=unclassified]', '[sec=unofficial]',
        '[solved]', '[visit-announce]', '[visit-commits]',
        'visit-core-support', 'visit-dav/visit', 'visit-developers', 'visit-help-asc',
        'visit-help-scidac', 're:', 're :', 'fwd:', 'fw:', '[unclassifed]',
        '[non-dod source]', 'possible spam', 'suspicious message', 'warning: attachment unscanned',
        'warning: unscannable extraction failed', '[]', '()', '{}']

    for s in stringsToRemove:
        su = su.replace(s,'')

    # Get rid of GitHub bug identifiers
    su = re.sub('\s+\(#[0-9]*\)','',su)

    return su.strip()

#
# Replacement function for re.sub to replace phone number matches with
# a string of the same number of characters
#
def overwriteChars(m):
    return 'X' * len(m.group())

#
# Method to filter body. Currently designed towards the notion that the
# body will be rendered as "code" (between ```) and not GitHub markdown.
#
wrapper = textwrap.TextWrapper(width=100)
def filterBody(body):

    retval = body[:20000]+' truncated...' if (len(body)) > 20000 else body

    # filter out anything that looks like a phone number including international #'s
    # Unfortunately, this can corrupt any lines of raw integer or floating point
    # data in the email body. Masking telephone numbers trumps raw data though.
    retval = re.sub('[ ({[]?[0-9]{3}[ )}\]]?[-\.+=: ]?[0-9]{3}[-\.+=: ]?[0-9]{4}',
        overwriteChars,retval,0,re.MULTILINE)
    retval = re.sub('[ ({[]?[0-9]{3}[ )}\]]?[-\.+=: ]?[0-9]{4}[-\.+=: ]?[0-9]{4}',
        overwriteChars,retval,0,re.MULTILINE)
    retval = re.sub('[ ({[]?[0-9]{2}[ )}\]]?[-\.+=: ]?[ ([]?[0-9]{2}[ )\]]?[-\.+=: ]?[0-9]{4}[-\.+=: ]?[0-9]{4}',
        overwriteChars,retval,0,re.MULTILINE)
    retval = re.sub('[ ({[]?[0-9]{3}[ )}\]]?[-\.+=: ]?[ ([]?[0-9]{1}[ )\]]?[-\.+=: ]?[0-9]{3}[-\.+=: ]?[0-9]{4}',
        overwriteChars,retval,0,re.MULTILINE)
    retval = re.sub('[ ({[]?[0-9]{3}[ )}\]]?[-\.+=: ]?[ ([]?[0-9]{1}[ )\]]?[-\.+=: ]?[0-9]{2}[-\.+=: ]?[0-9]{2}[-\.+=: ]?[0-9]{2}[-\.+=: ]?[0-9]{2}',
        overwriteChars,retval,0,re.MULTILINE)

    # Remove these specific lines. In many cases, these lines are quoted and
    # re-wrapped (sometimes with chars inserted in arbitrary places) so this isn't
    # foolproof.
    retval = re.sub('^[>\s]*VisIt Users Wiki: http://[ +_\*]*visitusers.org/.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*Frequently Asked Questions for VisIt: http://[ +_\*]*visit.llnl.gov/FAQ.html.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*To Unsubscribe: send a blank email to visit-developers-unsubscribe at elist.ornl.gov.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*More Options: https://[ +_\*]*elist.ornl.gov/mailman/listinfo/visit-developers.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*To Unsubscribe: send a blank email to.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*visit-developers-unsubscribe at elist.ornl.gov.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*-------------- next part --------------.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*An HTML attachment was scrubbed\.\.\..*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*URL: <?https://[ +_\*]*elist.ornl.gov/pipermail/visit-developers/attachments.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*URL: <?https://[ +_\*]*email.ornl.gov/pipermail/visit-developers/attachments.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*List subscription information: https://[ +_\*]*email.ornl.gov/mailman/listinfo/visit-developers.*$', '',retval,0,re.MULTILINE)
    retval = re.sub('^[>\s]*Searchable list archives: https://[ +_\*]*email.ornl.gov/pipermail/visit-users.*$', '',retval,0,re.MULTILINE)

    #
    # Filter out signature separator lines (e.g. '--') as these convince
    # GitHub the message is really HTML formatted
    #
    retval = re.sub('^\s*-+\s*$','\n---\n',retval,0,re.MULTILINE)
    
    # Take out some characters that cause problems with http/json parsing
    retval = retval.replace('\\',' ')
    retval = retval.replace('"',"'")

    # Take out some characters that might be intepreted as premature end
    # of the code block in which this body text is being embedded.
    retval = retval.replace('```',"'''")
    retval = retval.replace('~~~',"'''")

    # wrap the body text for GitHub text box size (because we're going to
    # literal quote it (```) and want to avoid creating content that requires
    # horiz. scroll
    mylist = []
    for s in retval.split('\n'):
        more = wrapper.wrap(s.strip())
        if not more and mylist and mylist[-1] != '':
            mylist += ['']
        else:
            mylist += more
    retval = '\n'.join(mylist)

    return retval

#
# Simple method to build body text for a message
#
def buildBody(msgObj):

    body = ''
    body += '**Date: %s**\n'%mydt(msgObj['Date']).strftime('%a, %d %b %Y %H:%M:%S %z')
    body += '**From: %s**\n'%msgObj['From']
    body += 'This post was [imported from the visit-developers@ornl.gov email archive](https://github.com/visit-dav/visit/wiki/About-the-visit-users@ornl.gov-email-archive)\n'
    body += '\n---\n```\n'
    body += filterBody(msgObj.get_payload())
    body += '\n```\n---\n'

    return body

#
# load the restart file
#
def restartFromRestart():
    processedKeys = []
    if os.path.exists('email2discussions-restart.txt'):
        if not os.access('email2discussions-restart.txt', os.R_OK):
            raise RuntimeError('It appears a previous run has fully completed. Remove "email2discussions-restart.txt" to rerun.')
        with open('email2discussions-restart.txt', 'r') as f:
            processedKeys = [l.strip() for l in f.readlines()]
    return processedKeys

#
# Update our restart file
#
def updateRestart(k):
    with open('email2discussions-restart.txt', 'a') as f:
        f.write(k)
        f.write('\n')

#
# Debug: write message strings to be used in http/json graphql
# queries to text files
#
def testWriteMessagesToTextFiles(msgLists):

    shutil.rmtree("email2discussions-debug", ignore_errors=True)
    os.mkdir("email2discussions-debug")

    processedKeys = restartFromRestart()

    i = 0
    for k in list(msgLists.keys()):

        if k in processedKeys:
            print("Already processed \"%s\""%k)
            continue

        # Get the current message thread
        mlist = msgLists[k]

        # Create a valid file name from message id (key)
        #time.sleep(1) 
        kfname = k.replace("/","_").replace('<','_').replace('>','_')[:100]

        # assumes 'tmp' is dir already available to write to
        with open("email2discussions-debug/%s"%kfname, 'w') as f:
            subject = filterSubject(mlist[0]['Subject'])
            body = buildBody(mlist[0])
            if subject == 'No subject' or subject == 'no subject' or subject == '':
                for s in body.split('\n')[5:]:
                    if s.strip():
                        subject = s.strip()
                        break
            print("Working on thread %d, \"%s\""%(i,k))
            print("    %d messages, subject \"%s\""%(len(mlist),subject))
            f.write("Subject: \"%s\"\n"%subject)
            f.write(body)
            f.write("\n")
            for m in mlist[1:]:
                body = buildBody(m)
                f.write(body)
                f.write("\n")
        i += 1

        updateRestart(k)

    # indicate run fully completed
    os.chmod('email2discussions-restart.txt', 0)


#
# Loop over the message list, adding each thread of
# messages as a discussion with comments
#
def importMessagesAsDiscussions(msgLists, repoid, catid, labid):

    # look for restart file
    processedKeys = restartFromRestart()

    # for k in list(msgLists.keys()): don't do whole shootin match yet
    i = 0
    for k in list(msgLists.keys()):

        i += 1

        if k in processedKeys:
            print("Already processed \"%s\""%k)
            continue

        # Make sure we don't exceed GitHub's GraphQL API limits
        throttleRate()

        # Get the current message thread
        mlist = msgLists[k]

        # Use first message (index 0) in thread for subject to
        # create a new discussion topic
        subject = filterSubject(mlist[0]['Subject'])
        print("Working on thread %d of %d (%d messages, subject = \"%s\")"%(i,len(msgLists.keys()),len(mlist),k))
        body = buildBody(mlist[0])
        discid = createDiscussion(repoid, catid, subject, body)

        # label this discussion for easy filtering
        addLabelsToLabelable(discid, labid)

        # Use remaining messages in thread (starting from index 1)
        # to add comments to this discussion
        for m in mlist[1:]:
            body = buildBody(m)
            addDiscussionComment(discid, body)

        # lock the discussion to prevent any non-owners from
        # ever adding to it
        lockLockable(discid)

        #
        # Update restart state
        #
        updateRestart(k)

    # indicate run fully completed
    os.chmod('email2discussions-restart.txt', 0)

#
# Main Program
#

# Read all email messages into a list sorted by date
items = readAllMboxFiles()

# Remove duplicates
items = removeDateAndSubjectDups(items)

# Thread the messages
msgLists = threadMessages(items)

# Eliminate common bad cases
removeBadMessages(msgLists)

#printDiagnostics(msgLists)
#testWriteMessagesToTextFiles(msgLists)
#sys.exit(0)

# Get the repository id where the discussions will be created
repoid = GetRepoID("visit-dav", "visit")

# Get the discussion category id for the email migration discussion
catid =  GetObjectIDByName("visit-dav", "visit", "discussionCategories", 10, "visit-developers email archive")

# Get the label id for the 'visit-uers email'
labid =  GetObjectIDByName("visit-dav", "visit", "labels", 30, "email archive")

# Import all the message threads as discussions
importMessagesAsDiscussions(msgLists, repoid, catid, labid)
