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

source "${TOOLSDIR}/check.build.is.ci.sh"

tDir=
dryrun=0

source "${TOOLSDIR}"/download.tools.sh

obsolete=0
targetDir="$TOOLSDIR/../3rd/inc"
targetLibDir="$TOOLSDIR/../3rd/lib"

if [[ "$cleanCmd" == "1" ]]; then
	DEL_FILES=(
		"$targetDir/msints"
		"$targetDir/CL"
		"$targetDir/TUIO"
		"$targetLibDir/android/arm/libopenh264.so"
		"$targetLibDir/android/arm/openh264.LICENSE.txt"
		"$targetLibDir/../../bin64/libs/libopenh264.dylib"
		"$targetLibDir/../../bin64/libs/libopenh264.dll"
		"$targetLibDir/../../bin/libs/openh264.LICENSE.txt"
		"$targetLibDir/../../bin/libs/libopenh264.dll"
	)
	delFiles DEL_FILES[@]
    return 0
fi
if [[ "$cleanCmd" == "2" ]]; then
	DEL_FILES=(
		"$tmpDir/msints.zip"
		"$tmpDir/msints"
		"$tmpDir/TUIO.zip"
		"$tmpDir/TUIO"
		"$tmpDir/openh264"
	)
	delFiles DEL_FILES[@]
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
echo 'Verifying 3rd party header files and sources...'


prepareDir "${tmpDir}"

prepareDir "${targetDir}"

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
    if [[ ! -f "$tmpDir/CL/$1" ]]; then
		#curl -L -o "$tmpDir/CL/$1" "https://www.khronos.org/registry/cl/api/2.0/$1"
		curl -L -o "$tmpDir/CL/$1" "https://raw.githubusercontent.com/KhronosGroup/OpenCL-Headers/opencl20/$1"
		[[ $? != 0 ]] && echo "Error downloading $1" && exit 1
    fi

    if [[ ! -f "$tDir/$1" ]]; then
    	cp "$tmpDir/CL/$1" "$tDir/$1"
		[[ $? != 0 ]] && echo "Error copying $1" && exit 1
    fi
    return 0
}


cd $tmpDir

if [[ -e "${TOOLSDIR}/../Android" ]] && [[ "${ISRASPBERYY}" == "" ]]; then
	STAGE=openh264
	tDir="$targetDir/${STAGE}"
	echo -e
	echo "Preparing ${STAGE} directory [$tDir]..."
	echo '----------------------------------------'
	
	#[[ "$dryrun" == "0" ]] && [[ -d "$tDir" ]] && rm -rf "$tDir"
	
	sDir="$tmpDir/${STAGE}"
	#[[ "$dryrun" == "0" ]] && [[ -d "$sDir" ]] && rm -rf "$sDir"
	[[ ! -d "$sDir" ]] && mkdir $sDir
	
	
	echo "Preparing ${STAGE} binary license from openh264.org ..."
	LICENSE="$sDir/${STAGE}.LICENSE.txt"
	
	if [[ ! -f "$LICENSE" ]]; then
	    echo "Download ${STAGE} binary license from openh264.org ..."
	    
	    curl -L -o "$LICENSE" "http://www.openh264.org/BINARY_LICENSE.txt"
	    [[ $? != 0 ]] && echo "Error ${STAGE}" && exit 1
	    
	    echo "Done."
	
	    cat $LICENSE
	    echo 'Please take notice of the cisco openh264 binary license.'
	    echo 'Press any key to continue ...'
	    
	    if [[ -z "${CI_SERVER}" ]]; then
	    	read response
	    fi
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
		
		cp lib${STAGE}.so $tDir/.
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
		
		cp lib${STAGE}.dylib $tDir/.
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
		
		cp lib${STAGE}.64.dll $tDir/lib${STAGE}.dll
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
		
		cp lib${STAGE}.32.dll $tDir/lib${STAGE}.dll
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
fi

cd $tmpDir

tName=msints
tDir="$targetDir/$tName"
echo -e
echo "Verifying $tName ..."
echo '----------------------------------------'
#[[ "$dryrun" == "0" ]] && [[ -d "$tDir" ]] && rm -rf "$tDir"
prepareDir "${tDir}"

sDir="$tmpDir/$tName"
#[[ "$dryrun" == "0" ]] && [[ -d "$sDir" ]] && rm -rf "$sDir"
prepareDir "${sDir}"

item="${tmpDir}/$tName.zip"

if [[ ! -f "$item" ]]; then
    echo "Download $tName from googlecode ..."
    
    curl -o "$item" "https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/msinttypes/msinttypes-r26.zip"
    [[ $? != 0 ]] && echo "Error $tName" && exit 1    
    
    echo "Done."

	cp "$item" "$sDir"/.
	[[ $? != 0 ]] && echo "Error $tName" && exit 1
	
	echo "Unpacking files ..."
	cd "$sDir"
	[[ $? != 0 ]] && echo "Error $tName" && exit 1
	
	unzip -qo "$item"
	[[ $? != 0 ]] && echo "Error $tName" && exit 1
	echo "Done."
	cd -    
fi

