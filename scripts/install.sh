#################################
#
#
#
# Requires: git, Python3. 
#################################

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
cd ../build
cmake ../ -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
make -j
cp ./PasswordManager ../bin
# cd ../
# rm -rf ./build
# rm -rf ../venv