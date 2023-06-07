AEC_VERSION="1.0.4"
HDF5_VERSION="1.12.1"

yum update -y
yum -y install zlib-devel
pushd /tmp

echo "Downloading & unpacking aec ${AEC_VERSION}"
curl -fsSLO https://gitlab.dkrz.de/k202009/libaec/uploads/ea0b7d197a950b0c110da8dfdecbb71f/libaec-${AEC_VERSION}.tar.gz
tar zxf libaec-$AEC_VERSION.tar.gz

echo "Building & installing libaec"
pushd libaec-$AEC_VERSION
./configure
make -j 2
make install
ldconfig
popd

echo "Downloading & unpacking HDF5 ${HDF5_VERSION}"
curl -fsSLO "https://www.hdfgroup.org/ftp/HDF5/releases/hdf5-${HDF5_VERSION%.*}/hdf5-$HDF5_VERSION/src/hdf5-$HDF5_VERSION.tar.gz"
tar -xzvf hdf5-$HDF5_VERSION.tar.gz

echo "Building & installing HDF5"
pushd hdf5-$HDF5_VERSION
./configure                      \
  --prefix=/usr                  \
  --enable-build-mode=production \
  --with-szlib                   \
  --enable-tools=no              \
  --enable-tests=no
make -j 2
make install
ldconfig
popd
