##############################################################
# This script assumes the following is installed: Python3, git, 
# cmake, conan, make, g++, and the Qt libraries and all dependencies. 
# This script will create a virtual environment, install, and build the project.
# It assumes that the script is run from the scripts directory. It also assumes 
# that the conan profile has been created. If not, it will create one.
#
# It expects the database file to be located at 
# /home/awhelan/cplus/pw-manager/PasswordManager/db/passwords.db.
# 
# If you want to change the location of the database file, you can edit the config 
# file located at ~/.config/PasswordManager/PasswordManager.conf. The config file 
# is copied from the config directory in the project root to the user's home 
# directory during installation.
##############################################################

#!/bin/bash

set -e

python3 -m venv ../../venv
source ../../venv/bin/activate
pip install conan
pip install conan --upgrade 
if [ ! -f "$HOME/.conan2/profiles/default" ]; then
    conan profile detect
fi
conan install ../ -c tools.build:jobs=4 --output-folder=../build --build=missing
mkdir -p ~/.config/PasswordManager
cp ../config/PasswordManager.conf  ~/.config/PasswordManager/PasswordManager.conf
cd ../build
cmake ../ -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
make -j
cp ./PasswordManager ../bin/PasswordManager