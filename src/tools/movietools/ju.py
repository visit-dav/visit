import os

###############################################################################
# Function: 
#
# Purpose:    This function gets the pool information from the ju command.
#
# Programmer: Brad Whitlock
# Date:       Mon Jan 13 12:59:46 PDT 2003
#
###############################################################################

def GetPoolInformationHelper(ju_command):
    # Run the ju command and get its output.
    f = os.popen(ju_command)
    lines = f.readlines()
    f.close()
    # Construct a list of dictionaries with the information.
    ju = []
    for line in lines[1:]:
        pool = line[5:11]
        total = int(line[24:29])
        down = int(line[29:34])
        used = int(line[34:39])
        available = int(line[39:45])
        capacity = int(line[45:49])
        poolInfo = {"pool":pool,\
"total":total,\
"down":down,\
"used":used,\
"available":available,\
"capacity":capacity}
        ju = ju + [poolInfo]
    return ju

###############################################################################
# Function: GetRemotePoolInformation
#
# Purpose:    This function gets the pool information from the ju command on
#             a remote machine.
#
# Arguments:
#   host : The host that we want to know about.
#
# Programmer: Brad Whitlock
# Date:       Mon Jan 13 12:59:46 PDT 2003
#
###############################################################################

def GetRemotePoolInformation(host):
    return GetPoolInformationHelper("ssh %s ju" % host)

###############################################################################
# Function: GetPoolInformation
#
# Purpose:    This function gets the pool information from the ju command.
#
# Programmer: Brad Whitlock
# Date:       Mon Jan 13 12:59:46 PDT 2003
#
###############################################################################

def GetPoolInformation():
    return GetPoolInformationHelper("ju")

###############################################################################
# Function: ChoosePool
#
# Purpose:    This function chooses a pool and number of processors given
#             the current scheduling conditions.
#
# Arguments:
#   host              : The host that we want to use.
#   procsPerNode      : The number of processors per node on the host.
#   minProcessors     : The minimum number of processors that we'll take.
#   desiredProcessors : The ideal number of processors that we want.
#   timeLimit         : The time limit for out job.
#
# Return:
#   A tuple consisting of the following values:
#   (pool name, number of nodes, number of procs, time limit)
#
# Notes:
#   This function is coded for the frost machine. It doesn't take into
#   account the existence of other pools like the VIEWS nodes on machines
#   like white.
#
# Programmer: Brad Whitlock
# Date:       Mon Jan 13 12:59:46 PDT 2003
#
###############################################################################

def ChoosePool(host, procsPerNode, minProcessors, desiredProcessors, timeLimit):
    # Get up to date information on the available resources
    poolInfo = GetRemotePoolInformation(host)
    minNodes = minProcessors / procsPerNode
    desiredNodes = desiredProcessors / procsPerNode
    if(len(poolInfo) > 1):
        # This code assumes frost's configuration.
        pdebug = poolInfo[0]
        pbatch = poolInfo[1]    
        if(pbatch["available"] <= desiredNodes):
            # The batch pool has all the nodes we want.
            return ("pbatch", desiredNodes, desiredNodes * procsPerNode, timeLimit*2)
        elif(pbatch["available"] >= minNodes):
            # We have to settle for fewer nodes
            return ("pbatch", pbatch["available"], pbatch["available"] * procsPerNode, timeLimit)
        elif(pdebug["available"] > 0):
            # The batch pool has nothing but the debug pool is free.
            return ("pdebug", pdebug["available"], pdebug["available"] * procsPerNode, timeLimit)
        else:
            # No pools really have what we want. Scale back the number of nodes we want.
            nnodes = int(desiredNodes*0.75)
            return ("pbatch", nnodes, nnodes * procsPerNode, timeLimit*2)
    else:
        nnodes = (desiredProcessors - minProcessors) / 2 / procsPerNode
        if(nnodes == 0):
            nnodes = 1
        return ("pbatch", nnodes, nnodes * procsPerNode, timeLimit)

#
# Chooses the best pool to use on frost.
#
def ChoosePoolFrost(minProcessors, desiredProcessors, timeLimit):
    return ChoosePool("frost", 16, minProcessors, desiredProcessors, timeLimit)
