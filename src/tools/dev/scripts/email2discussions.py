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

# Notes:
#     Handle quoted previous email
#     Handle MIME attachments

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
    'KST', 'MSK', 'EES']
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

    #files = glob.glob("%s/*.txt"%rootDir)
    files = ["/Users/miller86/visit/visit-users-email/2013-February.txt"]
    items = []
    for f in files:
        mb = mailbox.mbox(f)
        items += mb.items()

    return items

#
# Reorganize single list of emails, above, as a dict of lists
# (threads) key'd by Message-ID
#
def threadMessages(items):

    msgLists = {}
    for i in items:

        # mailbox imports all messages as a pair <int, message object>
        # So here, we get just the message object
        msg = i[1]

        # Try to put in an existing thread via Reference ids
        rids = msg['References']
        if rids:
            rids = rids.split()
            rids.reverse()

            foundIt = False
            for rid in rids:
                if rid in msgLists.keys():
                    msgLists[rid] += [msg]
                    foundIt = True
                    break
            if foundIt:
                continue

        # Try to put in an existing thread via In-Reply-To id
        # In-Reply-To isn't necessarily very reliable
        # see https://cr.yp.to/immhf/thread.html
        rtid = msg['In-Reply-To']
        if rtid and rtid in msgLists.keys():
            msgLists[rtid] += [msg]
            continue
   
        #
        # Ok, try to match to an existing thread based on same
        # subject and date within 60 days
        # 
        if msg['Subject'] and msg['Date']:
            print(msg['Subject'])
            sub1 = filterSubject(msg['Subject'])
            date1 = mydt(msg['Date'])
            foundIt = False
            for k in msgLists.keys():
                if msgLists[k][0]['Subject'] and msgLists[k][0]['Date']:
                    sub2 = filterSubject(msgLists[k][0]['Subject'])
                    date2 = mydt(msgLists[k][0]['Date'])
                    ddate = date2-date1 if date2>date1 else date1-date2
                    if sub1 == sub2 and ddate < datetime.timedelta(days=60):
                        msgLists[k] += [msg]
                        foundIt = True
                        break
            if foundIt:
                continue

        # It looks like we can't put in an existing thread
        mid = msg['Message-ID']
        if mid not in msgLists.keys():
            msgLists[mid] = [msg]
        else:
            msgLists[mid] += [msg]

    #
    # After processing all messages into threads, sort the
    # messages in each thread by date
    #
    for k in msgLists.keys():
        mlist = msgLists[k]
        msgLists[k] = sorted(mlist, key=lambda m: mydt(m['Date']))

    return msgLists

#
# Handle (skip) all messages that had no id
#
def removeBadMessages(msgLists):
    if None in msgLists.keys():
        print("There are", len(msgLists[None]), "messages without a Message-ID")
        del msgLists[None]

#
# Debug: list all the subject lines
#
def debugListAllSubjects(msgLists):
    for k in msgLists.keys():
        mlist = msgLists[k]
        print("List for ID = \"%s\"\n"%k)
        for m in mlist:
            print(",",filterSubject(m['Subject']))
        print("\n")

#
# Debug: Write whole raw archive of messages, each thread to its own file
#
def debugWriteAllMessagesToFiles(msgLists):
    for k in msgLists.keys():
        mlist = msgLists[k]
        kfname = k.replace("/","_")
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
    maxlen = 0
    maxth = None
    for k in msgLists:
        th = msgLists[k] 
        l = len(th)
        if l > maxlen:
            maxlen = l
            maxth = th
    print("Total threads = %d"%len(msgLists))
    print("Max thread length = %d with subject..."%maxlen)
    print("    \"%s\""%filterSubject(maxth[0]['Subject']))


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

    # Handle occasional odd-ball encoding
    subytes,encoding = email.header.decode_header(su)[0]
    if isinstance(subytes,bytes):
        su = ''.join([chr(i) if i < 128 else ' ' for i in subytes])

    # handle line-wraps and other strange whitespace
    su = ' '.join(su.split()).replace('"',"'")

    # Get rid of all these terms
    stringsToRemove = ['[visit-users]', '[EXTERNAL]', '[visit-dav/live-customer-response]', '[Bulk]',
        '[BULK]', '[EXT]', '[GitHub]', '[Ieee_vis]', '[SEC=UNCLASSIFIED]', '[SEC=UNOFFICIAL]',
        '[SOLVED]', '[VISIT-USERS]', '[VisIt-users]', '[VisIt]', '[visit-announce]', '[visit-commits]',
        '[visit-core-support]', '[visit-dav/visit]', '[visit-developers]', '[visit-help-asc]',
        '[visit-help-scidac]', 'RE:', 'RE :', 'Re:', 'Fwd:']
    for s in stringsToRemove:
        su = su.replace(s,'')

    return su.strip()

