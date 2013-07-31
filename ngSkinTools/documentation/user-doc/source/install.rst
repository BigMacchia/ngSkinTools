Installation
============

ngSkinTools installs itself as module into Maya, therefore it's important that when using manual installation from archive packages, folder structure is preserved when extracting. With instructions below, ngSkinTools module file (through which Maya locates ngSkinTools plugin and it's other important files) is created in current user's module dir (``<user dir>/maya/<maya version>/modules``); if you want ngSkinTools installation to be available for all users, you should move that file to ``<maya-home-folder>/modules`` instead. 

Windows
--------

Executable installer
~~~~~~~~~~~~~~~~~~~~~

This is your ordinary automated install. Files are extracted into your specified directory, and module discovery file placed into ``<user dir>/maya/<maya version>/modules`` folder. If you're installing newer plugin version, uninstalling previous one is not required, but recommended.


Manual install
~~~~~~~~~~~~~~~

1. Extract archive somewhere you'll keep it, for example ``C:\my-maya-plugins\ngSkinTools``.
2. If you have Python installed, running *install.py* should do the rest of the work.
   Otherwise, manually copy file *ngSkinTools-module.txt* to ``<user documents>\maya\<maya version>\modules`` directory, where:
	
   * ``<user documents>`` is your windows user home folder (under WinXP that's ``C:\Documents And Settings\Username\Documents`` and in Win7 that's ``C:\Users\Username\My Documents``)
   * ``<maya version>`` is the folder for your Maya version. If you've downloaded plugin for 2011 64bit Maya, folder will be named *"2011 x64"*. This folder should already exist, as this is where Maya keeps your personalized settings, custom shelves, etc.

   Edit the *ngSkinTools-module.txt*, replacing ``C:\Path\to\ngSkinTools\base\dir`` with location where you extracted archive contents.

   .. note::	If ``modules`` directory does not exist, just go ahead and create it as well. It's not created by default by Maya, though added automatically into module search path.
	

3. (Re)start Maya.
4. New shelf should be available, *ngSkinTools*. Open ngSkinTools UI and, just in case, check the "About" window to verify that plugin version matches the one you've downloaded.


Linux
-------

1. Extract archive somewhere you'll keep it, for example ``/home/myself/maya-plugins/ngSkinTools/``.
2. Go to that directory.
3. Run "``install.py``" script. It should be extracted as executable, if not - run that from shell as "``python ./install.py``" or "``chmod u+x ./install.py;install.py``". This creates Maya module description file in ``<user home dir>/maya/<maya version>/modules``, through which Maya will be able to find plugin files.
4. (Re)start Maya.
5. New shelf should be available, *ngSkinTools*. Open ngSkinTools UI and, just in case, check the "About" window to verify that plugin version matches the one you've downloaded.

If anything goes wrong with *install.py*, or you just want to install manually, instead of step 3, manually copy file *ngSkinTools-module.txt* to ``<user home dir>/maya/<maya version>/modules`` directory, where:
	* ``<maya version>`` is the directory for your Maya version. If you've downloaded plugin for 2011 64bit Maya, folder will be named *"2011 x64"*. This directory should already exist, as this is where Maya keeps your personalized settings, custom shelves, etc.
	* ``modules`` subdirectory might not exist and might have to be created manually.
	* Edit the *ngSkinTools-module.txt*, replacing ``/path/to/ngskintools/base/dir`` with location where you extracted archive contents.

Mac
------

Instructions will be available as soon as install packages are!:]
