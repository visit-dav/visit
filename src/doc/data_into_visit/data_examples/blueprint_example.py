import sys
sys.path.append("/usr/gapps/conduit/software/ascent/0.7.1/toss_3_x86_64_ib/openmp/gnu/conduit-install/lib/python3.7/site-packages")

import conduit
import conduit.relay.io
import conduit.blueprint.mesh
import numpy

mesh = conduit.Node()

# Create the coordinate set.
mesh["coordsets/coords/type"] = "uniform"
mesh["coordsets/coords/dims/i"] = 3
mesh["coordsets/coords/dims/j"] = 3
mesh["coordsets/coords/origin/x"] = -10.0
mesh["coordsets/coords/origin/y"] = -10.0
mesh["coordsets/coords/spacing/dx"] = 10.0
mesh["coordsets/coords/spacing/dy"] = 10.0

# Add the topology.
mesh["topologies/topo/type"] = "uniform"
mesh["topologies/topo/coordset"] = "coords"

# Add a simple element-associated field.
mesh["fields/ele_example/association"] = "element"
mesh["fields/ele_example/topology"] = "topo"

edata = numpy.array([1, 2, 3, 4], dtype=numpy.float64)
mesh["fields/ele_example/values"] = edata

# Add a simple vertex-associated field.
mesh["fields/vert_example/association"] = "vertex"
mesh["fields/vert_example/topology"] = "topo"
vdata = numpy.array([1, 1, 1, 2, 2, 2, 3, 3, 3], dtype=numpy.float64)
mesh["fields/vert_example/values"] = vdata

# Verify that the mesh conforms to the specification.
verify_info = conduit.Node()
if not conduit.blueprint.mesh.verify(mesh, verify_info):
    print("Verify failed")
    print(verify_info)

print(mesh)

conduit.relay.io.blueprint.write_mesh(mesh, "blueprint_example", "json")
