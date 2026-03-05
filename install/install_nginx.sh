#!/bin/bash

mkdir -p /opt/nginx

cp -rf ./nginx/* /opt/nginx/

chmod +x /opt/nginx/sbin/nginx
cp ./nginx/nginx.service /lib/systemd/system/nginx.service

systemctl daemon-reload
systemctl enable nginx
systemctl restart nginx
systemctl status nginx --no-pager
