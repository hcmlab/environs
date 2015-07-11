#!/bin/bash
######################################################
#
# Install Environs to the example apps
# Download 3rd party Android projects
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

echo -e
echo "----------------------------------------"
echo "Install/Distribute Android native layer"
echo "----------------------------------------"
SRC="${SCRIPTDIR}"/../libs
[[ ! -e "${SRC}" ]] && echo "Error: Build [${SRC}] not found." && exit 1

DST="${SCRIPTDIR}/../Androids/MediaBrowser/app/libs"
[[ ! -d "${DST}" ]] && mkdir -p "${DST}" && [[ $? != 0 ]] && echo "Error." && exit 1

cp "${SRC}"/*.aar* "${DST}"/.
[[ $? != 0 ]] && echo "Error." && exit 1

DST="${SCRIPTDIR}/../Androids/SimpleEnvirons/app/libs"
[[ ! -d "${DST}" ]] && mkdir -p "${DST}" && [[ $? != 0 ]] && echo "Error." && exit 1

cp "${SRC}"/*.aar* "${DST}"/.
[[ $? != 0 ]] && echo "Error." && exit 1

CURDIR="${SCRIPTDIR}"
SCRIPTDIR="${SCRIPTDIR}/../3rd"
cd ${SCRIPTDIR}

. "${CURDIR}"/download.android.projects.sh $cleanCmd

echo "Done."
exit 0

