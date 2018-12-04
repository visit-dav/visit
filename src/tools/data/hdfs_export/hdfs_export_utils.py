import os
import vtk

#
# Represent a zero-origin index using a base-64 ascii digits string.
# Digit chars LSC to MSC are 0-9a-zA-Z%# for 64 (6 bits) digit chars.
# Using 6-bit digits chars (base-64) in this way means our ascii
# representation for an integer index is 25% inefficient because
# we're using 8-bit ascii chars to represent 6-bit digits of an integer
#
# We divide an entity's 'id' into 6 fields of some number of characters
# If a field has N characters, the total number of possible members of
# that class is 64^N. So, with this KeyConfig, we support 64^2=4096 users,
# each user having 64^3=262,144 databases, etc. In addition, we encode
# node, edge, face and volume elements as 8n+{0,1,2,3} respectively for
# non-ghost elements and 8n+{4,5,6,7} for ghost elements. So, given
# an entity key that after dividing by 8 has a remainder of 0, we know
# that entity is a node where as if the remainder is 4 we know its a
# ghost node or if the remainder is 3, we know is a 3D (volume) element.
#
KeyConfig = {
    "max_users":  2,
    "max_dbs":    3,
    "max_states": 2,
    "max_meshes": 1,
    "max_blocks": 4,
    "max_ents":   6,
}
KeyDigits = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ%#"
outClasses =   [               "users",               "dbs",                "states",                "meshes",                "blocks"]
outKeyDigits = [KeyConfig["max_users"], KeyConfig["max_dbs"], KeyConfig["max_states"], KeyConfig["max_meshes"], KeyConfig["max_blocks"]]
KeyTable = [-1 for i in range(256)]
for i in range(len(KeyDigits)):
    KeyTable[ord(KeyDigits[i])] = i

def IndexToAsciiKey(keybase, idx, max_chars):
    negate = False
    if idx < 0:
        negate = True
        idx = -idx
    nchars = 0 
    result = ""
    while True:
        if (idx < 64):
            result = KeyDigits[idx] + result
            nchars += 1
            while nchars < max_chars:
                result = '0' + result
                nchars += 1
            break
        s = idx % 64
        result = KeyDigits[s] + result
        nchars += 1
        if nchars == max_chars:
            return None
        idx = idx / 64
    if negate:
        result = '-' + result
    return keybase + result

def AsciiKeyToIndex(key, keybase=None):
    if key is None:
        return 0
    kb = 0
    if keybase:
        kb = AsciiKeyToIndex(keybase)
    addone = 0;
    if key[0] == '-':
        addone = 1
    result = 0
    mult = pow(64,len(key)-1-addone)
    for i in range(len(key)-addone):
        result = result + mult * KeyDigits.index(key[i+addone])
        mult = mult / 64
    if addone:
        result = -result
    return result-kb

#
# Compare two keys without resorting to computing their
# Python integer equivs because this is faster
#
def AsciiKeyCmp(keyA, keyB):
    keyA = keyA.lstrip('0')
    keyB = keyB.lstrip('0')
    lenA = len(keyA)
    lenB = len(keyB)
    if lenA != lenB:
        if lenA < lenB:
            return -1
        else:
            return 1
    for i in range(lenA):
        if keyA[i] != keyB[i]:
            diff = KeyTable[ord(keyA[i])] - KeyTable[ord(keyB[i])]
            if diff > 0:
                return 1
            else:
                return -1
    return 0

# The word 'zone' or 'cell' is ambiguous as it can mean
# a point in a 0D mesh, an edge in a 1D mesh, a (polygonal)
# face in a 2D mesh or a volume in a 3D mesh. Combined
# with knowledge of the mesh dimension, 'zone' or 'cell'
# are suitable aliases for one of node, edge, face or
# volume

# All 0D node key ids are key%8 == 0 (non-ghost), 4 (ghost)
def D0EntityIDKey(keybase,i,ghost=False):
    return IndexToAsciiKey(keybase,i*8+0+4*ghost,KeyConfig["max_ents"])

# All 1D key ids are key%8 == 1 (non-ghost), 5 (ghost)
def D1EntityIDKey(keybase,i,ghost=False):
    return IndexToAsciiKey(keybase,i*8+1+4*ghost,KeyConfig["max_ents"])

