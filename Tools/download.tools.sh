#!/bin/bash
######################################################
#
# Download 3rd party headers from origins
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

if [[ -z "$tmpDir" ]]; then
#    curDir=${PWD##*/}
    tmpDir="$TOOLSDIR/../../tmpEnvirons"
fi

rescueDir="$TOOLSDIR/../../EnvironsRescued"

source "${TOOLSDIR}"/platform.detect.sh

[[ $# > 0 ]] && cleanCmd=$1

function safeMove
{
	smSrc="$1"
	smDst="$2"
	smName="$3"
	
    if [[ -e "${smSrc}/${smName}" ]]; then
    	[[ -e "${smDst}/${smName}" ]] && rm -rf "${smDst}/${smName}"
    	
    	[[ ! -e "${smDst}" ]] && mkdir -p "${smDst}" && [[ $? != 0 ]] &&  echo "Error mkdir ${smDst}" && exit 1
    	
    	mv "${smSrc}/${smName}" "${smDst}"/.
    fi
    [[ $? != 0 ]] && echo "Error moving ${smSrc}/${smName} to ${smDst}" && exit 1
}

function delFile
{	
	if [[ -z "$1" ]]; then
		return 0
	fi
	[[ -f "$1" ]] && rm -rf "$1"
}

function delDir
{	
	if [[ -z "$1" ]]; then
		return 0
	fi
	[[ -d "$1" ]] && rm -rf "$1"
}

function delFiles
{	
	declare -a toDels=("${!1}")
	
    for item in "${toDels[@]}"; do
		if [[ -e "$item" ]]; then
			rm -rf "$item"
		fi
    done
    return 0
}

function prepareDir
{	
	if [[ -z "$1" ]]; then
		return 0
	fi
	if [[ ! -d "$1" ]]; then
		[[ -z "${CI}" ]] && echo -e		
		[[ -z "${CI}" ]] && echo "Preparing directory [$1]"
		mkdir -p $1 && [[ $? != 0 ]] && echo "Error: Creating directory $1" && exit 1
	fi
	return 0
}
