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

if [[ -z "$tmpDir" ]]; then
#    curDir=${PWD##*/}
    tmpDir="$SCRIPTDIR/../../tmpEnvirons"
fi

cd "${SCRIPTDIR}/.."
[[ $? != 0 ]] && echo "Error" && exit 1

rootDir=$(pwd)
cd - >/dev/null 2>/dev/null

target="${SCRIPTDIR}/../Mediator/Mediator.x86.exe"

targetBin="$tmpDir/Environs-bin"
targetArch="${targetBin}.tar.gz"

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
# [[ -d "$rootDir/bin" ]] && rm -rf "$rootDir/bin/*.+"
    return 0
fi
if [[ "$cleanCmd" == "2" ]]; then
    [[ -f "$targetArch" ]] && rm -rf "$targetArch"
    [[ -d "$targetBin" ]] && rm -rf "$targetBin"
    return 0
fi

if [[ "$cleanCmd" == "3" ]]; then
#    safeMove "$rootDir"  "${rescueDir}" ""
#    safeMove "$rootDir"  "${rescueDir}" "aFileDialog"

    return 0
fi
if [[ "$cleanCmd" == "4" ]]; then
    [[ ! -d "${rescueDir}" ]] && return 0
	
#    safeMove "$rescueDir"  "${targetDir}" "ActionBarSherlock"
#    safeMove "$rescueDir"  "${targetDir}" "aFileDialog"

    return 0
fi


echo -e
echo 'Downloading Environs prebuilt binaries...'

echo -e
echo "Preparing tmp dir [$tmpDir]"
[[ ! -d "${tmpDir}" ]] && mkdir $tmpDir && [[ $? != 0 ]] && echo "Error" && exit 1


cd ${tmpDir}
[[ $? != 0 ]] && echo "Error $targetBin" && exit 1

if [[ -e "${targetArch}" ]] && [[ -e "$target" ]]; then
	echo "The binaries already exists .."
	echo "If you want to redownload, then delete [${targetArch}]"
else
	if [[ ! -e "${targetArch}" ]]; then
		echo -e
		echo "Downloading ${targetArch} ..."
		
	    curl -L0k -o "${targetArch}" "https://hcm-lab.de/git/environs/binaries/repository/archive.tar.gz"
	    [[ $? != 0 ]] && echo "Error downloading $1" && exit 1
	fi
	
	[[ -d "${targetBin}" ]] && rm -rf ${targetBin}
	
	mkdir -p ${targetBin}
	[[ $? != 0 ]] && echo "Error $targetBin" && exit 1
	
	echo "Unpack files ..."
	tar -xzf ${targetArch} -C ${targetBin} --strip-components=1
	[[ $? != 0 ]] && echo "Error $targetBin" && exit 1
	                      
	echo -e
	echo "Copying [$targetBin] to ${rootDir}..."
	
	cp -rf "$targetBin"/bin/* "$rootDir"/bin
	[[ $? != 0 ]] && echo "Error $targetBin" && exit 1

	cp -rf "$targetBin"/bin64/* "$rootDir"/bin64
	[[ $? != 0 ]] && echo "Error $targetBin" && exit 1
	
	cp -rf "$targetBin"/libs/* "$rootDir"/libs
	[[ $? != 0 ]] && echo "Error $targetBin" && exit 1

	echo "Done."
fi

