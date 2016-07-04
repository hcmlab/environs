#!/bin/bash

[[ "$TOOLSINCL" != "" ]] && return 0
TOOLSINCL=1

if [[ -z "${TOOLSDIR}" ]]; then
	pushd `dirname $0` > /dev/null
	TOOLSDIR=`pwd`
	popd > /dev/null
fi

source "${TOOLSDIR}"/platform.detect.sh

function prepareLinux
{
	if [[ "${ISDEBIAN}" == "1" ]]; then
		#sudo apt-get install uuid-dev ncurses-dev curl openssl libssl-dev
		sudo apt-get install uuid-dev openssl libssl-dev
		#opencl-headers
		return 0
	elif [[ "${ISRASPBERYY}" == "1" ]]; then

		if [[ "$1" != "" ]]; then
			sudo apt-get --yes autoremove
			sudo apt-get --yes autoclean
			sudo apt-get --yes clean
	
			return 0
		fi

		GCOMP="g++-4.8"
		#echo "Searching for ${GCOMP} ..."
		#sudo apt-cache search ${GCOMP}

		#if [[ $? != 0 ]]; then
		#	GCOMP="g++-4.7"
		#	echo "Not found. Using ${GCOMP}" 
		#else
		#	echo "Found." 
		#fi
		
		echo "Installing uuid-dev ${GCOMP} ncurses-dev curl libcurl4-openssl-dev openssl libssl-dev ..."
		
		sudo apt-get install uuid-dev ${GCOMP} ncurses-dev curl libcurl4-openssl-dev openssl libssl-dev
		# opencl-headers 
		[[ $? != 0 ]] && echo "Error: installing dependencies." && exit 1		
	fi
	return 0
}

prepareLinux