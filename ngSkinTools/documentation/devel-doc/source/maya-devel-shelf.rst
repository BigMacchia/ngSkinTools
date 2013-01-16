

load debug plugin
------------------

from ngSkinTools.debug import reloadplugin
reloadplugin.reload(
    r"C:\Viktoro\projects\ngSkinTools\RelaxCommandPlugin\output\debug 2011 x64\ngSkinTools.mll",
    r"C:\Users\uiron\Documents\maya\2011-x64\plug-ins\ngSkinTools.mll"
    )


reload source
--------------

from ngSkinTools.debug import reloader
reload(reloader)
reloader.unload()
    
start gui in debug mode
-----------------------

from ngSkinTools.utils import Utils
Utils.DEBUG_MODE = True
from ngSkinTools.ui.mainwindow import MainWindow
MainWindow.open()