# All 2D key ids are key%8 == 2 (non-ghost), 6 (ghost)
def D2EntityIDKey(keybase,i,ghost=False):
    return IndexToAsciiKey(keybase,i*8+2+4*ghost,KeyConfig["max_ents"])

# All 3D key ids are key%8 == 3 (non-ghost), 7 (ghost)
def D3EntityIDKey(keybase,i,ghost=False):
    return IndexToAsciiKey(keybase,i*8+3+4*ghost,KeyConfig["max_ents"])

def EntityIDKey(dim,keybase,i,ghost=False):
    assert(0 <= dim and dim <= 3)
    if dim == 0:
        return D0EntityIDKey(keybase,i,ghost)
    elif dim == 1:
        return D1EntityIDKey(keybase,i,ghost)
    elif dim == 2:
        return D2EntityIDKey(keybase,i,ghost)
    elif dim == 3:
        return D3EntityIDKey(keybase,i,ghost)
    else:
        return D0EntityIDKey(keybase,i,ghost)

#  VTK_EMPTY_CELL = 0, VTK_VERTEX = 1, VTK_POLY_VERTEX = 2,VTK_LINE = 3, 
#  VTK_POLY_LINE = 4, VTK_TRIANGLE = 5, VTK_TRIANGLE_STRIP = 6, VTK_POLYGON = 7, 
#  VTK_PIXEL = 8, VTK_QUAD = 9, VTK_TETRA = 10, VTK_VOXEL = 11, 
#  VTK_HEXAHEDRON = 12, VTK_WEDGE = 13, VTK_PYRAMID = 14,
def EntityTypeIDKey(type,keybase,i,ghost=False):
    assert(vtk.VTK_EMPTY_CELL <= type and type <= vtk.VTK_PYRAMID)
    dim = {
        vtk.VTK_EMPTY_CELL:0,
        vtk.VTK_VERTEX:0,
        vtk.VTK_POLY_VERTEX:0,
        vtk.VTK_LINE:1,
        vtk.VTK_POLY_LINE:1,
        vtk.VTK_TRIANGLE:2,
        vtk.VTK_TRIANGLE_STRIP:2,
        vtk.VTK_POLYGON:2,
        vtk.VTK_PIXEL:0,
        vtk.VTK_QUAD:2,
        vtk.VTK_TETRA:3,
        vtk.VTK_VOXEL:3,
        vtk.VTK_HEXAHEDRON:3,
        vtk.VTK_WEDGE:3,
        vtk.VTK_PYRAMID:3
    }.get(type, 0)  
    return EntityIDKey(dim,keybase,i,ghost)

def NodeIDKey(keybase,i,ghost=False):
    return D0EntityIDKey(keybase,i,ghost)

#
# Create (when necessary) output directory and update
# keybase and outdir
#
def descendOutDir(outdir, keybase, name, data=None):
    depth = len(outdir)
    oldpath = '/'.join(outdir)
    outdir.append(name)
    newpath = '/'.join(outdir)
    outclass = outClasses[depth-1]
    if depth == 0:
        if not os.path.exists(name):
            os.mkdir(name)
        return
    idx = 0
    foundit = False
    if os.path.exists("%s/%s.txt"%(oldpath,outclass)):
        with open("%s/%s.txt"%(oldpath,outclass),"r") as f:
            for line in f:
                if line.strip('\n').split(',')[1] == name:
                    foundit = True
                    break
                idx = idx + 1
        f.close()
    outdigits = outKeyDigits[depth-1]
    keybase.append(IndexToAsciiKey("", idx, outdigits))
    if not foundit:
        os.mkdir(newpath)
        with open("%s/%s.txt"%(oldpath,outclass),"a") as f:
            if data == None:
                f.write("%s,%s\n"%(''.join(keybase),name))
            else:
                f.write("%s,%s,%s\n"%(''.join(keybase),name,','.join(str(e) for e in data)))
        f.close()

#
# Pop both output directory keybase
#
def ascendOutDir(outdir,keybase):
    outdir.pop(len(outdir)-1)
    keybase.pop(len(keybase)-1)
