.. _command_macros:

Macros
------

VisIt's **Command** window contains controls that allow you to record most
GUI actions and view Python scripting code needed to accomplish those actions.
The **Command** window provides 8 conventional tabs that serve as
destinations for recorded Python coding. In addition to those 8 tabs, there is
a special tab called **Macros** that shows the contents of the ``visitrc``
file. If you record Python code to the **Macros** tab then that Python code
is turned into a function that can be called in response to a button click
from a button in the **Macros** window.

Recording a macro
~~~~~~~~~~~~~~~~~

Here are the steps involved in *recording* a macro.

#. Open the **Command** window and choose to **Store commands in Macros**.

   .. figure:: images/commandwindow.png   

      Command Window Macros Tab

#. Click the **Record** button
#. Perform any GUI actions that you want to record to a single button click.
#. Click the **Stop** button in the **Command** window.
#. Enter the name of a Python function in which to store your set of recorded commands.

   .. figure:: images/macrorecord1.png   

      Setting the Python funtion name

#. Enter the text for the macro button as it will appear in the **Macro** window.

   .. figure:: images/macrorecord2.png   

      Setting the Macro Button text

#. Now, the **Macros** tab will contain a function for your recorded commands and
   it will call the RegisterMacro function from the VisIt Python Interface to
   associate your Python function with the named  button. **Note:** Remembe that you
   can edit the recorded Python code to suit your needs. You can generalize the code
   so it can, for example, operate on the active database instead of a specific
   database. The state information that you need to generalize can often be
   returned by the GetGlobalAttributes(), GetWindowInformation(), or GetMetaData() functions.
#. Click the **Update macros** button to make VisIt update the buttons in the
   **Macros** window so it will contain your new button.

   .. figure:: images/macrorecord3.png   

      The final Macro Button that is produced

#. No further steps need to be taken to save your macro since the macro definitions in
   the **Macros** tab of the **Command** window will be automatically saved to your
   ``visitrc`` file.
#. Click the new button in the **Macros** window whenever you want to replay the
   recorded set of commands.

.. warning::

    Users should be aware that VisIt_'s macro recording feature records only those operations that use methods in VisIt_'s *python* interface.
    Any operations that would not ordinarily *require* VisIt_'s python methods to complete are not recorded.
    In particular, operations involving interaction with the file system such as changing the current working directory and creating a directory (or folder) are not recorded.
