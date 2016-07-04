#!/bin/bash
######################################################
#
# Prepare windows binaries environment
#
# Author: Chi-Tai Dang, dang@hcm-lab.de
# Copyright (c) Chi-Tai Dang, University of Augsburg
#
######################################################

DBG=1

rm .env.*

if [[ -e "/C/Windows/System32/msvcr140.dll" ]]; 
then
	echo "Preparing v140 Environment ..."
	
	cp "./libs/v140/Environs.Net.dll" .
	cp "./libs/v140/Environs.PixelSense.dll" .
	echo "" >>./.env.v140
	exit 0
fi

if [[ -e "/C/Windows/System32/msvcr120.dll" ]];
then
	echo "Preparing v120 Environment ..."
	
	cp "./libs/v120/Environs.Net.dll" .
	cp "./libs/v120/Environs.PixelSense.dll" .
	echo "" >>./.env.v120
	exit 0
fi

if [[ -e "/C/Windows/System32/msvcr100.dll" ]];
then
	echo "Preparing v100 Environment ..."
	
	cp "./libs/v100/Environs.Net.dll" .
	cp "./libs/v100/Environs.PixelSense.dll" .
	echo "" >>./.env.v100
	exit 0
fi

echo "Error: No supported windows runtime detected!"
exit 1