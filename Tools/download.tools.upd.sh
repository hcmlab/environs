#!/bin/bash
######################################################
#
# Download gitlab ci tools
#
# Author: Chi-Tai Dang, dang@hcm-lab.de
# Copyright (c) Chi-Tai Dang, University of Augsburg
#
######################################################

if [[ -z "${TOOLSDIR}" ]]; then
    pushd `dirname $0` > /dev/null
    TOOLSDIR=`pwd`
    popd > /dev/null
fi

toolsDir="$TOOLSDIR/../ToolsTmp"

toolsUrl="https://hcm-lab.de/downloads/environs/EnvBuildTools.tar.gz"
[[ "$ENVIRONSTOOLSUSER" != "" ]] && toolsUrl="https://${ENVIRONSTOOLSUSER}:${ENVIRONSTOOLSPASS}@hcm-lab.de/downloads/environs/EnvBuildTools.tar.gz"

source "${TOOLSDIR}"/download.tools.sh

delDir "${toolsDir}"
prepareDir "${toolsDir}"

cd "${toolsDir}"
[[ $? != 0 ]] && echo "Error cd ${toolsDir}" && exit 1

curl -L0k -o "EnvBuildTools.tar.gz" "${toolsUrl}"
[[ $? != 0 ]] && echo "Error tools download" && exit 1
    
if [[ -z "${CI}" ]]; then
	tar -xzvf EnvBuildTools.tar.gz
else
	tar -xzf EnvBuildTools.tar.gz
fi
[[ $? != 0 ]] && echo "Error tools decompress" && exit 1

rm Tools/download.tools.upd.sh
rm -rf Tools/src

cp Tools/* ../Tools/.
[[ $? != 0 ]] && echo "Error cp" && exit 1

cd ..
[[ $? != 0 ]] && echo "Error cd" && exit 1

rm -rf "${toolsDir}"
[[ $? != 0 ]] && echo "Error rm ${toolsDir}" && exit 1

exit 0