if [[ ! -f "$tDir/inttypes.h" ]]; then
	echo "Copying files to [$tDir] ..."
	cp $sDir/*.h $tDir/.
	[[ $? != 0 ]] && echo "Error $tName" && exit 1
fi
echo "Done: $tName."

if [[ "${ISRASPBERYY}" == "" ]]; then
	tName=openssl
	tDir="$targetDir/$tName"
	echo -e
	echo "Preparing $tName directory [$tDir]..."
	echo '----------------------------------------'
	#[[ "$dryrun" == "0" ]] && [[ -d "$tDir" ]] && rm -rf "$tDir"
	prepareDir "${tDir}"

	sDir="$tmpDir/$tName"
	#[[ "$dryrun" == "0" ]] && [[ -d "$sDir" ]] && rm -rf "$sDir"
	prepareDir "${sDir}"

	item="${tmpDir}/$tName.zip"

	if [[ ! -f "$item" ]]; then
		echo "Download $tName ..."
	
		#curl -L -o "$item" "https://github.com/openssl/openssl/archive/master.zip"
		curl -L -o "$item" "https://github.com/openssl/openssl/archive/6e3d015363ed09c4eff5c02ad41153387ffdf5af.zip"
		[[ $? != 0 ]] && echo "Error $tName" && exit 1    
	
		echo "Done."
	
		cp "$item" "$sDir"/.
		[[ $? != 0 ]] && echo "Error $tName" && exit 1
	
		echo "Unpacking files ..."
		cd "$sDir"
		[[ $? != 0 ]] && echo "Error $tName" && exit 1
	
		unzip -qo "$item"
		[[ $? != 0 ]] && echo "Error $tName" && exit 1
		echo "Done."
		cd -
	fi

    
	if [[ ! -f "$tDir/aes.h" ]]; then
		echo "Copying files to [$tDir] ..."
	
		cp $sDir/openssl-6e3d015363ed09c4eff5c02ad41153387ffdf5af/include/openssl/* $tDir/.
		[[ $? != 0 ]] && echo "Error $tName" && exit 1
	fi
fi

echo "Done."




if [[ -e "${TOOLSDIR}/../Android" ]]; then
	tDir="$targetDir/CL"
	echo -e
	echo "Preparing OpenCL ..."
	echo '----------------------------------------'
	
	#[[ "$dryrun" == "0" ]] && [[ -d "$tDir" ]] && rm -rf "$tDir"
	prepareDir "${tDir}"
	prepareDir "${tmpDir}/CL"
	
	echo "Updating OpenCL from khronos.org ..."
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
	
	
	tName=TUIO
	tDir="$targetDir/$tName"
	echo -e
	echo "Preparing $tName directory [$tDir]..."
	echo '----------------------------------------'
	
	#[[ "$dryrun" == "0" ]] && [[ -d "$tDir" ]] && rm -rf "$tDir"
	
	sDir="$tmpDir/$tName"
	#[[ "$dryrun" == "0" ]] && [[ -d "$sDir" ]] && rm -rf "$sDir"
	prepareDir "${sDir}"
	
	item="$tmpDir/tuio.zip"
	
	if [[ ! -f "$item" ]]; then
	    echo "Download $tName source from sourceforge.net ..."
	    
	    curl -L -o "$item" "http://prdownloads.sourceforge.net/reactivision/TUIO11_CPP-1.1.5.zip?download"
	    [[ $? != 0 ]] && echo "Error $tName" && exit 1
	    
	    echo "Done."
	
		echo "Unpacking files ..."
		cd "$tmpDir"
		[[ $? != 0 ]] && echo "Error $tName" && exit 1
		
		prepareDir "${tName}"
		
		unzip -qo tuio.zip -d $tName/
		[[ $? != 0 ]] && echo "Error $tName" && exit 1
		echo "Done."
	fi
	
	cd $tName
	TUIODir=""
	
	for entry in ./*
	do  
	  [[ -d "$entry" ]] && TUIODir="$entry" && break
	done
	[[ ! -d "${TUIODir}" ]] && echo "Error: $tName directory is empty!" && exit 1
	#echo ${TUIODir}
	
	cd ${TUIODir}
	[[ $? != 0 ]] && echo "Error ${TUIODir}" && exit 1
	
	if [[ ! -f "$targetDir/$tName/TuioClient.h" ]]; then
		prepareDir "${tDir}"
		
		echo "Copying $tName to [$tDir] ..."
		
		cp -R TUIO/* $targetDir/$tName/.
		[[ $? != 0 ]] && echo "Error $tName" && exit 1
	fi
	
	if [[ ! -d "$targetDir/$tName/ip" ]]; then
		cp -R oscpack/* $targetDir/TUIO/.
		[[ $? != 0 ]] && echo "Error $tName/ip" && exit 1
	fi
fi

echo -e
echo "Cleanup tmp directory..."
#[[ -d "TUIO" ]] && rm -rf TUIO
#[[ -d "msints" ]] && rm -rf msints
echo "Done."

cd "${TOOLSDIR}/.."

# Restore 
if [[ -e "${TOOLSDIR}/../Android" ]]; then
    rm -rf "$targetDir/TUIO"/LibExport.h
    git checkout -- "$targetDir/TUIO/LibExport.h"
fi

echo "Done."




