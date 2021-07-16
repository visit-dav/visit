import datetime, glob, mailbox, os, pytz, re, sys

rootDir = "/Users/miller86/visit/visit-users-email"

#>>> mb = mailbox.mbox('../../visit-users-email/2013-February.txt')
#>>> mb.items()
#msg = email.message_from_string(msgstr)
#print(type(msg))
#print(dir(msg))
#print(msg.is_multipart())
#print(msg.items(), msg.keys(), msg.values())

# >>> msg = mb.get_message(132)
# >>> datetime.datetime.strptime(msg['Date'],'%a, %d %b %Y %H:%M:%S %z')


files = glob.glob("%s/*.txt"%rootDir)
#files = ["/Users/miller86/visit/visit-users-email/2013-February.txt"]
items = []
for f in files:
#    print("Processing file \"%s\"\n"%f)
    mb = mailbox.mbox(f)
#    print("new items = ", len(mb.items()))
    items += mb.items()
#    print("#items = ", len(items))

#msg = items[132][1]

#print(msg.keys())
#print("From: ", msg['From'])
#print("Date: ", msg['Date'])
#print("Subject: ", msg['Subject'])
#print("In-Reply-To: ", msg['In-Reply-To'])

nltkStopList = ["i", "me", "my", "myself", "we", "our", "ours", "ourselves", "you", "your", "yours", "yourself", "yourselves", "he", "him", "his", "himself", "she", "her", "hers", "herself", "it", "its", "itself", "they", "them", "their", "theirs", "themselves", "what", "which", "who", "whom", "this", "that", "these", "those", "am", "is", "are", "was", "were", "be", "been", "being", "have", "has", "had", "having", "do", "does", "did", "doing", "a", "an", "the", "and", "but", "if", "or", "because", "as", "until", "while", "of", "at", "by", "for", "with", "about", "against", "between", "into", "through", "during", "before", "after", "above", "below", "to", "from", "up", "down", "in", "out", "on", "off", "over", "under", "again", "further", "then", "once", "here", "there", "when", "where", "why", "how", "all", "any", "both", "each", "few", "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same", "so", "than", "too", "very", "s", "t", "can", "will", "just", "don", "should", "now"]

# Remove funky chars in subject and text between them to use in re key
# Handle multi-line subjects.
# Handle "No subject" subjects....all map to the same discussion
#   - get unique words, all lower case, filter out stop words if > 50% in common...then part of same discussion
# Handle quoted previous email
# Handle MIME attachments
# Keep subject and date

# Using email header info, you can find cases of 'replies' or 'references' that match orig. message id
# Message-ID: <76FE9F3BFF0B1B4A9EFDA35095CBA32C0637C8EB@PRDEXMBX-07.the-lab.llnl.gov>
# In-Reply-To: <76FE9F3BFF0B1B4A9EFDA35095CBA32C0637C8EB@PRDEXMBX-07.the-lab.llnl.gov>
# References: <76FE9F3BFF0B1B4A9EFDA35095CBA32C0637C8EB@PRDEXMBX-07.the-lab.llnl.gov>
# Ignore emails that do not contain 'From', 'Subject' and 'Date'

# Required header keys for any valid message
reqHdrKeys = ['From', 'Date', 'Subject', 'Message-ID', 'In-Reply-To', 'References']

badItems = []
msgLists = {}

# Reorganize as a dict of lists key'd by Message-ID
for i in items:

    # Gather bad messages without req'd header keys
    if not [x for x in reqHdrKeys if x in i[1].keys()]:
        badItems += [i]
        continue

    msg = i[1]
    mid = msg['Message-ID']
    rtid = msg['In-Reply-To']
    rids = msg['References']

    # Try to put in an existing thread via References
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
    if rtid and rtid in msgLists.keys():
        msgLists[rtid] += [msg]
        continue

    # It looks like we can't put in an existing thread
    if mid not in msgLists.keys():
        msgLists[mid] = [msg]
    else:
        msgLists[mid] += [msg]

for k in msgLists.keys():
    mlist = msgLists[k]
    print("List for ID=\"%s\"\n"%k)
    for m in mlist:
        su = " ".join(m['Subject'].split())
        print(",",su)
    print("\n")

# Smarter datetime that culls time-zone name if present
tzNames = [ \
    'GMT', 'UTC', 'ECT', 'EET', 'ART', 'EAT', 'MET', 'NET', 'PLT', 'IST', 'BST', 'VST',
    'CTT', 'JST', 'ACT', 'AET', 'SST', 'NST', 'MIT', 'HST', 'AST', 'PST', 'PNT', 'MST',
    'CST', 'EST', 'IET', 'PRT', 'CNT', 'AGT', 'BET', 'CAT', 'CET', 'PDT', 'EDT', 'CES',
    'KST', 'MSK', 'EES']
def mydt(d):
    tzn = d.split()[-1][1:4]
    if tzn in tzNames:
        d = " ".join(d.split()[:-1])
    try: 
        return datetime.datetime.strptime(d,'%a, %d %b %Y %H:%M:%S %z').astimezone()
    except:
        try:
            return datetime.datetime.strptime(d,'%a, %d %b %Y %H:%M %z').astimezone()
        except:
            return datetime.datetime.strptime(d,'%a, %d %b %Y %H:%M:%S').astimezone()

# Sort each list in msgLists by date
for k in msgLists.keys():
    mlist = msgLists[k]
    msgLists[k] = sorted(mlist, key=lambda m: mydt(m['Date']))
#    msgLists[k] = sorted(mlist, key=lambda m: datetime.datetime.strptime(m['Date'],'%a, %d %b %Y %H:%M:%S %z %Z'))
        
# Write each message list to a file
for k in msgLists.keys():
    mlist = msgLists[k]
    with open("tmp/%s"%k, 'w') as f:
        for m in mlist:
            f.write("From: %s\n"%m['From'])
            f.write("Gorfo-Date: %s\n"%mydt(m['Date']).strftime('%a, %d %b %Y %H:%M:%S %z'))
            f.write("Subject: %s\n"%m['Subject'])
            f.write("Message-ID: %s\n"%m['Message-ID'])
            f.write("%s\n"%m.get_payload())
        
#a = msg['Subject'].replace("[visit-users] ","").replace(" (Chris Malone)","")
#print(a)

#subItems = []
#counts = {'From': 0, 'Date':0, 'Subject':0, 'Message-ID':0}
#for i in items:
#    for k in ['From', 'Date', 'Subject', 'Message-ID']: 
#        if k in i[1].keys():
#            counts[k] += 1
#    if 'Subject' in i[1].keys():
#        if re.search(a, i[1]['Subject'], re.IGNORECASE):
#            subItems += [i]

#print(len(items), counts)

#
# Sort matching items by time to find *first* to use as discussion creation.
# The rest will be comments added to the discussion
#
#print(subItems)
#for i in sorted(subItems, key=lambda item: datetime.datetime.strptime(item[1]['Date'],'%a, %d %b %Y %H:%M:%S %z')):
#    print("Date: ", i[1]['Date'])
#    print("Subject: ", i[1]['Subject'])
#    print(i[1].get_payload())
