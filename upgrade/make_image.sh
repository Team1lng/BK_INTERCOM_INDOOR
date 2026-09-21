#!/bin/sh
###
 # @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 # @Date: 2024-02-20 15:07:10
 # @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 # @LastEditTime: 2024-05-15 14:16:03
 # @FilePath: /two-wire-indoor/upgrade/make_image.sh
 # @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
### 
#在应用层只需修改app分区即可

#创建一个用于保存升级文件的目录
create_platform()
{
	if [ -d "platform" ]; then
                echo "remove platform dir"
                # rm -f "platform/*"
                # mkdir "platform"
        else
                echo "mkdir platform"
                mkdir "platform"
        fi
}

#用mksquashfs工具将app目录打包成 app.sqsh4 文件系统
make_squashfs_images()
{
        ./tools/mksquashfs ./app platform/app.sqsh4 -noappend -comp xz
}

#把 升级脚本 和 进度条显示程序 也放进去打包压缩
images_compress()
{
        cp -f scripts/update.sh                    platform/
        cp -f upgrade_progress/upgrade_progress    platform/

        rm -rf SAT_ANYKAOS*
        rm -rf ../SAT_ANYKAOS*
        cd platform/
        tar -zcvf $SAT_OS_CHECK *
        mv $SAT_OS_CHECK ../../
        cd ../
}

uboot_img_compress()
{
       cp -f platform/app.sqsh4 ../../../../Bcom/SDK/AK37E_SDK_V1.05/upgrade/platform
}

build_timestamp=$(date "+%m%d%H%M")

SAT_OS_CHECK=SAT_ANYKAOS_$build_timestamp

create_platform

make_squashfs_images

images_compress

# uboot_img_compress
