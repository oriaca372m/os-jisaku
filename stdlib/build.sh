#!/bin/sh

set -eu

cd "`dirname "$0"`"

exe() {
    echo "$@"
    "$@"
}

echo "==========================================="
echo "Preparing build environment."
echo "==========================================="
IIDFILE=./docker-image-id
exe sudo docker build --iidfile="${IIDFILE}" --tag stdlib-builder .
IID=$(cat ${IIDFILE})
exe rm -f ${IIDFILE}

echo "==========================================="
echo "Building standard libraries."
echo "==========================================="
CIDFILE=./docker-container-id
exe sudo docker run --cidfile="${CIDFILE}" ${IID}
CID=$(cat ${CIDFILE})
exe rm -f ${CIDFILE}

echo "==========================================="
echo "Copying standard libraries."
echo "==========================================="
rm -rf build
mkdir -p build
DST="`readlink -f build`"
exe sudo docker cp ${CID}:/usr/local/x86_64-elf/. "$DST"

SRC=/usr/local/src
exe sudo docker cp ${CID}:${SRC}/newlib-cygwin/COPYING.NEWLIB "${DST}/LICENSE.newlib"
exe sudo docker cp ${CID}:${SRC}/llvm-project/libcxx/LICENSE.TXT "${DST}/LICENSE.libcxx"
exe sudo docker cp ${CID}:${SRC}/freetype-2.10.1/docs/FTL.TXT "${DST}/LICENSE.freetype"

echo ""
echo "Done. Standard libraries at $DST"
