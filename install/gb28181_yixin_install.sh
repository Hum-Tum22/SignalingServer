#!/bin/bash

set -e

mkdir -p /opt/gb28181
cp -rf ./GB28181App /opt/gb28181/
cp -rf ./libsdkJson.so.1.0 /opt/gb28181/
cp -n ./repro.config /opt/gb28181/
chmod +x /opt/gb28181/GB28181App



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

# 安装nginx
chmod +x ./install_nginx.sh
./install_nginx.sh

cd /opt/gb28181
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

# 启动gb28181
systemctl enable gb28181
systemctl daemon-reload
systemctl enable gb28181
systemctl restart gb28181
systemctl status gb28181 --no-pager
