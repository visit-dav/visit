# An example to get the remaining rate limit using the Github GraphQL API.

import requests

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
    request = requests.post('https://api.github.com/graphql', json={'query': query}, headers=headers)
    if request.status_code == 200:
        return request.json()
    else:
        raise Exception("Query failed to run by returning code of {}. {}".format(request.status_code, query))

        
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
    result = run_query(query)
    # {'data': {'createDiscussion': {'discussion': {'id': 'MDEwOkRpc2N1c3Npb24zNDY0NDI1'}}}}
    return result['data']['createDiscussion']['discussion']['id']

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
    result = run_query(query)
    # {'data': {'addDiscussionComment': {'comment': {'id': 'MDE3OkRpc2N1c3Npb25Db21tZW50MTAxNTM5Mw=='}}}}
    return result['data']['addDiscussionComment']['comment']['id']

# Test getting repo ids
repoid = GetRepoID("temporary-play-with-discussions")
catid =  GetDiscCategoryID("temporary-play-with-discussions", "Ideas")
discid = createDiscussion(repoid, catid, "Discussion with python script", "Hello world")
print(discid)
addDiscussionComment(discid, "Adding a comment")
addDiscussionComment(discid, "Adding another comment")
