from conans import python_requires
from conans import ConanFile
import os

commonlib = python_requires("conan-commonlib/2.6.4@ifx/release")

class UniversalConan(ConanFile):
    name = "universal"
    version = commonlib.Utils.git_rev_package_version()
    url = "https://bitbucket.vih.infineon.com/scm/p2s_common/universal.git"
    description = ("This is the universal embedded package. It contains preprocessor definitions"
      " for communication from an embedded device.")
    scm = {
        'type': 'git',
        'url': 'ssh://git@bitbucket.vih.infineon.com:7999/p2s_common/universal.git',
        'revision': 'auto'
    }

    def package(self):
        self.copy("*.h", "include/universal", keep_path=True)

    def package_info(self):
        self.user_info.universal_rev = self.scm["revision"]
