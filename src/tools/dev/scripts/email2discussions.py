import datetime, glob, mailbox, os, pytz, re, requests, sys, time

# Notes:
#     Handle multi-line subjects.
#     Handle quoted previous email
#     Handle MIME attachments
#     Keep subject and date

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
# items together into one long list
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

        msg = i[1]

        # Try to put in an existing thread via References
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

        # Try to put in an existing thread via In-Reply-To
        rtid = msg['In-Reply-To']
        if rtid and rtid in msgLists.keys():
            msgLists[rtid] += [msg]
            continue
   
        # It looks like we can't put in an existing thread
        mid = msg['Message-ID']
        if mid not in msgLists.keys():
            msgLists[mid] = [msg]
        else:
            msgLists[mid] += [msg]

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
            su = " ".join(m['Subject'].split())
            print(",",su)
        print("\n")

#
# Debug: Write each message list to a file
#
def debugWriteMessagesToFiles(msgLists):
    for k in msgLists.keys():
        mlist = msgLists[k]
        kfname = k.replace("/","_")
        with open("tmp/%s"%kfname, 'w') as f:
            for m in mlist:
                f.write("From: %s\n"%(m['From'] if m['Form'] else ''))
                f.write("Gorfo-Date: %s\n"%mydt(m['Date']).strftime('%a, %d %b %Y %H:%M:%S %z'))
                f.write("Subject: %s\n"%(m['Subject'] if m['Subject'] else ''))
                f.write("Message-ID: %s\n"%(m['Message-ID'] if m['Message-ID'] else ''))
                f.write("%s\n"%m.get_payload())

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
# Read token from 'ghToken.txt'
#
def GetGHToken():
    if not hasattr(GetGHToken, 'ghToken'):
        try:
            with open('ghToken.txt', 'r') as f:
                GetGHToken.ghToken = f.readline().strip()
        except:
            raise RuntimeError('Unable to read \'ghToken.txt\' file with your GitHub token')
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
# Workhorse routine for performing a query
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

def throttleRates():
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
                cost
                remaining
                resetAt
            }
        }
    """
    try:
        result = run_query(query)

        # Gather rate limit info from the query result
        resetAt = datetime.datetime.strptime(result['data']['rateLimit']['resetAt'],'%Y-%m-%dT%H:%M:%SZ')
        remaining = result['data']['rateLimit']['remaining']
        limit = result['data']['rateLimit']['limit']
        cost = result['data']['rateLimit']['cost']

        # calculate rates
        burnOutCummRate   = float(remaining) / (resetAt - now2).total_seconds()
        currentCummRate   = float(limit - remaining) / (now2 - run_query.start).total_seconds()
        thisQueryInstRate = float(cost) / (now2 - now1).total_seconds()

        if thisQueryInstRate > burnOutCummRate:
            ratio = thisQueryInstRate / burnOutCummRate
            delay = (now2-now1).total_seconds()*ratio
            print("Burn out rate = %f, current rate = %f, this query rate = %f"%\
                (burnOutCummRate, currentCummRate, thisQueryInstRate))
            print("Sleeping %f seconds to stay within rate limit"%delay)
            time.sleep(delay)

    except:
        pass

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
        print(result)
        # result = {'data': {'repository': {'id': 'MDEwOlJlcG9zaXRvcnkzMjM0MDQ1OTA='}}}
        setattr(GetRepoID, reponame, result['data']['repository']['id'])
    return getattr(GetRepoID, reponame)


#
# Get discussion category id by name for given repo name in visit-dav org.
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
        print("result", result)
        print("subject", subject)
        print("body", body)
    
    return None

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
        print(result)
        print("body", body)
    
    return None

# lock an object (primarily to lock a discussion)
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
        print(result)
    except:
        print(result)
        print("unable to lock", nodeid)

def filterSubject(su):
    return " ".join(su.split()).replace('"',"'")

def filterBody(body):
    retval = body
    retval = retval.replace('"',"'")
    return retval

# Read all email messages into a list
items = readAllMboxFiles()

# Thread messages together
msgLists = threadMessages(items)

# Eliminate failure cases
removeBadMessages(msgLists)

#debugListAllSubjects(msgLists)

# Print some diagnostics
printDiagnostics(msgLists)

sys.exit(1)

# Get the repository id where the discussions will be created
repoid = GetRepoID("temporary-play-with-discussions")

# Get the discussion category id for the email migration discussion
catid =  GetDiscCategoryID("temporary-play-with-discussions", "VisIt Users Email Archive")

# Loop over the message list, adding each thread of
# messages as a discussion with comments
for k in list(msgLists.keys())[7:8]:
    mlist = msgLists[k]
    subject = filterSubject(mlist[0]['Subject'])
    discid = createDiscussion(repoid, catid, subject, filterBody(mlist[0].get_payload()))
    for m in mlist[1:]:
        addDiscussionComment(discid, filterBody(m.get_payload()))
    # lock this discussion
    lockLockable(discid)
