#!/bin/bash

set -e

Select=`. /etc/os-release && echo $ID`
# osName=`. /etc/os-release && echo $ID`

shopt -s extglob

compileUbt2204()
{
    rm -rf install_ubt2204
    mkdir -p build
    cd build
    rm -rf !(*.pem|*.ini|*.tar.gz|*.so.*|*.config)
    # Release Debug
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$PWD/../install_ubt2204 
    
    make -j8
    make install
    strip ../install_ubt2204/lib/*.so
    cd ../install_ubt2204
    tar -zcvf plugin_x86.tar.gz *
}
compileCentos()
{
    cd install
    rm -rf lib/*.so include config.ini plugin_x86.tar.gz
    cd ..
    mkdir -p build-centos
    cd build-centos
    # 启用扩展模式 
    ## 删除除 file1、dir1 和 .log 文件外的所有内容
    rm -rf !(*.pem|*.ini)
    # Release Debug
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$PWD/../install  -DBUILD_SYSTEM=centos

    make -j8
    make install
    strip ../install/lib/*.so
    cd ../install
    tar -zcvf plugin_x86.tar.gz *
}

case "$Select" in
	centos)
		compileCentos
	;;
    ubuntu)
        compileUbt2204
    ;;
esac