# Copyright (c) 2021, Lawrence Livermore National Security, LLC
#
# Python script using GraphQL interface to GitHub discussions to read
# .txt file mbox files of VisIt User's email archive and import each
# email thread as a GitHub discussion.
#
# Programmer: Mark C. Miller, Tue Jul 20 10:21:40 PDT 2021
#
import datetime, email.header, glob, mailbox, os, pytz
import re, requests, shutil, sys, textwrap, time
from difflib import SequenceMatcher

# directory containing all the .txt files from the email archive
rootDir = "/Users/miller86/visit/visit-users-email"

#
# Smarter datetime func that culls time-zone name if present and
# normalizes all datetimes to pacific (current) time zone
# CEST and EEST are handled as only first 3 letters
#
tzNames = [ \
    'GMT', 'UTC', 'ECT', 'EET', 'ART', 'EAT', 'MET', 'NET', 'PLT', 'IST', 'BST', 'VST',
    'CTT', 'JST', 'ACT', 'AET', 'SST', 'NST', 'MIT', 'HST', 'AST', 'PST', 'PNT', 'MST',
    'CST', 'EST', 'IET', 'PRT', 'CNT', 'AGT', 'BET', 'CAT', 'CET', 'PDT', 'EDT', 'CES',
    'KST', 'MSK', 'EES', 'MDT', 'CDT', 'SGT', 'AKD']
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
# items together into one long list. For testing, currently the glob
# is disabled and we're randomly looking at just one file.
#
def readAllMboxFiles():

    print("Reading messages...")
    files = glob.glob("%s/*.txt"%rootDir)
    files = sorted(files, key=lambda f: datetime.datetime.strptime(f,'/Users/miller86/visit/visit-users-email/%Y-%B.txt'))
    #files = ["/Users/miller86/visit/visit-users-email/2013-February.txt"]
    items = []
    for f in files:
        mb = mailbox.mbox(f)
        items += mb.items()
        print("    read %d items from file \"%s\""%(len(mb.items()),f),end='\r')
    print("\n%d messages read"%len(items))

    return items

#
# Reorganize single list of emails, above, as a dict of lists
# (threads) key'd by Message-ID
#
def threadMessages(items):

    print("Threading messages...")
    countRemoveByGitHub = 0
    countByMid = 0
    countRemoveReleases = 0
    countMerges = 0
    countRemoveEqualDates = 0
    msgLists = {}
    msgIds = {}
    for i in range(len(items)):
