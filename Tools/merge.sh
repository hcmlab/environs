#!/bin/bash

if [[ -z "${SCRIPTPATH}" ]]; then
	pushd `dirname $0` > /dev/null
	SCRIPTPATH=`pwd`
	popd > /dev/null
fi

SCRIPTNAME=$(basename $0)

#echo "Usage ${SCRIPTNAME} dscan[0|1] repo[0|1] dryrun[0|1]"

# Parameters to adapt to your branch
#------------------------------------------
# The relative path from this script to the root of your repository
REPO_ROOT=".."

# The directory name that the merge helper scripts will copied into
MERGER_WORKDIR="merger"

# The relative path from this script to the merger tools
TOOLS_DIR=
#------------------------------------------

dryrun=0
domerge=0
doprepare=0
data=
dscan=0
getLatestMerger=0
syncBeforeMerge=1

# Parse branch
if [[ $# > 0 ]]; then

	if [[ "$1" == *h* ]]; then
		echo -e
		echo "Usage: ${SCRIPTNAME} [dscan] [syncBeforeMerge] [dryrun] [branchname]"
		echo -e
		echo "  All arguments are optional. Defaults are dscan=0, syncBeforeMerge=1,"
		echo "      dryrun=0, branch=autodetect"
		echo -e
		echo "  where 0: disabled, 1: enabled"
		echo -e
		echo "  e.g. ${SCRIPTNAME} 1 0)"
		echo "  (merge with dscan enabled and without syncing with origin)"
		exit 0
	fi

    [[ "$1" == "1" ]] && dscan=1
    
    if [[ $# > 1 ]]; then
		syncBeforeMerge=$2
		
		if [[ $# > 2 ]]; then
		    dryrun=$3
		    if [[ $# > 3 ]]; then
				data=$4
		    fi
		fi	 
    fi
      
fi

function execheck {
    eval $1
    ret=$?
    [[ $ret != 0 ]] && echo "ERROR $ret: [$1]" && exit $?
}
    
MERGE_CONF_NAME="do-merge.sh.conf"

if [[ "$SCRIPTPATH" != *"${MERGER_WORKDIR}" ]]; then    
    DEST="${SCRIPTPATH}/${REPO_ROOT}/../${MERGER_WORKDIR}"

    [[ ! -d $DEST ]] && execheck "mkdir \"$DEST\""

    # Get absolute path to repository root
    #
    cd "${SCRIPTPATH}/${REPO_ROOT}"
    [[ $? != 0 ]] && echo "ERROR $ret: [$1]" && exit $?

    AREPO_ROOT=$(pwd)

    cd - >/dev/null
    [[ $? != 0 ]] && echo "ERROR $ret: [$1]" && exit $?
    
    # Build merge configuration file
    #
    MERGE_CONF="${DEST}/do-${MERGE_CONF_NAME}-conf"
    
    echo "export MERGE_HELPER_ROOT_REPO=\"${AREPO_ROOT}\"" > "${MERGE_CONF}"
    
    ATOOLS_DIR="${SCRIPTPATH}"
    
    if [[ ! -z "${TOOLS_DIR}" ]]; then
		ATOOLS_DIR="$ATOOLS_DIR/${TOOLS_DIR}"
    fi
    echo "export MERGE_HELPER_TOOLS_REPO=\"${ATOOLS_DIR}\"" >> "${MERGE_CONF}"

    execheck "cp ${SCRIPTPATH}/${SCRIPTNAME} ${DEST}/do-${SCRIPTNAME}"

    echo "Prepared merger environment."
    echo "Enter following to run: ../$MERGER_WORKDIR/do-${SCRIPTNAME} $2 $1 $3"
    exit 0
fi

MERGE_CONF="${SCRIPTPATH}/do-${MERGE_CONF_NAME}-conf"

[[ -e "${MERGE_CONF}" ]] && source "${MERGE_CONF}"

[[ -z "${MERGE_HELPER_ROOT_REPO}" ]] && echo "Error: [${MERGE_HELPER_ROOT_REPO}" && exit 1

cd "${MERGE_HELPER_ROOT_REPO}"
[[ $? != 0 ]] && echo "Error." && exit 1
 
DEST="${MERGE_HELPER_ROOT_REPO}/../${MERGER_WORKDIR}"

if [[ -e "3rd/download.sh" ]]; then
    tmpSD="${SCRIPTDIR}"
    SCRIPTDIR="${MERGE_HELPER_ROOT_REPO}/3rd"

    eval "3rd/download.sh 3"

    SCRIPTDIR="${tmpSD}"
fi

echo -e
echo -e "Make sure that NDEBUG flag is set."
echo -n "Press return or y to continue (any other key will quit the process): "
read response
[[ "$response" != 'y' ]] && [[  "$response" != '' ]] && exit 0
echo -e

[[ "$data" == "" ]] && data=$(git rev-parse --abbrev-ref HEAD)

echo Collected:$# $data $dryrun $dscan

echo -e "\nStashing current working state of ${data}..."
git stash -u

if [[ "$getLatestMerger" != "0" ]]; then
	echo -e "\nUpdating develop branch to the latest HEAD revision ...\n"

	echo -e "\nFetching origin ..."
	execheck "git fetch origin"
	
	echo -e "\nSwitching to develop ..."
	execheck "git checkout master"
	
	echo -e "\nUpdating develop branch ..."
	execheck "git pull"
else
	echo -e "\nCheckout merger tools from develop ..."
	execheck "git checkout master -- \"${MERGE_HELPER_TOOLS_REPO}\""
fi

echo -e "\nget merger tools..."

#echo rm -rf "${MERGE_HELPER_TOOLS_REPO}/src"
rm -rf "${MERGE_HELPER_TOOLS_REPO}/src"

execheck "cp '${MERGE_HELPER_TOOLS_REPO}'/* '${SCRIPTPATH}'/."
chmod ogu+x "${SCRIPTPATH}"/*

if [[ "$getLatestMerger" != "0" ]]; then
	echo -e "\nSwitching back to branch $data ..."
	execheck "git checkout ${data}"
	
	#echo -e "\nLoading previous working state of $data ..."
	#execheck "git stash pop"
else
	echo -e "\nRestore current branch for merging ..."
	execheck "git reset -q HEAD \"${MERGE_HELPER_TOOLS_REPO}\" >/dev/null"
	
	execheck "git clean -df \"${MERGE_HELPER_TOOLS_REPO}\" >/dev/null"
	
	execheck "git checkout -- \"${MERGE_HELPER_TOOLS_REPO}\""
fi

# Invoke merge
echo -e
echo -n "Invoking merge ..."
echo -e

"${DEST}"/git-update.sh $data $dscan $syncBeforeMerge $dryrun
[[ $? != 0 ]] && echo "Error." && exit 1

git status >status.log
echo -e "\nCheck status.log for details."

echo -e "\nCleaning merge environment ..."
rm -rf "${DEST}" >/dev/null 2>/dev/null

if [[ -e "3rd/download.sh" ]]; then
    tmpSD="${SCRIPTDIR}"
    SCRIPTDIR="${MERGE_HELPER_ROOT_REPO}/3rd"

    eval "3rd/download.sh 4"

    SCRIPTDIR="${tmpSD}"
fi

echo -e
echo "Merge successfull"

exit 0

