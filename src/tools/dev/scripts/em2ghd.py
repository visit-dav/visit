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


# Test getting repo ids
print(GetRepoID("visit"))
print(GetRepoID("dashboard"))
print(GetRepoID("visit"))

print(GetDiscCategoryID("visit","Share cool stuff"))
print(GetDiscCategoryID("visit","Share cool stuff"))
print(GetDiscCategoryID("visit","Help using VisIt"))
