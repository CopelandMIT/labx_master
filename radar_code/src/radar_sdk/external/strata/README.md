# STRATA Host Library

Building locally...
-------------------

... using git-submodules without Conan (simpler)
------------------------------------------------

Check out the repository and don't forget to update the submodules via `git submodule update --init --recursive`. This is done automatically by most graphical git clients.

Open the CMakeLists.txt in the project root with Qt Creator (it should automatically detect the right settings and generate the project)
When building, please have a look at Projects - Build - CMake, where you have several options:
    STRATA_BUILD_SAMPLES
    STRATA_BUILD_TOOLS
    STRATA_BUILD_TESTS
    STRATA_BUILD_WRAPPERS
    STRATA_SHARED_LIBRARY

Please make sure to set them according to your needs.

... using Conan with git-submodules
--------------------------------------------------------

The git-submodules needs already be checked out before using the conan command. Used for CTRX and Mira development.

Example instructions for building:

    git submodule update --init --recursive
    mkdir build
    cd build
    conan install -g virtualenv -u ..
    source activate.sh
    conan build ..
    cmake --install .


Additional Info:
-at step "conan install -g virtualenv -u ..": add "-pr <profile containing Visual Studio Compiler> if not default, can be found in .conan folder in users/<username>   
-at step "source activate.sh": depending on used console it can be also activate.bat or  or other 
-Calling Tests: With ctest all included tests are executed
For directly calling the executable of the tests:
Copy the now generated libusb_strata.dll from /scripts/Python/strata/ into the folder where the tests are (e.g. ~/build/tests/integration_tests/platform)
Execute tests with cmd
    
