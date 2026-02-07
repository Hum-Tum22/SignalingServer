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


echo "[Unit]
Description=GB28181App
After=network.target

[Service]
Environment=LD_LIBRARY_PATH=/mnt/data/gb28181/
Environment="CONFIG_FILE=/mnt/data/gb28181/repro.config"
Type=notify
WorkingDirectory=/mnt/data/gb28181/
ExecStart=/mnt/data/gb28181/GB28181App
ExecStop=pgrep GB28181App | xargs kill -9
NotifyAccess=all
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target" > /usr/lib/systemd/system/gb28181.service

systemctl daemon-reload
systemctl enable gb28181
systemctl restart gb28181

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD