#!/bin/bash
export daemon_home=/root/
export xlog_path=/tmp/oran_daemon/


if [ ! -d ${xlog_path} ]; then
    mkdir -p ${xlog_path}
fi

chmod +x $daemon_home/oran_daemon

while true
do
    procnum=`ps -ef|grep "oran_daemon"|grep -v grep|wc -l`
        if [ $procnum -eq 0  ];then
            #if [[ -e /tmp/ncxserver.sock ]];then
            #    rm -rf /tmp/ncxserver.sock
            #fi
            $daemon_home/oran_daemon
            echo `date +%Y-%m-%d` `date +%H:%M:%S`  "restart oran_daemon" >> $xlog_path/xlog.error.log
        fi
        sleep 3
done
