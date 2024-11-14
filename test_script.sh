#!../lab3-src/shell
#
# This shell script prints some data 
# about the user
# and shows how to use environment variables
#
echo Hello ${USER}
echo Welcome to CS252
echo Your home directory is ${HOME}
echo And your current directory is `/bin/pwd`
echo Number arguments: ${#}
echo Arg0: ${0}
echo Arg1: ${1}
echo Arg2: ${2}
