#!/bin/bash

# The script builds release version of UGENE.
# Source code of UGENE is expected to be in 'ugene' folder, the build version is in 'ugene/src/_release'.
# The script does not pack the build into MacOS application bundle.

if [ ! -d "${QT_DIR}" ]; then
  echo "QT_DIR: ${QT_DIR} is not found."
  exit 1
fi

TEAMCITY_WORK_DIR=$(pwd)
UGENE_DIR="${TEAMCITY_WORK_DIR}/ugene"
BUILD_DIR="${UGENE_DIR}/src/_release"

cd "${UGENE_DIR}" || {
  echo "Can't change dir to '${UGENE_DIR}'"
  exit 1
}

echo "##teamcity[blockOpened name='env']"
env
echo "##teamcity[blockClosed name='env']"

if [ "${UGENE_BUILD_SKIP_CLEAN}" != "1" ]; then
  echo "##teamcity[blockOpened name='clean']"
  rm -rf "${BUILD_DIR}"
  echo "##teamcity[blockClosed name='clean']"
fi

if [ "${UGENE_BUILD_SKIP_QMAKE}" != "1" ]; then
  if
    "${QT_DIR}/bin/qmake" -r ugene.pro
  then
    echo
  else
    echo "##teamcity[buildStatus status='FAILURE' text='{build.status.text}. qmake -r ugene.pro failed']"
    exit 1
  fi
  echo "##teamcity[blockClosed name='qmake -r ugene.pro']"
fi

if [ "${UGENE_BUILD_SKIP_MAKE}" != "1" ]; then
  CORES=$(sysctl -n hw.ncpu)
  if [ -n "${CORES}" ]; then
    UGENE_MAKE_PARAMS="-j 4 "${UGENE_MAKE_PARAMS}
  fi
  echo "##teamcity[blockOpened name='make ${UGENE_MAKE_PARAMS}']"
  if
    make ${UGENE_MAKE_PARAMS}
  then
    echo
  else
    echo "##teamcity[buildStatus status='FAILURE' text='{build.status.text}. make $UGENE_MAKE_PARAMS failed']"
    exit 1
  fi
  echo "##teamcity[blockClosed name='make ${UGENE_MAKE_PARAMS}']"
fi
