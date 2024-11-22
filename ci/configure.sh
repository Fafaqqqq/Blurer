#!/bin/bash

PROJECT=main
CMAKE=cmake
PROFILE=Release

while [ -n "$1" ]
do
  case "$1" in
  --name) PROJECT="$2";;
  --type) PROFILE="$2";;
  *) echo "$1 is not option." ;;
  esac
  shift
  shift
done

BLDDIR=.build/$PROJECT/$PROFILE
[ ! -d "$BLDDIR" ] && mkdir -p $BLDDIR

cd $BLDDIR
$CMAKE -G Ninja -DCMAKE_BUILD_TYPE=$PROFILE $OLDPWD
cd $OLDPWD
