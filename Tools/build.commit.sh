#!/bin/bash

[[ "$TOOLSINCL" != "" ]] && return 0
TOOLSINCL=1

[[ -d "c:/Windows" ]] && isWindows=1

if [[ -z "${TOOLSDIR}" ]]; then
	pushd `dirname $0` > /dev/null
	TOOLSDIR=`pwd`
	popd > /dev/null
fi

source "${TOOLSDIR}"/platform.detect.sh

function buildCommitH
{

    COMMITH="${TOOLSDIR}/../Common/Environs.Commit.h"
    ECOMMIT="${TOOLSDIR}/../Common/ORIG_HEAD"
    COMMIT="${TOOLSDIR}/../.git/ORIG_HEAD"
    
    if [[ -e "${COMMIT}" ]]; then
		#echo "Commit available ${COMMIT}"
	
		if [[ -e ${ECOMMIT} ]]; then
	    		diff --brief "${COMMIT}" "${ECOMMIT}"
	    		if [[ $? == 0 ]]; then
				echo "Commit hashs are identical"
				return 0
	    	fi
		fi
	
		echo "Commit hashs are different. Updating ..."
		cp "${COMMIT}" "${ECOMMIT}"	    
	
		echo -n "#define BUILD_COMMIT " >${COMMITH}
		cat "${TOOLSDIR}/../.git/ORIG_HEAD" >>${TOOLSDIR}/../Common/Environs.Commit.h
		return 0
    fi
    
    if [[ ! -e "${COMMITH}" ]]; then
		echo -n "#define BUILD_COMMIT " >${COMMITH}
		echo "UNKNOWN" >>${COMMITH}
    fi
    return 0
}

buildCommitH