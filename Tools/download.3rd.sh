#!/bin/bash
######################################################
#
# Download 3rd party headers from origins
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

if [[ "${SCRIPTDIR}" != *"3rd"* ]]; then
	SCRIPTDIR="${SCRIPTDIR}/../3rd"
fi

obsolete=0
targetDir="$SCRIPTDIR/inc"
targetLibDir="$SCRIPTDIR/lib"

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
    [[ -e "$targetDir/msints" ]] && rm -rf "$targetDir/msints"
    [[ -e "$targetDir/CL" ]] && rm -rf "$targetDir/CL"
    [[ -e "$targetDir/TUIO" ]] && rm -rf "$targetDir/TUIO"
    [[ -e "$targetLibDir/android/arm/libopenh264.so" ]] && rm -rf "$targetLibDir/android/arm/libopenh264.so"
    [[ -e "$targetLibDir/android/arm/openh264.LICENSE.txt" ]] && rm -rf "$targetLibDir/android/arm/openh264.LICENSE.txt"
    [[ -e "$targetLibDir/../../bin64/libs/openh264.LICENSE.txt" ]] && rm -rf "$targetLibDir/../../bin64/libs/openh264.LICENSE.txt"
    [[ -e "$targetLibDir/../../bin64/libs/libopenh264.dylib" ]] && rm -rf "$targetLibDir/../../bin64/libs/libopenh264.dylib"
    [[ -e "$targetLibDir/../../bin64/libs/libopenh264.dll" ]] && rm -rf "$targetLibDir/../../bin64/libs/libopenh264.dll"
    [[ -e "$targetLibDir/../../bin/libs/openh264.LICENSE.txt" ]] && rm -rf "$targetLibDir/../../bin/libs/openh264.LICENSE.txt"
    [[ -e "$targetLibDir/../../bin/libs/libopenh264.dll" ]] && rm -rf "$targetLibDir/../../bin/libs/libopenh264.dll"
    return 0
fi
if [[ "$cleanCmd" == "2" ]]; then
    [[ -e "$tmpDir/msints.zip" ]] && rm -rf "$tmpDir/msints.zip"
    [[ -e "$tmpDir/msints" ]] && rm -rf "$tmpDir/msints"
    [[ -e "$tmpDir/TUIO.zip" ]] && rm -rf "$tmpDir/TUIO.zip"
    [[ -e "$tmpDir/TUIO" ]] && rm -rf "$tmpDir/TUIO"
    [[ -e "$tmpDir/openh264" ]] && rm -rf "$tmpDir/openh264"    
    return 0
fi
if [[ "$cleanCmd" == "3" ]]; then    
    safeMove "$targetDir"  "${rescueDir}" "msints"
    safeMove "$targetDir"  "${rescueDir}" "CL"
    safeMove "$targetDir"  "${rescueDir}" "TUIO"
    safeMove "$targetDir"  "${rescueDir}" "openh264"
    
	pref=android/arm
    safeMove "$targetLibDir/${pref}"  "${rescueDir}/${pref}" "libopenh264.so"
    safeMove "$targetLibDir/${pref}"  "${rescueDir}/${pref}" "openh264.LICENSE.txt"
    
	pref=bin64/libs
	tpref=../../${pref}
    safeMove "$targetLibDir/${tpref}"  "${rescueDir}/${pref}" "openh264.LICENSE.txt"
    safeMove "$targetLibDir/${tpref}"  "${rescueDir}/${pref}" "libopenh264.dylib"
    safeMove "$targetLibDir/${tpref}"  "${rescueDir}/${pref}" "libopenh264.dll"
        
	pref=bin/libs
	tpref=../../${pref}
    safeMove "$targetLibDir/${tpref}"  "${rescueDir}/${pref}" "openh264.LICENSE.txt"
    safeMove "$targetLibDir/${tpref}"  "${rescueDir}/${pref}" "libopenh264.dll"
    
#    [[ -e "$tmpDir" ]] && mv "$tmpDir" "${rescueDir}"/.
    return 0
fi
if [[ "$cleanCmd" == "4" ]]; then
    [[ ! -d "${rescueDir}" ]] && return 0
    
    safeMove "${rescueDir}"  "${targetDir}" "msints"
    safeMove "${rescueDir}"  "${targetDir}" "CL"
    safeMove "${rescueDir}"  "${targetDir}" "TUIO"
    safeMove "${rescueDir}"  "${targetDir}" "openh264"

	pref=android/arm
    safeMove "$rescueDir/${pref}"  "${targetLibDir}/${pref}" "libopenh264.so"
    safeMove "$rescueDir/${pref}"  "${targetLibDir}/${pref}" "openh264.LICENSE.txt"

    
	pref=bin64/libs
	tpref=../../${pref}
    safeMove "$rescueDir/${pref}"  "${targetLibDir}/${tpref}" "openh264.LICENSE.txt"
    safeMove "$rescueDir/${pref}"  "${targetLibDir}/${tpref}" "libopenh264.dylib"
    safeMove "$rescueDir/${pref}"  "${targetLibDir}/${tpref}" "libopenh264.dll"
    
	pref=bin/libs
	tpref=../../${pref}
    safeMove "$rescueDir/${pref}"  "${targetLibDir}/${tpref}" "openh264.LICENSE.txt"
    safeMove "$rescueDir/${pref}"  "${targetLibDir}/${tpref}" "libopenh264.dll"
    