#
# Method to filter body. Currently designed towards the notion that the
# body will be rendered as "code" (between ```) and not GitHub markdown.
#
wrapper = textwrap.TextWrapper(width=100)
def filterBody(body):
    retval = body

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
    # literal quote it (```).
    mylist = [wrapper.wrap(s)[0] for s in retval.split('\n') if s.strip() != '']
    retval = '\n'.join(mylist)

    return retval

#
# Simple method to build body text for a message
#
def buildBody(msgObj):

    body = ''
    body += '**Date: %s**\n'%msgObj['Date']
    body += '**From: %s**\n'%msgObj['From']
    body += '\n---\n```\n'
    body += filterBody(msgObj.get_payload())
    body += '\n```\n---\n'

    return body

#
# Debug: write message strings to be used in http/json graphql
# queries to text files
#
def testWriteMessagesToTextFiles(msgLists):

    shutil.rmtree("email2discussions-debug", ignore_errors=True)
    os.mkdir("email2discussions-debug")

    for k in list(msgLists.keys()):

        # Get the current message thread
        mlist = msgLists[k]

        # Create file name from message id (key)
        kfname = k.replace("/","_")

        # assumes 'tmp' is dir already available to write to
        with open("email2discussions-debug/%s"%kfname, 'w') as f:
            subject = filterSubject(mlist[0]['Subject'])
            body = buildBody(mlist[0])
            if subject == 'No subject' or subject == 'no subject' or subject == '':
                for s in body.split('\n')[5:]:
                    if s.strip():
                        subject = s.strip()
                        break
            print("Working on thread of %d messages \"%s\""%(len(mlist),subject))
            f.write("Subject: \"%s\"\n"%subject)
            f.write(body)
            f.write("\n")
            for m in mlist[1:]:
                body = buildBody(m)
                f.write(body)
                f.write("\n")

#
# Loop over the message list, adding each thread of
# messages as a discussion with comments
#
def importMessagesAsDiscussions(msgLists, repoid, catid):

    # for k in list(msgLists.keys()): don't do whole shootin match yet
    for k in list(msgLists.keys())[:6]:

        # Get the current message thread
        mlist = msgLists[k]

        # Use first message (index 0) in thread for subject to
        # create a new discussion topic
        subject = filterSubject(mlist[0]['Subject'])
        print("Working on thread of %d messages \"%s\""%(len(mlist),subject))
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

#
# Main Program
#

# Read all email messages into a list
items = readAllMboxFiles()

# Thread messages together
msgLists = threadMessages(items)

# Eliminate failure cases
removeBadMessages(msgLists)

#debugListAllSubjects(msgLists)

# Print some diagnostics
printDiagnostics(msgLists)

# Get the repository id where the discussions will be created
repoid = GetRepoID("temporary-play-with-discussions")

# Get the discussion category id for the email migration discussion
catid =  GetDiscCategoryID("temporary-play-with-discussions", "VisIt Users Email Archive")

# Import all the message threads as discussions
#importMessagesAsDiscussions(msgLists, repoid, catid)

# Testing: write all message threads to a text file
testWriteMessagesToTextFiles(msgLists)
