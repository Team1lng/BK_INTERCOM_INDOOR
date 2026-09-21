./configure \
prefix=$(pwd)/output \
CFLAGS="-fPIC" \
--host=arm-linux \
CC=/opt/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-gcc \
CXX=/opt/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-g++  \`
