#!/usr/bin/env bash

set -e

echo '----------------------------------------------------------------------------'
echo 'Welcome to Fast Flow Cloud API Setup Wizard.'
echo ''
echo '----------------------------------------------------------------------------'

if echo "$SHELL" | grep 'bash' >/dev/null 2>&1 ; then
  MYPATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
else
  MYPATH="$( cd "$( dirname "$0" )" >/dev/null 2>&1 && pwd )"
fi

FFC_ROOT=`cd $MYPATH/.. && pwd`

# read command line args
SUDO=''
INSTALL_DIR=$FFC_ROOT/installation
JOBS='-j2'

for i in "$@"
do
case $i in
  -h|--help)
  echo ''
  echo 'Usage: sh install.sh [OPTION]'
  echo ''
  echo '-h  , --help         show arguments'
  echo '      --prefix=PATH  override default install location'
  echo '      --install-as-root  root user is necessary to install the applications'
  echo '-j=N, --jobs=N       specifies the number of jobs to run simultaneously'
  exit 0
  shift # past argument=value
  ;;
  --prefix=*)
  INSTALL_DIR="${i#*=}"
  shift # past argument=value
  ;;
  --install-as-root)
  SUDO='sudo '
  shift # past argument with no value
  ;;
  -j=*|--jobs=*)
  JOBS="-j${i#*=}"
  shift # past argument=value
  ;;
  *)
  # unknown option
  echo ''
  echo 'install.sh: invalid option!'
  echo ''
  echo "Try: 'bash install.sh --help' for more information."
  exit 1
  ;;
esac
done

$SUDO mkdir -p $INSTALL_DIR

cd $INSTALL_DIR
INSTALL_DIR=`pwd`
cd $CALLPATH

check_lib() {
  LIB_FILE=$1.'so'
  CHECK=$(ldconfig -p | grep ${LIB_FILE})

  if [[ ${CHECK} != *${LIB_FILE}* ]]
  then
    tput setaf 1; echo 'Error: '${LIB_FILE}' is not installed!'
    exit 1
  fi
}


echo "cleaning up..."
echo ""
rm -rf $FFC_ROOT/opae-sdk/mybuild
echo "end of cleaning up!"
echo ""

echo "installing FFC..."
echo ""

make $JOBS
$SUDO make install
echo "end of installing FFC"
echo ""

echo "Installation completed successfully!"
echo ""

cd $CALLPATH
