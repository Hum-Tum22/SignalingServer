#!/bin/bash

# 软链接的目标路径和链接名
#target="/usr/lib/x86_64-linux-gnu/libsystemd.so.0"
#link="/usr/lib/x86_64-linux-gnu/libsystemd.so"
#
## 检查软链接是否存在
#if [ ! -L "$link" ]; then
#    # 如果软链接不存在，则创建它
#    ln -s "$target" "$link"
#else
#    # 如果软链接已存在，则输出信息
#    echo "软链接 $link 已存在"
#fi

if [ -L "libsdkJson.so.1" ]; then
    echo "软连接存在"
else
    echo "软连接不存在 或 不是软连接"
    ln -s libsdkJson.so.1.0 libsdkJson.so.1
fi
if [ -L "libsdkJson.so" ]; then
    echo "软连接存在"
else
    echo "软连接不存在 或 不是软连接"
    ln -s libsdkJson.so.1 libsdkJson.so
fi

echo "[Unit]
Description=GB28181App
After=network.target remote-fs.target nss-lookup.target mysql.target

[Service]
Type=notify
PIDFile=/var/run/GB28181App.pid
WorkingDirectory=/opt/gb28181/
ExecStart=/opt/gb28181/GB28181App
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target" > /usr/lib/systemd/system/gb28181.service

systemctl enable gb28181

systemctl daemon-reload
systemctl enable gb28181
systemctl restart gb28181

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD