#!/bin/bash
# Requred clang-format to be installed on machine brefore run!
# You can add folder to exclude with argument to clang-format
# To use it install Python3 and do 'pip install clang-format'

#folder=./
#exclude_folder=./libs
#find ${folder} -type f -path ${exclude_folder} -prune -regex '.*\.\(cpp\|hpp\|cc\|c\|h\)' -exec clang-format -style=file -i {} \;

folder_PettieMain=./main
find ${folder_PettieMain} -regex '.*\.\(cpp\|hpp\|cc\|c\|h\)' -exec clang-format -style=file -i {} \;