#    [[ -e "$rescueDir/tmp" ]] && mv "$rescueDir/tmp" "$tmpDir"
    return 0
fi


echo -e
echo 'Downloading 3rd party header files and sources...'

echo -e
echo "Preparing tmp dir [$tmpDir]"
#[[ "$dryrun" == "0" ]] && [[ -d "$tmpDir" ]] && rm -rf $tmpDir

[[ ! -d "${tmpDir}" ]] && mkdir $tmpDir

[[ ! -d "$targetDir" ]] && mkdir $targetDir

function copyFiles
{
    litem=$2
    item="$targetDir/$2"
#    echo $item
    [[ -d "$item" ]] && return 0

    [[ ! -d "$item" ]] && mkdir $item

    cp $tmpDir/$1/$2/*.h $item/.
    return 0
}

function dlKhronos
{
    if [[ ! -f "$tDir/$1" ]]; then
		curl -L -o "$tDir/$1" "https://www.khronos.org/registry/cl/api/2.0/$1"
		[[ $? != 0 ]] && echo "Error downloading $1" && exit 1
    fi
    return 0
}


cd $tmpDir

STAGE=openh264
tDir="$targetDir/${STAGE}"
echo -e
echo "Preparing ${STAGE} directory [$tDir]..."
echo '----------------------------------------'

[[ "$dryrun" == "0" ]] && [[ -d "$tDir" ]] && rm -rf "$tDir"

sDir="$tmpDir/${STAGE}"
#[[ "$dryrun" == "0" ]] && [[ -d "$sDir" ]] && rm -rf "$sDir"
[[ ! -d "$sDir" ]] && mkdir $sDir

echo "Download ${STAGE} binary license from openh264.org ..."
LICENSE="$sDir/${STAGE}.LICENSE.txt"

if [[ ! -f "$LICENSE" ]]; then
    echo "Download ${STAGE} binary license from openh264.org ..."
    
    curl -L -o "$LICENSE" "http://www.openh264.org/BINARY_LICENSE.txt"
    [[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
    
    echo "Done."

    cat $LICENSE
    echo 'Please take notice of the cisco openh264 binary license.'
    echo 'Press any key to continue ...'
    read response
fi


if [[ ! -e "$targetLibDir/android/arm/lib${STAGE}.so" ]]; then
	item="$sDir/lib${STAGE}.so.bz2"
	
	if [[ ! -f "$item" ]] && [[ ! -f "$sDir/lib${STAGE}.so" ]]; then
	    echo "Download ${STAGE} Android binary from cisco ..."
	    
	    curl -L -o "$item" "https://github.com/cisco/openh264/releases/download/v1.4.0/libopenh264-1.4.0-android19.so.bz2"
	    [[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	    
	    echo "Done."
	fi
	
	cd "$sDir"
	
	if [[ ! -f "$sDir/lib${STAGE}.so" ]]; then
		echo "Unpacking ${STAGE} Android binary ..."
		
		bzip2 -d lib${STAGE}.so.bz2
		[[ $? != 0  ]] && echo "Error" && exit 1
		echo "Done."
	fi


	tDir="$targetLibDir/android/arm"
	echo -e
	echo "Preparing ${STAGE} directory [$tDir]..."
	echo '----------------------------------------'
	[[ ! -d "$tDir" ]] && mkdir -p $tDir
	
	cp "${LICENSE}" "$tDir"/.
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	
	mv lib${STAGE}.so $tDir/.
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
fi


if [[ ! -e "$targetLibDir/../../bin64/libs/lib${STAGE}.dylib" ]]; then
	item="$sDir/lib${STAGE}.dylib.bz2"
	
	if [[ ! -f "$item" ]] && [[ ! -f "lib${STAGE}.dylib" ]]; then
	    echo "Download ${STAGE} OSX binary from cisco ..."
	    
	    curl -L -o "$item" "https://github.com/cisco/openh264/releases/download/v1.4.0/libopenh264-1.4.0-osx64.dylib.bz2"
	    [[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	    
	    echo "Done."
	fi
	
	cd "$sDir"
	
	if [[ ! -f "$sDir/lib${STAGE}.dylib" ]]; then
		echo "Unpacking ${STAGE} OSX binary ..."
		
		bzip2 -d lib${STAGE}.dylib.bz2
		[[ $? != 0  ]] && echo "Error" && exit 1
		echo "Done."
	fi
	
	tDir="$targetLibDir/../../bin64/libs"
	echo -e
	echo "Preparing ${STAGE} directory [$tDir]..."
	echo '----------------------------------------'
	[[ ! -d "$tDir" ]] && mkdir -p $tDir
	
	cp "${LICENSE}" "$tDir"/.
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	
	mv lib${STAGE}.dylib $tDir/.
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
fi


if [[ ! -e "$targetLibDir/../../bin64/libs/lib${STAGE}.dll" ]]; then
	item="$sDir/lib${STAGE}.64.dll.bz2"
	
	if [[ ! -f "$item" ]] && [[ ! -f "lib${STAGE}.64.dll" ]]; then
	    echo "Download ${STAGE} Windows 64 bit binary from cisco ..."
	    
	    curl -L -o "$item" "https://github.com/cisco/openh264/releases/download/v1.4.0/openh264-1.4.0-win64msvc.dll.bz2"
	    [[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	    
	    echo "Done."
	fi
	
	cd "$sDir"
	
	if [[ ! -f "$sDir/lib${STAGE}.64.dll" ]]; then
		echo "Unpacking ${STAGE} Windows 64 bit binary ..."
		
		bzip2 -d lib${STAGE}.64.dll.bz2
		[[ $? != 0  ]] && echo "Error" && exit 1
		echo "Done."
	fi
	
	tDir="$targetLibDir/../../bin64/libs"
	echo -e
	echo "Preparing ${STAGE} directory [$tDir]..."
	echo '----------------------------------------'
	[[ ! -d "$tDir" ]] && mkdir -p $tDir
	
	cp "${LICENSE}" "$tDir"/.
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	
	mv lib${STAGE}.64.dll $tDir/lib${STAGE}.dll
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
fi


if [[ ! -e "$targetLibDir/../../bin/libs/lib${STAGE}.dll" ]]; then
	item="$sDir/lib${STAGE}.32.dll.bz2"
	
	if [[ ! -f "$item" ]] && [[ ! -f "lib${STAGE}.32.dll" ]]; then
	    echo "Download ${STAGE} Windows 32 bit binary from cisco ..."
	    
	    curl -L -o "$item" "https://github.com/cisco/openh264/releases/download/v1.4.0/openh264-1.4.0-win32msvc.dll.bz2"
	    [[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	    
	    echo "Done."
	fi
	
	cd "$sDir"
	
	if [[ ! -f "$sDir/lib${STAGE}.32.dll" ]]; then
		echo "Unpacking ${STAGE} Windows 64 bit binary ..."
		
		bzip2 -d lib${STAGE}.32.dll.bz2
		[[ $? != 0  ]] && echo "Error" && exit 1
		echo "Done."
	fi
	
	tDir="$targetLibDir/../../bin/libs"
	echo -e
	echo "Preparing ${STAGE} directory [$tDir]..."
	echo '----------------------------------------'
	[[ ! -d "$tDir" ]] && mkdir -p $tDir
	
	cp "${LICENSE}" "$tDir"/.
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	
	mv lib${STAGE}.32.dll $tDir/lib${STAGE}.dll
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
fi



# "Download ${STAGE} source from github ..."
#--------------------------------------------------

tDir="$targetDir/${STAGE}"
	
if [[ ! -e "$tDir/codec_api.h" ]]; then
	item="$sDir/${STAGE}.tar.gz"
	
	if [[ ! -f "$item" ]]; then
	    echo "Download ${STAGE} source from github ..."
	    
	    curl -L -o "$item" "https://github.com/cisco/openh264/archive/v1.4.0.tar.gz"
	    [[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	    
	    echo "Done."
	fi
	
	echo "Unpacking files ..."
	cd "$sDir"
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	
	[[ ! -d "${STAGE}" ]] && mkdir ${STAGE}
	tar -xzf $item -C ${STAGE} --strip-components=1
	
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	echo "Done."
	cd ${STAGE}
	
	tDir="$targetDir/${STAGE}"
	echo "Preparing ${STAGE} include directory [$tDir]..."
	echo '----------------------------------------'
	[[ ! -d "$tDir" ]] && mkdir -p $tDir
	
	cp codec/api/svc/* "$tDir"/.
	[[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	
	echo "Done."
fi


cd $tmpDir

tDir="$targetDir/msints"
echo -e
echo "Preparing msints directory [$tDir]..."
echo '----------------------------------------'
#[[ "$dryrun" == "0" ]] && [[ -d "$tDir" ]] && rm -rf "$tDir"
[[ ! -d "$tDir" ]] && mkdir $tDir

sDir="$tmpDir/msints"
#[[ "$dryrun" == "0" ]] && [[ -d "$sDir" ]] && rm -rf "$sDir"
[[ ! -d "$sDir" ]] && mkdir $sDir

item="${tmpDir}/msints.zip"

if [[ ! -f "$item" ]]; then
    echo "Download msints from googlecode ..."
    
    curl -o "$item" "https://msinttypes.googlecode.com/files/msinttypes-r26.zip"
    [[ $? != 0 ]] && echo "Error msints" && exit 1    
    
    echo "Done."
fi

cp "$item" "$sDir"/.
[[ $? != 0 ]] && echo "Error msints" && exit 1

echo "Unpacking files ..."
cd "$sDir"
[[ $? != 0 ]] && echo "Error msints" && exit 1

unzip -qo "$item"
[[ $? != 0 ]] && echo "Error msints" && exit 1
echo "Done."
cd -
    
echo "Moving files to [$tDir] ..."
cp $sDir/*.h $tDir/.
[[ $? != 0 ]] && echo "Error msints" && exit 1

echo "Done."


tDir="$targetDir/CL"
echo -e
echo "Preparing OpenCL directory [$tDir]..."
echo '----------------------------------------'

#[[ "$dryrun" == "0" ]] && [[ -d "$tDir" ]] && rm -rf "$tDir"
[[ ! -d "$tDir" ]] && mkdir $tDir

sDir="$tmpDir/msints"
#[[ "$dryrun" == "0" ]] && [[ -d "$sDir" ]] && rm -rf "$sDir"
[[ ! -d "$sDir" ]] && mkdir $sDir

echo "Download OpenCL from khronos.org ..."
echo '----------------------------------------'

dlKhronos "cl.h"
dlKhronos "opencl.h"
dlKhronos "cl_platform.h"
dlKhronos "cl_ext.h"
dlKhronos "cl_egl.h"
dlKhronos "cl_dx9_media_sharing.h"
dlKhronos "cl_d3d10.h"
dlKhronos "cl_d3d11.h"
dlKhronos "cl_gl.h"
dlKhronos "cl_gl_ext.h"

echo "Done."


tDir="$targetDir/TUIO"
echo -e
echo "Preparing TUIO directory [$tDir]..."
echo '----------------------------------------'

[[ "$dryrun" == "0" ]] && [[ -d "$tDir" ]] && rm -rf "$tDir"

sDir="$tmpDir/TUIO"
#[[ "$dryrun" == "0" ]] && [[ -d "$sDir" ]] && rm -rf "$sDir"
[[ ! -d "$sDir" ]] && mkdir $sDir

echo "Download TUIO source from sourceforge.net ..."
item="$tmpDir/tuio.zip"

if [[ ! -f "$item" ]]; then
    echo "Download TUIO source from sourceforge.net ..."
    
    curl -L -o "$item" "http://prdownloads.sourceforge.net/reactivision/TUIO11_CPP-1.1.5.zip?download"
    [[ $? != 0 ]] && echo "Error TUIO" && exit 1
    
    echo "Done."
fi

echo "Unpacking files ..."
cd "$tmpDir"
[[ $? != 0 ]] && echo "Error TUIO" && exit 1

[[ ! -d "TUIO" ]] && mkdir TUIO

unzip -qo tuio.zip -d TUIO/
[[ $? != 0 ]] && echo "Error TUIO" && exit 1
echo "Done."
cd TUIO

TUIODir=""

for entry in ./*
do  
  [[ -d "$entry" ]] && TUIODir="$entry" && break
done
[[ ! -d "${TUIODir}" ]] && echo "Error: TUIO directory is empty!" && exit 1
#echo ${TUIODir}

cd ${TUIODir}
[[ $? != 0 ]] && echo "Error ${TUIODir}" && exit 1

if [[ ! -d "$targetDir/TUIO" ]]; then
	echo "Moving TUIO to [$tDir] ..."
	mv TUIO $targetDir/.
	[[ $? != 0 ]] && echo "Error TUIO" && exit 1
fi

if [[ ! -d "$targetDir/TUIO/ip" ]]; then
	mv oscpack/* $targetDir/TUIO/.
	[[ $? != 0 ]] && echo "Error TUIO/ip" && exit 1
fi

# Restore 
rm -rf "$targetDir/TUIO"/LibExport.h
git checkout -- $targetDir/TUIO/LibExport.h

echo "Done."

cd ../..

echo -e
echo "Cleanup tmp directory..."

[[ -d "TUIO" ]] && rm -rf TUIO
[[ -d "msints" ]] && rm -rf msints
echo "Done."




