#!/bin/bash
######################################################
#
# Download 3rd party stuff
#
# Author: Chi-Tai Dang, dang@hcm-lab.de
# Copyright (c) Chi-Tai Dang, University of Augsburg
#
######################################################

if [[ -z "${SCRIPTDIR}" ]]; then
    pushd `dirname $0` > /dev/null
    SCRIPTDIR=`pwd`
    popd > /dev/null
fi

targetDir="$SCRIPTDIR/inc"
curDir=${PWD##*/}
tmpDir="$SCRIPTDIR/../../tmpEnvirons"
tDir=
dryrun=0
cleanCmd=0

[[ $# > 0 ]] && cleanCmd="$1"

if [[ "$cleanCmd" == *h* ]]; then
	sname=$(basename $0)
	echo -e
	echo "Usage: $sname cmd"
	echo -e
	echo "  where cmd 0: download all"
	echo "            1: delete installed files"
	echo "            2: delete downloaded temporary files"
	echo "            3: rescue temporary files to ../EnvironsRescue"
	echo "            4: restore temporary files from ../EnvironsRescue"
	echo -e
	echo "  e.g. $sname 2 (clean up temp files)"
	exit 0
fi

TOOLSDIR="${SCRIPTDIR}/../Tools"

source "${TOOLSDIR}/check.build.is.ci.sh"

if [[ -e "${TOOLSDIR}/../Android" ]]; then
    . "${TOOLSDIR}"/download.android.projects.sh $cleanCmd
fi

. "${TOOLSDIR}"/download.3rd.sh $cleanCmd

if [[ -e "${TOOLSDIR}/../Android" ]]; then
    . "${TOOLSDIR}"/download.binaries.sh $cleanCmd
fi

if [[ -e "${TOOLSDIR}/../Android" ]]; then
    . "${TOOLSDIR}"/download.opencv.sh $cleanCmd
fi
