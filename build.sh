###
 # @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 # @Date: 2023-07-04 10:33:17
 # @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 # @LastEditTime: 2024-04-11 16:20:03
 # @FilePath: /two-wire-indoor/build.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 

CC="$(pwd)/compilat_tool/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-gcc"
CXX="$(pwd)/compilat_tool/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-g++"

clear
rm CMakeFiles/ -r
rm CMakeCache.txt -f
cmake ./ -DCMAKE_C_COMPILER=${CC} -DCMAKE_CXX_COMPILER=${CXX}
make clean
make -j16

cp ./src/layout/resource/rom.bin ./
#tar -czf ANYKA37EOS ANYKA37E.BIN rom.bin
# tar -czvf two_wire_indoor.update ANYKA37E.BIN rom.bin ak_eth.ko
#cp -f ./two_wire_indoor.update /home/share/run_bin/
cp -f ./ANYKA37E.BIN ./upgrade/app/
cp -f ./rom.bin ./upgrade/app/
cp -f ./language.xls ./upgrade/app/
# cp -f ./sat_leo.ttf ./upgrade/app/

# # 拷贝到SDK
# rm -rf ../../../MEIOU/SDK/AK37E_SDK_V1.05/rootfs/rootfs/app/app
# cp -rf ./upgrade/app ../../../MEIOU/SDK/AK37E_SDK_V1.05/rootfs/rootfs/app/
# cp -f ./upgrade/platform/anyka_logo.rgb ../../../MEIOU/SDK/AK37E_SDK_V1.05/image/anyka_logo.rgb


# rm -rf ../../../Bcom/SDK/AK37E_SDK_V1.05/rootfs/rootfs/app/app
# cp -rf ./upgrade/app ../../../Bcom/SDK/AK37E_SDK_V1.05/rootfs/rootfs/app/
# cp -f ./upgrade/platform/anyka_logo.rgb ../../../Bcom/SDK/AK37E_SDK_V1.05/image/anyka_logo.rgb

cd ./upgrade
./make_image.sh
cd -

echo "Done!!!"
# cp ./ANYKA37EOS rom.bin ANYKA37E.BIN /home/shf/NFS

#./configure --prefix=/opt/madplay/target-x86 --disable-debugging --disable-shared --enable-static CPPFLAGS=-I/opt/madplay/target-x86/include LDFLAGS=-L/opt/madplay/target-x86/lib
