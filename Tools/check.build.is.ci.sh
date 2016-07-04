#!/bin/bash

if [[ -z "${TOOLSDIR}" ]]; then
    pushd `dirname $0` > /dev/null
    TOOLSDIR=`pwd`
    popd > /dev/null
fi


function doCIBuild
{	
	[[ -z "${GITLAB_CI}" ]] && [[ -z "${CI}" ]] && echo "No CI environment." && return 1
	
	echo -n "CI environment found. "
	
	cd "${TOOLSDIR}"
	[[ $? != 0 ]] && echo "Error: cd ${TOOLSDIR}" && exit 1
	    
	msg=$(git log -1 --oneline)
	
	cd - >>./build.log
	[[ $? != 0 ]] && echo "Error: cd -" && exit 1
	
#	echo "Error: ${msg}"
	
	if [[ "${msg}" == *\[ci\]* ]]; then	
		echo "Build ..."
		[[ ! -z "${CIRETURN}" ]] && exit 0
		#return 1
	else
		echo "Skip build."
		[[ ! -z "${CIRETURN}" ]] && exit 1
		exit 0
	fi
	
	return 0
}


# Returns 1 for do the build
# 0 means stop. do not build
doCIBuild
#CICHECKRES=$?

#[[ "${CICHECKRES}" != "1" ]] && exit 1

#if [[ -z "${NOCICHECK}" ]]; then
#	exit 0
#fi
