#! /bin/bash

source script/discover_os

discover_os

# check if the directory $1/HDF5 exist

if [ -d "$1/ADIOS2" -a -f "$1/ADIOS2/include/adios2.h" ]; then
  echo "ADIOS2 is already installed"
  exit 0
fi

rm -rf ADIOS2-2.9.1
rm ADIOS2-2.9.1.tar.gz
wget -O ADIOS2-2.9.1.tar.gz https://github.com/ornladios/ADIOS2/archive/refs/tags/v2.9.1.tar.gz
tar -xf ADIOS2-2.9.1.tar.gz
cd ADIOS2-2.9.1

mkdir build
cd build
cmake ../. -DADIOS2_USE_MPI=ON -DADIOS2_USE_Fortran=OFF  -DCMAKE_INSTALL_PREFIX=$1/ADIOS2 #MACOSSONOMA -DCMAKE_DISABLE_FIND_PACKAGE_LibFFI=TRUE  -DCMAKE_DISABLE_FIND_PACKAGE_BISON=TRUE -DCMAKE_DISABLE_FIND_PACKAGE_CrayDRC=TRUE -DCMAKE_DISABLE_FIND_PACKAGE_FLEX=TRUE -DCMAKE_DISABLE_FIND_PACKAGE_LibFFI=TRUE -DCMAKE_DISABLE_FIND_PACKAGE_NVSTREAM=TRUE
make -j $2

make install
cd ../..
rm ADIOS2-2.9.1.tar.gz
rm -rf ADIOS2-2.9.1
if [ $? -ne 0 ]; then
    echo "ADIOS2 error installing"
    exit 0
fi
echo 1 > $1/ADIOS2/version
