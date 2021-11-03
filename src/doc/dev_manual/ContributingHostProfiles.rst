Contributing Host Profiles
==========================

Host profiles live in /src/resources/hosts/<compute center name>.

There are a couple of helper file and scripts in the 'hosts' directory that 
allow the centers and profiles to be added during installation of VisIt, 
or imported later via the gui.

* If you add or remove directories from 'hosts', please edit networks.dat accordingly.

* If you add or remove directories or files, please regenerate networks.json by running dump_dir_to_networks_json.py

* Note that if run on Windows, the generated networks.json file will have dos-style line endings. Convert it to Unix style before committing.


