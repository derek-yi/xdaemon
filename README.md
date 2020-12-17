# oran_daemon  

## 编译RRU版本    
    export ARCH=arm
    export BD_TYPE=rru  (if no BD_TYPE, make rru as default)
	export DAEMON_RELEASE=1  (0-debug, 1-release)
    make

## 编译RHUB版本  
    export ARCH=arm  
    export BD_TYPE=rhub  
	export DAEMON_RELEASE=1(0-debug, 1-release)
    make

## 编译ko
1、check & modify kernel_dir in Makefile  
2、make modules  



