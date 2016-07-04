#!/bin/bash
######################################################
#
# Download 3rd party opencv headers
#
#
# Author: Chi-Tai Dang, dang@hcm-lab.de
# Copyright (c) Chi-Tai Dang, University of Augsburg
#
######################################################

if [[ -z "${TOOLSDIR}" ]]; then
    pushd `dirname $0` > /dev/null
    TOOLSDIR=`pwd`
    popd > /dev/null
else
    cd "${TOOLSDIR}"
    [[ $? != 0 ]] && echo "Error" && exit 1
    
    TOOLSDIR=$(pwd)
    cd - >/dev/null 2>/dev/null
fi

source "${TOOLSDIR}/check.build.is.ci.sh"

dryrun=0

VERSION="2.4.11"
SRC="opencv-${VERSION}.tar.gz"
DLINK="https://github.com/Itseez/opencv/archive/${VERSION}.tar.gz"

targetDir="${TOOLSDIR}/../3rd/inc"

source "${TOOLSDIR}"/download.tools.sh

target1="${targetDir}/opencv"
target2="${targetDir}/opencv2"
tmpFile="${tmpDir}/${SRC}"
cvSrc="${tmpDir}/opencv"

if [[ "$cleanCmd" == "1" ]]; then
	delDir "$targetDir/opencv"
	delDir "$targetDir/opencv2"
	return 0
fi
if [[ "$cleanCmd" == "2" ]]; then
	delFile "$tmpFile"
	delDir "$targetDir/opencv"
	return 0
fi
if [[ "$cleanCmd" == "3" ]]; then
    safeMove "$targetDir"  "${rescueDir}" "opencv"
    safeMove "$targetDir"  "${rescueDir}" "opencv2"
#    [[ -e "$tmpDir" ]] && mv "$tmpDir" "$rescueDir"/.
    return 0
fi
if [[ "$cleanCmd" == "4" ]]; then
	[[ ! -d "${rescueDir}" ]] && return 0	
    safeMove "$rescueDir"  "${targetDir}" "opencv"
    safeMove "$rescueDir"  "${targetDir}" "opencv2"
#	[[ -e "$rescueDir/tmp" ]] && mv "$rescueDir/tmp" "$tmpDir" 	
	return 0
fi


echo -e
echo 'Verifying opencv headers...'

prepareDir "${tmpDir}"

function copyFolder
{
    t=$1
    if [[ ! -d "${target1}/$t" ]]; then
        echo -e
        echo "Copying $t to inc ..."
        cp -R "${cvSrc}/modules/$t/include/opencv2/$t" "${target2}"/.
        [[ $? != 0 ]] && echo "Error $1" && exit 1
    fi
}

cd ${tmpDir}
[[ $? != 0 ]] && echo "Error ${tmpDir}" && exit 1

if [[ ! -e "${tmpFile}" ]]; then
    echo -e
    echo "Downloading ${target} ..."
	
    curl -L0k -o "${tmpFile}" "${DLINK}"
    [[ $? != 0 ]] && echo "Error downloading $1" && exit 1
fi

if [[ ! -e "${cvSrc}" ]]; then
	prepareDir "${cvSrc}"
	
	echo "Unpacking files ..."
	tar -xzf "${tmpFile}" -C ${cvSrc} --strip-components=1
	[[ $? != 0 ]] && echo "Error tar ${tmpFile}" && exit 1
fi

#[[ -d "${target1}" ]] && rm -rf ${target1}
#[[ -d "${target2}" ]] && rm -rf ${target2}
                      
if [[ ! -d "${target1}" ]]; then
    echo -e
    echo "Copying opencv to inc ..."
    cp -R "${cvSrc}/include/opencv" "${targetDir}"/.
    [[ $? != 0 ]] && echo "Error ${target1}" && exit 1
fi

if [[ ! -d "${target2}" ]]; then
    echo -e
    echo "Copying opencv2 to inc ..."
    cp -R "${cvSrc}/include/opencv2" "${targetDir}"/.
    [[ $? != 0 ]] && echo "Error ${target2}" && exit 1
fi

copyFolder "core"
copyFolder "calib3d"
copyFolder "features2d"
copyFolder "flann"
copyFolder "highgui"
copyFolder "imgproc"
copyFolder "legacy"
copyFolder "objdetect"
copyFolder "video"

echo "Done."




