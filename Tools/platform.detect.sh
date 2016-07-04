#!/bin/bash
######################################################
#
# Platform Detection
#
# Author: Chi-Tai Dang, dang@hcm-lab.de
# Copyright (c) Chi-Tai Dang, University of Augsburg
#
######################################################

#if [[ "${PLATFORMDETECTED}" == "" ]]; then
	PLATFORMDETECTED=0
	ISRASPBERYY=
	ISWINDOWS=
	isWindows=
	ISUBUNTU=
	ISDEBIAN=
	ISOSX=
#fi

function detectPlatform
{
	RETSTR=
	
	#if [[ "${PLATFORMDETECTED}" == "1" ]]; then
	#	if [[ "$1" == "" ]] || [[ "$1" == "3" ]]; then
	#		return 0
	#	fi
	#fi

	PLATFORMDETECTED=1
	if [[ -e "/etc/issue" ]]; then
		ISSUE=`cat /etc/issue`
	
		if [[ "${ISSUE}" == *Rasp* ]]; then
			ISRASPBERYY=1
				
			ENVCC=g++-4.8			
			[[ ! -e "/usr/bin/${CC}" ]] && export ENVCC="g++-4.7"		
			
			if [[ "$1" == "1" ]]; then
				RETSTR=${ENVCC}
			elif [[ "$1" == "2" ]]; then
				RETSTR=-lrt
			else
				RETSTR="Detected: Raspberry PI. Using ${ENVCC}"
			fi	
		elif [[ "${ISSUE}" == *Ubuntu* ]]; then
			ISUBUNTU=1
			ISDEBIAN=1
			
			if [[ "$1" == "1" ]]; then
				RETSTR=g++
			elif [[ "$1" == "2" ]]; then
				RETSTR=
			else
				RETSTR="Detected: Ubuntu"
			fi	
		elif [[ "${ISSUE}" == *Debian* ]]; then
			ISDEBIAN=1
			
			if [[ "$1" == "1" ]]; then
				RETSTR=g++
			elif [[ "$1" == "2" ]]; then
				RETSTR=
			else
				RETSTR="Detected: Debian"
			fi	
		fi
	elif [[ -e "/C/Windows" ]]; then
		ISWINDOWS=1
		isWindows=1
		
		if [[ "$1" == "1" ]]; then
			RETSTR=
		elif [[ "$1" == "2" ]]; then
			RETSTR=
		else
			RETSTR="Detected: Windows"
		fi
	elif [[ -e "/Applications/Safari.app" ]]; then
		ISOSX=1
		
		if [[ "$1" == "1" ]]; then
			RETSTR=g++
		elif [[ "$1" == "2" ]]; then
			RETSTR=
		else
			RETSTR="Detected: OSX"
		fi
	fi	
	
	[[ ! "$1" == "3" ]] && [[ ! "$1" == "" ]] && echo ${RETSTR}
	return 0
}

detectPlatform $1


