'''
    All tests assembled into one test suite
'''
import logging
from ngSkinTools import log 
from ngSkinToolsTest.cutCopyPasteActionsTest import CopyPasteActionsTest
from ngSkinToolsTest.meshDataImportExportTest import MeshDataImportExportTest
log.LoggerFactory = log.SimpleLoggerFactory(logging.WARN)

from ngSkinToolsTest.influenceNameTransformsTest import InfluenceNameTransformsTest
from ngSkinToolsTest.actionsTest import ActionsTest, BaseActionTest
from ngSkinToolsTest.versionTest import VersionTest
from ngSkinToolsTest.versioncheckTest import VersionCheckTest
from ngSkinToolsTest.importExportTest import ImportExportTest,\
    VariousImportScenarios
from ngSkinToolsTest.versioncheckIntegrationTest import VersionCheckIntegrationTest
from ngSkinToolsTest.testInheritance import TestInheritance
from ngSkinToolsTest.loggingTest import DummyLoggingTest, LoggingTest
from ngSkinToolsTest.ui.updateCheckWindowTest import UpdateCheckThreadTest
from ngSkinToolsTest.ui.mainWindowTest import MainWindowTest
from ngSkinToolsTest.utilities.importInfluencesTest import ImportInfluencesTest
from ngSkinToolsTest.influenceNameFilterTest import InfluenceNameFilterTest
from ngSkinToolsTest.mllInterfaceTest import MllInterfaceTest


InfluenceNameTransformsTest
ActionsTest
BaseActionTest
VersionTest
VersionCheckTest
ImportExportTest
VariousImportScenarios
VersionCheckIntegrationTest
TestInheritance
DummyLoggingTest
LoggingTest
UpdateCheckThreadTest
MainWindowTest
ImportInfluencesTest
InfluenceNameFilterTest
MllInterfaceTest
CopyPasteActionsTest
MeshDataImportExportTest