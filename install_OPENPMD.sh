#! /bin/bash

source script/discover_os

discover_os

# check if the directory $1/HDF5 exist

./script/install_ADIOS2.sh $1 $2

if [ -d "$1/OPENPMD" -a -f "$1/OPENPMD/include/openPMD/openPMD.hpp" ]; then
  echo "OPENPMD is already installed"
  exit 0
fi

wget https://github.com/openPMD/openPMD-api/archive/refs/tags/0.15.2.tar.gz -O openPMD-api-0.15.2.tar.gz
tar -xf openPMD-api-0.15.2.tar.gz
cd openPMD-api-0.15.2

mkdir build
cd build

cmake ../. -DopenPMD_USE_MPI=ON -DopenPMD_USE_ADIOS2=ON -DHDF5_DIR=$1/HDF5 -DHDF5_ROOT=$1/HDF5  -DopenPMD_USE_HDF5=ON -DCMAKE_PREFIX_PATH="$1/ADIOS2;$1/OPENPMD/lib64/cmake/openPMD/" -DCMAKE_INSTALL_PREFIX=$1/OPENPMD

make -j $2
make install
rm openPMD-api-0.15.2.tar.gz
rm -rf openPMD-api-0.15.2
if [ $? -ne 0 ]; then
    echo "openPMD error installing"
    exit 0
fi
echo 1 > $1/OPENPMD/version

