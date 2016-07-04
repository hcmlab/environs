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

if [[ -z "${TOOLSDIR}" ]]; then
    pushd `dirname $0` > /dev/null
    TOOLSDIR=`pwd`
    popd > /dev/null
fi

source "${TOOLSDIR}/check.build.is.ci.sh"

source "${TOOLSDIR}"/download.tools.sh

tDir=
dryrun=0

cd "${TOOLSDIR}/.."
[[ $? != 0 ]] && echo "Error" && exit 1

rootDir=$(pwd)
cd - >/dev/null 2>/dev/null

target="${TOOLSDIR}/../bin/Environs.Net.dll"

targetBin="$tmpDir/Environs-bin-${CI_BUILD_ID}"
[[ -z "${GITLAB_CI}" ]] && targetBin="$tmpDir/Environs-bin"

targetArch="${targetBin}.tar.gz"

if [[ -z "$cleanCmd" ]]; then
	cleanCmd=2
fi

if [[ "$cleanCmd" == "1" ]]; then
# [[ -d "$rootDir/bin" ]] && rm -rf "$rootDir/bin/*.+"
    return 0
fi
if [[ "$cleanCmd" == "2" ]]; then
	delFile "$targetArch"
	delDir "$targetBin"
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
echo 'Verifying Environs prebuilt binaries...'

prepareDir "${tmpDir}"
echo $target
if [[ -e "${targetArch}" ]] && [[ -e "$target" ]]; then
	echo "The binaries already exists .."
	echo "If you want to redownload, then delete [${targetArch}]"
else
	if [[ ! -e "${targetArch}" ]]; then
		echo -e
		echo "Downloading ${targetArch} ..."
		
		if [[ -z "${GITLAB_CI}" ]]; then
			curl -L0k -o "${targetArch}" "https://hcm-lab.de/git/environs/binaries/repository/archive.tar.gz?ref=master"
		else
			curl -L0k -o "${targetArch}" "https://hcm-lab.de/downloads/environs/bin.tar.gz"
		fi
	    [[ $? != 0 ]] && echo "Error downloading $1" && exit 1
	
		echo "Done."
		
		#[[ -d "${targetBin}" ]] && rm -rf ${targetBin}
		
		prepareDir "${targetBin}"
		
		echo "Unpack files ..."
		tar -xzf ${targetArch} -C ${targetBin} --strip-components=1
		[[ $? != 0 ]] && echo "Error $targetBin" && exit 1
	fi
	                      
	echo -e
	echo "Copying [$targetBin] to ${rootDir}..."
	
	[[ ! -e "$rootDir/bin" ]] && mkdir -p "$rootDir/bin"
	
	cp -rf "$targetBin"/bin/* "$rootDir"/bin
	[[ $? != 0 ]] && echo "Error $targetBin/bin" && exit 1

	[[ ! -e "$rootDir/bin64" ]] && mkdir -p "$rootDir/bin64"
	
	cp -rf "$targetBin"/bin64/* "$rootDir"/bin64
	[[ $? != 0 ]] && echo "Error $targetBin/bin64" && exit 1
	
	[[ ! -e "$rootDir/libs" ]] && mkdir -p "$rootDir/libs"
	
	cp -rf "$targetBin"/libs/* "$rootDir"/libs
	[[ $? != 0 ]] && echo "Error $targetBin/libs" && exit 1
	
	[[ ! -e "$rootDir/Mediator" ]] && mkdir -p "$rootDir/Mediator"
	
	cp -rf "$targetBin"/Mediator/* "$rootDir"/Mediator
	[[ $? != 0 ]] && echo "Error $targetBin/Mediator" && exit 1

	cd "$rootDir"/bin
	../Tools/0_prepare_windows_runtime.sh

	cd "$rootDir"/bin64
	../Tools/0_prepare_windows_runtime.sh
	
	[[ ! -z "${GITLAB_CI}" ]] && rm -rf "${targetArch}"
	
	echo "Done."
fi

