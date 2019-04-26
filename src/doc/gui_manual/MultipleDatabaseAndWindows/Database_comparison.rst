.. _Database comparison:

Database comparison
-------------------

Comparing the results of multiple related simulation databases is one of
VisIt_'s main uses. You can put plots from multiple databases in the same
window or you can put plots from each database in adjacent visualization
windows, allowing you to compare plots visually. In addition to these
visual comparison modes, VisIt_ supports more direct comparison of databases
using database comparisons. Database comparison allows you to plot direct
differences between two databases or between different time states in the
same database (i.e. time derivatives). VisIt_'s expression language is used
extensively to facilitate the database comparison.

The role of expressions
~~~~~~~~~~~~~~~~~~~~~~~

Database comparison is accomplished by creating new expressions that
involve the contents of multiple databases using VisIt_'s expression
language. Database comparisons use special expressions called
:ref:`Cross-Mesh Field Evaluation (CMFE) <Comparison_Expressions>`
expressions, :ref:`pos_cmfe() <Pos_Cmfe_Expression_Function>` and
:ref:`conn_cmfe() <Conn_Cmfe_Expression_Function>`,
which are capable of mapping a field from one mesh to another mesh
The name "conn_cmfe" means
*connectivity-based cross mesh field evaluation* (CMFE) and as the name
implies, the expression takes fields from one mesh and maps the field onto
another mesh by taking the cell or node-centered values on the donor mesh
and mapping them onto the cells or nodes having the same indices in the
new mesh. For valid results, conn_cmfe requires the donor and target meshes
be topologically congruent (e.g. size, connectivity, decomposition, etc.).
If this is not the case, then one should use the position-based, pos_cmfe
expression. The CMFE expressions can be used to map fields from a mesh in
one database onto a mesh in the active database, which then allows you to
create difference expressions involving the active database.

There is also a position-based CMFE (*pos_cmfe*), which will resample the field
from one mesh onto another mesh by calculating the values of the field on
the first mesh using the locations of the cells or nodes in a second mesh.
More information on CMFE expressions are found in the
:ref:`Cross-Mesh Field Evaluation (CMFE) <Comparison_Expressions>` section
of the :ref:`Exprssions <Expressions>` chapter.

Note that there is also a helpful *wizard*, the
:ref:`Data Level Comparison Wizard <DataLevelComparisonsWizard>`, that
simplifies the process of defining comparison expressions. Here, we
describe the :ref:`CMFE <Comparison_Expressions>` expressions and
demonstrate how to use them in comparisons.

Plotting the difference between two databases
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Simulations are often run as parts of parameter studies, meaning the
initial conditions are changed slightly to see what observable effects
the changes produce. The ability to do direct numerical comparison of
multiple simulation databases is required in order to observe the often
miniscule differences between the fields in the two databases. VisIt_
provides the conn_cmfe expression, which allows you to map a field from
one simulation database onto a mesh from another simulation database.
Once the mapping has been done, you can then perform difference operations
using an expression like this:

<mesh/ireg> - conn_cmfe(</usr/local/visit/data/dbB00.pdb:mesh/ireg>, mesh)

The expression above is a simple difference operation of database A minus
database B. The assumption made by this expression is that database A is
the active database and we're trying to map database B onto it so we can
subtract it from database A's *mesh/ireg* variable. Note that the conn_cmfe
expression takes two arguments. The first argument encodes the name of
the file and the field that we're mapping onto a mesh from the active
database, where the mesh name is given by the second argument. In this
example, we're mapping database B's *mesh/ireg* field onto database A's
mesh. :numref:`Figure %s <a_minus_b>` shows a picture that illustrates
the database differencing operation.

.. _a_minus_b:

.. figure:: images/a_minus_b.png

   Database B subtracted from database A

Plotting values relative to the first time state
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Plotting a variable relative to its initial values can be important for
understanding how the variable has changed over time. The conn_cmfe
expression is also used to plot values from one time state relative to
the values at the current time state. Consider the following expression:

<mesh/ireg> - conn_cmfe(</usr/local/visit/data/dbA00.pdb[0]i:mesh/ireg>, mesh)

The above expression subtracts the value of *mesh/ireg* at time state zero
from the value of *mesh/ireg* at the current time state. The interesting
feature about the above expression is its use of the expression language's
*[]* operator for specifying a database time state. The expression uses
*"[0]i"* , which means use time state zero because the *"i"* suffix
indicates that the number inside of the square brackets is to be used as
an absolute time state index. As you change the time slider, the values
for the current time state will change but the part of the expression
using conn_cmfe, which in this case uses the first database time state,
will not change. This allows you to create expressions that compare the
current time state to a fixed time state. You can, of course, substitute
different time state indices into the conn_cmfe expression so you don't
have to always compare the current time state to time state zero.

.. _a_minus_a0:

.. figure:: images/a_minus_a0.png

   Time state 6 minus time state 0

Plotting time derivatives
~~~~~~~~~~~~~~~~~~~~~~~~~

Plotting time derivatives is much like plotting the difference between the
current time state and a fixed time state except that instead of being
fixed, the second time state being compared is free to move relative to
the current time state. To plot a simple time derivative such as the
current time state minus the last time state, create an expression similar
to the following expression:

<mesh/ireg> - conn_cmfe(</usr/local/visit/data/dbA00.pdb[-1]id:mesh/ireg>, mesh)

The important piece of the above expression is its use of *"[-1]id"* to
specify a time state delta of -1, which means add -1 to the current time
state to get the time state whose data will be used in the conn_cmfe
calculation. You could provide different values for the time state in the
*[]* operator. Substituting a value of *3*, for example, would make the
conn_cmfe expression consider the data for 3 time states beyond the current
time state. If you use a time state delta, which always uses the *"d"*
suffix, the time state being considered is always relative to the current
time state. This means that as you change time states for the active
database using the time slider, the plots that use the conn_cmfe expression
will update properly. :numref:`Figure %s <a_minus_preva>` shows an example
plot of a time derivative.

.. image:: images/a.png

.. _a_minus_preva:

.. figure:: images/a_minus_preva.png

   Plot of a variable and its time derivative plot