#    for i in range(3000):

        # mailbox imports all messages as a pair <int, message object>
        # So here, we get just the message object
        msg = items[i][1]

        # Skip stuff already on GitHub via SRE process
        # using raw (unfiltered) subject
        sub = msg['Subject']
        if sub and 'visit-dav' in sub:
            countRemoveByGitHub += 1
            continue
        if msg['In-Reply-To'] and 'visit-dav' in msg['In-Reply-To']:
            countRemoveByGitHub += 1
            continue
        if msg['References']:
            haveIt = False
            for r in msg['References'].split():
                if 'visit-dav' in r:
                    haveIt = True
                    break
            if haveIt:
                countRemoveByGitHub += 1
                continue

        # Try to put in an existing thread via Reference ids
        rids = msg['References']
        if rids:
            rids = rids.split()
            rids.reverse()

            foundIt = False
            for rid in rids:
                if rid in msgIds.keys():
                    subjectKey = msgIds[rid]
                    foundIt = True
                    break
            if foundIt:
                countByMid += 1
                msgLists[subjectKey] += [msg]
                continue

        # Try to put in an existing thread via In-Reply-To id
        # In-Reply-To isn't necessarily very reliable
        # see https://cr.yp.to/immhf/thread.html
        rtid = msg['In-Reply-To']
        if rtid and rtid in msgIds.keys():
            countByMid += 1
            subjectKey = msgIds[rtid]
            msgLists[subjectKey] += [msg]
            continue

        #
        # Ok, using message ids didn't work to find the
        # thread, so now try via filtered subject
        #
        sub = filterSubject(sub)

        # Remove subjects related to announcements of releases
        if re.match('visit [0-9]*.[0-9]*.[0-9]* released', sub):
            countRemoveReleases += 1
            continue

        if sub in msgLists.keys():
            msgLists[sub] += [msg]
        else:
            mid = msg['Message-ID']
            msgIds[mid] = sub
            msgLists[sub] = [msg]

    #
    # After processing all messages into threads, sort the
    # messages in each thread by date
    #
    for k in msgLists.keys():
        mlist = msgLists[k]
        msgLists[k] = sorted(mlist, key=lambda m: mydt(m['Date']))

    #
    # Remove cases of duplicate dates in same thread
    #
    for k in msgLists.keys():
        mlist = msgLists[k]
        deli = []
        for i in range(1,len(mlist)):
            if mlist[i]['Date'] and mlist[i-1]['Date'] and \
               mydt(mlist[i]['Date']) == mydt(mlist[i-1]['Date']):
                deli += [i]
        countRemoveEqualDates += len(deli)
        deli.reverse()
        for i in deli:
            del(msgLists[k][i])

    #
    # We invariably wind up with a ton of threads of length
    # one. Ordinarily, that should be rare as most messages
    # to visit-users get replied to. So, most threads should
    # be of minimum length 2. So, now make a pass trying to
    # merge any threads of size one into other threads using
    # 'similarity' of subject matches and nearnest of dates.
    #
    print("Merging threads by similarity of subjects...")
    i = 0
    for k1 in msgLists.keys():

        i += 1
        p10 = int(100*float(i)/len(msgLists))
        if not p10 % 10:
            print("    %d %% completed"%p10, end='\r')

        # Disregard these matches here
        if re.match('digest, vol [0-9]*, issue [0-9]*', k1):
            continue

        mlist1 = msgLists[k1]

        if len(mlist1) > 1:
            continue

        if len(mlist1) == 0:
            continue

        date1 = mydt(mlist1[0]['Date'])
        maxMatchRatio = 0
        maxMatchKey = None
        for k2 in msgLists.keys():
        
            if k2 == k1:
                continue

            if not msgLists[k2]:
                continue

            date2 = mydt(msgLists[k2][0]['Date'])
            ddate = date2-date1 if date2>date1 else date1-date2
            if ddate > datetime.timedelta(days=90):
                continue

            r = SequenceMatcher(None, k1, k2).ratio()
            if r < maxMatchRatio:
                continue
            maxMatchRatio = r
            maxMatchKey = k2

        if maxMatchRatio < 0.6:
            continue

        #print("Merging keys (ratio = %g)...\n    \"%s\" and\n    \"%s\"\n"%(maxMatchRatio,k1,maxMatchKey))
        countMerges += 1
        msgLists[k1] += msgLists[maxMatchKey]
        msgLists[maxMatchKey] = []

    #
    # Ok, sort threads again by date
    #
    for k in msgLists.keys():
        mlist = msgLists[k]
        msgLists[k] = sorted(mlist, key=lambda m: mydt(m['Date']))

    #
    # Remove again any cases of duplicate dates in same thread
    #
    for k in msgLists.keys():
        mlist = msgLists[k]
        deli = []
        for i in range(1,len(mlist)):
            if mlist[i]['Date'] and mlist[i-1]['Date'] and \
               mydt(mlist[i]['Date']) == mydt(mlist[i-1]['Date']):
                deli += [i]
        deli.reverse()
        countRemoveEqualDates += len(deli)
        for i in deli:
            del(msgLists[k][i])

    print("Thread %d messages by message id"%countByMid)
    print("Removed %d GitHub messages"%countRemoveByGitHub)
    print("Removed %d release messages"%countRemoveReleases)
    print("Removed %d equal date messages"%countRemoveEqualDates)
    print("Merged %d threads"%countMerges)

    return msgLists

#
# Handle (skip) all messages that had no id
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
    if '(no subject)' in msgLists.keys():
        print("Deleting %d messages with no subject"%len(msgLists['(no subject)']))
        del msgLists['(no subject)']

#
# Debug: list all the subject lines
#
def debugListAllSubjects(msgLists):
    for k in msgLists.keys():
        mlist = msgLists[k]
        print("%d messages for subject = \"%s\"\n"%(len(mlist),k))

#
# Debug: Write whole raw archive of messages, each thread to its own file
#
def debugWriteAllMessagesToFiles(msgLists):
    for k in msgLists.keys():
        mlist = msgLists[k]
        kfname = k.replace("/","_")[:100]
        with open("tmp/%s"%kfname, 'w') as f:
            for m in mlist:
                f.write("From: %s\n"%(m['From'] if m['Form'] else ''))
                f.write("Date: %s\n"%mydt(m['Date']).strftime('%a, %d %b %Y %H:%M:%S %z'))
                f.write("Subject: %s\n"%(filterSubject(m['Subject']) if m['Subject'] else ''))
                f.write("Message-ID: %s\n"%(m['Message-ID'] if m['Message-ID'] else ''))
                f.write("%s\n"%filterBody(m.get_payload()))

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
    with open("email2discussion-failures-log.txt", 'w+') as f:
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
#
def run_query(query): # A simple function to use requests.post to make the API call. Note the json= section.

    if not hasattr(run_query, 'queryTimes'):
        run_query.queryTimes = []

    # Post the request. Time it and keep 100 most recent times in a queue
    now1 = datetime.datetime.now()
    request = requests.post('https://api.github.com/graphql', json={'query': query}, headers=headers)
    now2 = datetime.datetime.now()
    run_query.queryTimes.insert(0,(now2-now1).total_seconds())
    if len(run_query.queryTimes) > 10: # keep record of last 10 query call times
        run_query.queryTimes.pop()

    if request.status_code == 200:
        return request.json()
    else:
        raise Exception("Query failed to run by returning code of {}. {} {}".format(request.status_code, query, request.json()))

