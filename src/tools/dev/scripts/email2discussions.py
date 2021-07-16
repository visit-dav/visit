import datetime, glob, mailbox, os, pytz, re, sys

# directory containing all the .txt files from the email archive
rootDir = "/Users/miller86/visit/visit-users-email"

# Iterate over all files loading each as a mailbox and catenating the
# items together into one long list here
files = glob.glob("%s/*.txt"%rootDir)
#files = ["/Users/miller86/visit/visit-users-email/2013-February.txt"]
items = []
for f in files:
    mb = mailbox.mbox(f)
    items += mb.items()

# Remove funky chars in subject and text between them to use in re key
# Handle multi-line subjects.
# Handle "No subject" subjects....all map to the same discussion
#   - get unique words, all lower case, filter out stop words if > 50% in common...then part of same discussion
# Handle quoted previous email
# Handle MIME attachments
# Keep subject and date

#
# Reorganize single list of emails, above, as a dict of lists
# (threads) key'd by Message-ID
#
msgLists = {}
for i in items:

    msg = i[1]

    # Try to put in an existing thread via References
    rids = msg['References']
    if rids:
        rids = rids.split()
        rids.reverse()

        foundIt = False;
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

#
# Handle (skip) all messages that had no id
#
if None in msgLists.keys():
    print("There are", len(msgLists[None]), "unclassified messages")
    del msgLists[None]

#for k in msgLists.keys():
#    mlist = msgLists[k]
#    print("List for ID=\"%s\"\n"%k)
#    for m in mlist:
#        su = " ".join(m['Subject'].split())
#        print(",",su)
#    print("\n")

# Smarter datetime that culls time-zone name if present and
# normalizes all datetimes to pacific (current) time zone
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
# Sort each list in msgLists by date
#
for k in msgLists.keys():
    mlist = msgLists[k]
    msgLists[k] = sorted(mlist, key=lambda m: mydt(m['Date']))

#
# Test: Write each message list to a file
#
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
