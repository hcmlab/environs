#!/bin/bash
######################################################
#
# Download 3rd party android projects
#	e.g. ActionBarSherlock and aFileDialog
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

targetDir="${SCRIPTDIR}/../Androids/MediaBrowser/library"

if [[ -z "$tmpDir" ]]; then
#    curDir=${PWD##*/}
    tmpDir="$SCRIPTDIR/../../tmpEnvirons"
fi

rescueDir="$SCRIPTDIR/../../EnvironsRescued"
tDir=
dryrun=0


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


if [[ "$cleanCmd" == "1" ]]; then
	[[ -d "$targetDir/ActionBarSherlock" ]] && rm -rf "$targetDir/ActionBarSherlock"
	[[ -d "$targetDir/aFileDialog" ]] && rm -rf "$targetDir/aFileDialog"
	return 0
fi
if [[ "$cleanCmd" == "2" ]]; then
	[[ -f "$tmpDir/ActionBarSherlock.tar.gz" ]] && rm -rf "$tmpDir/ActionBarSherlock.tar.gz"
	[[ -d "$tmpDir/ActionBarSherlock" ]] && rm -rf "$tmpDir/ActionBarSherlock"
	[[ -f "$tmpDir/aFileDialog.tar.gz" ]] && rm -rf "$tmpDir/aFileDialog.tar.gz"
	[[ -d "$tmpDir/aFileDialog" ]] && rm -rf "$tmpDir/aFileDialog"
	return 0
fi

if [[ "$cleanCmd" == "3" ]]; then
    safeMove "$targetDir"  "${rescueDir}" "ActionBarSherlock"
    safeMove "$targetDir"  "${rescueDir}" "aFileDialog"

#	[[ -e "$tmpDir" ]] && mv "$tmpDir" "${rescueDir}"/.
	return 0
fi
if [[ "$cleanCmd" == "4" ]]; then
	[[ ! -d "${rescueDir}" ]] && return 0
	
    safeMove "$rescueDir"  "${targetDir}" "ActionBarSherlock"
    safeMove "$rescueDir"  "${targetDir}" "aFileDialog"

#	[[ -e "$rescueDir/tmp" ]] && mv "$rescueDir/tmp" "$tmpDir"
	return 0
fi


echo -e
echo 'Downloading additional Android projects...'

function patchGradle
{
	[[ -z "$1" ]] && echo "Error: A gradle.properties must be given as first argument" && return 1
	[[ -z "$2" ]] && echo "Error: A compileSdkVersion version must be given as second" && return 1
	[[ -z "$3" ]] && echo "Error: A buildToolsVersion must be given as third argument" && return 1
	
	local found=0
	local tmp="$1.h"
	
	echo -n "" >"${tmp}"
	
	IFS= 
	while read -r line; do
		if [[ "${line}" == *compileSdkVersion* ]]; then
			echo -e "\t\t compileSdkVersion $2" >>"${tmp}"
			found=1
			continue
		fi
		if [[ "${line}" == *buildToolsVersion* ]]; then
			echo -e "\t\t buildToolsVersion \"$3\"" >>"${tmp}"
			found=1
			continue
		fi
		echo "${line}" >>"${tmp}"
	done < "$1"
	
	[[ $? != 0 ]] && echo "Error" && return 1
	[[ $found != 1 ]] && echo "Error: compileSdkVersion/buildToolsVersion not found in [$1]." && return 1
	
	rm "$1"
	mv "${tmp}" "$1"
	echo "Patched gradle [$1]"	
}


echo -e
echo "Preparing tmp dir [$tmpDir]"
[[ ! -d "${tmpDir}" ]] && mkdir $tmpDir && [[ $? != 0 ]] && echo "Error" && exit 1

target="ActionBarSherlock"

cd ${tmpDir}
[[ $? != 0 ]] && echo "Error $target" && exit 1

if [[ -d "${targetDir}/${target}" ]]; then
	echo "The target already exists .."
	echo "[${targetDir}/${target}]"
else
	if [[ ! -e "${target}.tar.gz" ]]; then
		echo -e
		echo "Downloading ${target} 4.4.0 ..."
		
	    curl -L0k -o "${target}.tar.gz" "https://api.github.com/repos/JakeWharton/ActionBarSherlock/tarball/4.4.0"
	    [[ $? != 0 ]] && echo "Error downloading $1" && exit 1
	fi
	
	[[ -d "${target}" ]] && rm -rf ${target}
	
	mkdir -p ${target}
	[[ $? != 0 ]] && echo "Error $target" && exit 1
	
	echo "Unpack files ..."
	tar -xzf ${target}.tar.gz -C ${target} --strip-components=1
	[[ $? != 0 ]] && echo "Error $target" && exit 1
	                      
	echo -e
	echo "Moving [$target] to ${targetDir}..."
	[[ ! -d "${targetDir}" ]] && mkdir -p "${targetDir}" && [[ $? != 0 ]] && echo "Error $target" && exit 1
	
	mv ${target} "${targetDir}"/.
	[[ $? != 0 ]] && echo "Error $target" && exit 1
	
	echo "Done."
fi

patchGradle "${targetDir}/${target}/actionbarsherlock/build.gradle" "22" "22.0.1"
[[ $? != 0 ]] && echo "Error" && exit 1

target="aFileDialog"

cd ${tmpDir}
[[ $? != 0 ]] && echo "Error $target" && exit 1

if [[ -d "${targetDir}/${target}" ]]; then
	echo "The target already exists .."
	echo "[${targetDir}/${target}]"
else
	if [[ ! -e "${target}.tar.gz" ]]; then
		echo -e
		echo "Downloading ${target} ..."
		
	    curl -L0k -o "${target}.tar.gz" "https://github.com/jfmdev/aFileDialog/archive/v1.0.5.tar.gz"
	    [[ $? != 0 ]] && echo "Error downloading $1" && exit 1
	fi
	
	[[ -d "${target}" ]] && rm -rf ${target}
	
	mkdir -p ${target}
	[[ $? != 0 ]] && echo "Error $target" && exit 1
	
	echo "Unpack files ..."
	tar -xzf ${target}.tar.gz -C ${target} --strip-components=1
	[[ $? != 0 ]] && echo "Error $target" && exit 1
	                      
	echo -e
	echo "Moving [$target] to ${targetDir}..."
	[[ ! -d "${targetDir}" ]] && mkdir -p "${targetDir}" && [[ $? != 0 ]] && echo "Error $target" && exit 1

	mv ${target} "${targetDir}"/.
	[[ $? != 0 ]] && echo "Error $target" && exit 1
	
	echo "Done."
fi

patchGradle "${targetDir}/${target}/library/app/build.gradle" "22" "22.0.1"
[[ $? != 0 ]] && echo "Error $target" && exit 1