#
# Get various visit-dav org. repo ids. Caches results so that subsequent
# queries don't do any graphql work.
#
def GetRepoID(reponame):
    query = """
        query
        {
            repository(owner: \"visit-dav\", name: \"%s\")
            {
                id
            }
        }
    """%reponame
    if not hasattr(GetRepoID, reponame):
        result = run_query(query)
        # result = {'data': {'repository': {'id': 'MDEwOlJlcG9zaXRvcnkzMjM0MDQ1OTA='}}}
        setattr(GetRepoID, reponame, result['data']['repository']['id'])
    return getattr(GetRepoID, reponame)


#
# Get discussion category id by name for given repo name in visit-dav org.
# Caches reponame/discname pair so that subsequent queries don't do any
# graphql work.
#
def GetDiscCategoryID(reponame, discname):
    query = """
        query
        {
            repository(owner: \"visit-dav\", name: \"%s\")
            {
                discussionCategories(first:10)
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
    """%(reponame)
    if not hasattr(GetDiscCategoryID, "%s.%s"%(reponame,discname)):
        result = run_query(query)
        # result = d['data']['repository']['discussionCategories']['edges'][0] =
        # {'node': {'description': 'Ask the community for help using VisIt', 'name': 'Help using VisIt', 'id': 'MDE4OkRpc2N1c3Npb25DYXRlZ29yeTMyOTAyNDY5'}}
        # d['data']['repository']['discussionCategories']['edges'][1]
        # {'node': {'description': 'Share cool ways you use VisIt including pictures or movies', 'name': 'Share cool stuff', 'id': 'MDE4OkRpc2N1c3Npb25DYXRlZ29yeTMyOTAyNDcw'}}
        edges = result['data']['repository']['discussionCategories']['edges']
        for e in edges:
            if e['node']['name'] == discname:
                setattr(GetDiscCategoryID, "%s.%s"%(reponame,discname), e['node']['id'])
                break
    return getattr(GetDiscCategoryID, "%s.%s"%(reponame,discname))

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
        captureGraphQlFailureDetails('createDiscussion', query, result)
    
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
        captureGraphQlFailureDetails('addDiscussionComment %s'%discid, query, result)
    
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
        captureGraphQlFailureDetails('lockLockable %s'%nodeid, query, result)

#
# Method to filter subject lines
#
def filterSubject(su):

    if not su:
        return "no subject"

    gotit = False
    if '#261' in su:
        gotit = True
        print(su)

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

    if gotit:
        print(su)

    return su.strip()

#
# Method to filter body. Currently designed towards the notion that the
# body will be rendered as "code" (between ```) and not GitHub markdown.
#
wrapper = textwrap.TextWrapper(width=100)
def filterBody(body):

    # retain only the first 20,000 chars of body
    retval = body[:20000]

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
def importMessagesAsDiscussions(msgLists, repoid, catid):

    # look for restart file
    processedKeys = restartFromRestart()

    # for k in list(msgLists.keys()): don't do whole shootin match yet
    i = 0
    for k in list(msgLists.keys())[:6]:

        if k in processedKeys:
            print("Already processed \"%s\""%k)
            continue

        # Get the current message thread
        mlist = msgLists[k]

        # Use first message (index 0) in thread for subject to
        # create a new discussion topic
        subject = filterSubject(mlist[0]['Subject'])
        print("Working on thread %d, \"%s\""%(i,k))
        print("    %d messages, subject \"%s\""%(len(mlist),subject))
        body = buildBody(mlist[0])
        discid = createDiscussion(repoid, catid, subject, body)

        # Use remaining messages in thread (starting from index 1)
        # to add comments to this discussion
        for m in mlist[1:]:
            body = buildBody(m)
            addDiscussionComment(discid, body)

        # lock the discussion to prevent any non-owners from
        # ever adding to it
        lockLockable(discid)
        i += 1

        #
        # Update restart state
        #
        updateRestart(k)

    # indicate run fully completed
    os.chmod('email2discussions-restart.txt', 0)

#
# Main Program
#

# Read all email messages into a list
items = readAllMboxFiles()

# Thread messages together
msgLists = threadMessages(items)

# Eliminate failure cases
removeBadMessages(msgLists)

# Print some diagnostics
printDiagnostics(msgLists)

testWriteMessagesToTextFiles(msgLists)

sys.exit(0)

# Get the repository id where the discussions will be created
repoid = GetRepoID("temporary-play-with-discussions")

# Get the discussion category id for the email migration discussion
catid =  GetDiscCategoryID("temporary-play-with-discussions", "VisIt Users Email Archive")

# Import all the message threads as discussions
#importMessagesAsDiscussions(msgLists, repoid, catid)

# Testing: write all message threads to a text file
#testWriteMessagesToTextFiles(msgLists)
