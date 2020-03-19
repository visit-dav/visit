#
# Process the command line
#
version=undefined

for abc
do
    case $1 in
      -v)
        version=$2
        shift 2
        ;;
    esac
done

if [ $version = undefined ]
then
   echo "Usage: -v <version>"
   exit
fi

version2=`echo $version | tr "." "_"`

#
# Create the distribution.
#
tar zxf visit$version.tar.gz
cd visit$version
mkdir build
cd build
/home/visit/third-party/cmake/3.9.3/*/bin/cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release -DVISIT_INSTALL_THIRD_PARTY:BOOL=ON \
  -DVISIT_ENABLE_XDB:BOOL=ON -DVISIT_PARADIS:BOOL=ON \
  -DVISIT_CONFIG_SITE="/home/visit/visit-config.cmake" ../src
make manuals
make -j 4 package
mv visit$version2.linux-x86_64.tar.gz ../..

#
# Test the distribution.
#
cd ../..
cp visit$version/src/tools/dev/scripts/visit-install .
./visit-install -c none $version linux-x86_64 visit
visit/bin/visit -cli -nowin -s test_visit.py
