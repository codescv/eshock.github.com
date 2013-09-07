---
layout: post
title: "为virtualbox中的Linux虚拟机磁盘扩容"
date: 2013-08-20 20:02
comments: true
categories: 
---

最近想自己hack一下android的源代码, 可是发现代码好大啊, 编译一下大概需要40G的空闲空间. 而之前装好的Linux本来硬盘就只有40G, 于是研究了一下, 发现是可以直接扩容的, 虽然步骤有点麻烦.

## 备份数据
虽然说按照这个步骤做应该不会出什么问题, 但是重装系统毕竟是个麻烦事, 所以在扩容之前, 先做一下备份! 避免产生人品问题而后悔莫及. 毕竟备份虚拟机还是很省事的, 直接整个目录拷贝一份即可.

## 扩大VirtualDisk
我们先把VirtualBox里的这块硬盘扩大:

    C:
    cd "C:\Program Files\Oracle\VirtualBox"
    VBoxManage modifyhd "D:\VM\mint-xfce\mint-xfce.vdi" --resize 1000000

正常的话, 系统会输出:

    0%...10%...20%...30%...40%...50%...60%...70%...80%...90%...100%
然后就退出了.

我们可以去VirtualBox里看看硬盘大小:

![](images/virtualdisk.png)

已经变成1000G了(其实我不小心多打了1个0, 不过无所谓了).

## 在Linux中创建新的分区
进入Linux, 切换为root用户, 运行`fdisk -l`:

    chi-VirtualBox ~ # fdisk -l

    Disk /dev/sda: 1048.6 GB, 1048576000000 bytes
    255 heads, 63 sectors/track, 127482 cylinders, total 2048000000 sectors
    Units = sectors of 1 * 512 = 512 bytes
    Sector size (logical/physical): 512 bytes / 512 bytes
    I/O size (minimum/optimal): 512 bytes / 512 bytes
    Disk identifier: 0x00067091

       Device Boot      Start         End      Blocks   Id  System
    /dev/sda1   *        2048      499711      248832   83  Linux
    /dev/sda2          501758    83884031    41691137    5  Extended
    /dev/sda5          501760    83884031    41691136   8e  Linux LVM

    Disk /dev/mapper/mint--vg-root: 40.5 GB, 40542142464 bytes
    255 heads, 63 sectors/track, 4928 cylinders, total 79183872 sectors
    Units = sectors of 1 * 512 = 512 bytes
    Sector size (logical/physical): 512 bytes / 512 bytes
    I/O size (minimum/optimal): 512 bytes / 512 bytes
    Disk identifier: 0x00000000

    Disk /dev/mapper/mint--vg-root doesn't contain a valid partition table

    Disk /dev/mapper/mint--vg-swap_1: 2147 MB, 2147483648 bytes
    255 heads, 63 sectors/track, 261 cylinders, total 4194304 sectors
    Units = sectors of 1 * 512 = 512 bytes
    Sector size (logical/physical): 512 bytes / 512 bytes
    I/O size (minimum/optimal): 512 bytes / 512 bytes
    Disk identifier: 0x00000000

    Disk /dev/mapper/mint--vg-swap_1 doesn't contain a valid partition table

可以看到, 虽然硬盘的大小已经扩大了, 但是分区表还是没变. 所以现在我们去创建一个新的分区.
在我的Linux Mint里面有图形化界面可以创建, 所以我就不用`fdisk`了, 免得出错, 反正记得要
创建一个primary分区, 分区类型要选lvm(8e), 完成之后, 里面的分区应该是这样:

    chi-VirtualBox ~ # fdisk -l

    Disk /dev/sda: 1048.6 GB, 1048576000000 bytes
    255 heads, 63 sectors/track, 127482 cylinders, total 2048000000 sectors
    Units = sectors of 1 * 512 = 512 bytes
    Sector size (logical/physical): 512 bytes / 512 bytes
    I/O size (minimum/optimal): 512 bytes / 512 bytes
    Disk identifier: 0x00067091

       Device Boot      Start         End      Blocks   Id  System
    /dev/sda1   *        2048      499711      248832   83  Linux
    /dev/sda2          501758    83884031    41691137    5  Extended
    /dev/sda3        83886080  2047999999   982056960   8e  Linux LVM
    /dev/sda5          501760    83884031    41691136   8e  Linux LVM

    Disk /dev/mapper/mint--vg-root: 40.5 GB, 40542142464 bytes
    255 heads, 63 sectors/track, 4928 cylinders, total 79183872 sectors
    Units = sectors of 1 * 512 = 512 bytes
    Sector size (logical/physical): 512 bytes / 512 bytes
    I/O size (minimum/optimal): 512 bytes / 512 bytes
    Disk identifier: 0x00000000

    Disk /dev/mapper/mint--vg-root doesn't contain a valid partition table

    Disk /dev/mapper/mint--vg-swap_1: 2147 MB, 2147483648 bytes
    255 heads, 63 sectors/track, 261 cylinders, total 4194304 sectors
    Units = sectors of 1 * 512 = 512 bytes
    Sector size (logical/physical): 512 bytes / 512 bytes
    I/O size (minimum/optimal): 512 bytes / 512 bytes
    Disk identifier: 0x00000000

    Disk /dev/mapper/mint--vg-swap_1 doesn't contain a valid partition table


其中`/dev/sda3`是我们新创建的分区.

## 创建和扩展lvm分区
### 扩大volume group
由于使用了lvm, 我们可以方便的扩展已有分区. 首先查看已有的volume group:

    chi-VirtualBox ~ # vgdisplay 
    
    --- Volume group ---
    VG Name               mint-vg
    System ID             
    Format                lvm2
    Metadata Areas        1
    Metadata Sequence No  3
    VG Access             read/write
    VG Status             resizable
    MAX LV                0
    Cur LV                2
    Open LV               2
    Max PV                0
    Cur PV                1
    Act PV                1
    VG Size               39.76 GiB
    PE Size               4.00 MiB
    Total PE              10178
    Alloc PE / Size       10178 / 39.76 GiB
    Free  PE / Size       0 / 0   
    VG UUID               aR8Gl4-toof-YxoL-Jo2L-X8it-wfEe-B1YxUF

可以看到, 已有一个volume group, 我们可以把`/dev/sda3`加到这个volume group里面:

    chi-VirtualBox ~ # vgextend mint-vg /dev/sda3
    
    No physical volume label read from /dev/sda3
    Writing physical volume data to disk "/dev/sda3"
    Physical volume "/dev/sda3" successfully created
    Volume group "mint-vg" successfully extended

我们再看看volume group的情况:

    chi-VirtualBox ~ # vgdisplay 
    
    --- Volume group ---
    VG Name               mint-vg
    System ID             
    Format                lvm2
    Metadata Areas        2
    Metadata Sequence No  4
    VG Access             read/write
    VG Status             resizable
    MAX LV                0
    Cur LV                2
    Open LV               2
    Max PV                0
    Cur PV                2
    Act PV                2
    VG Size               976.32 GiB
    PE Size               4.00 MiB
    Total PE              249937
    Alloc PE / Size       10178 / 39.76 GiB
    Free  PE / Size       239759 / 936.56 GiB
    VG UUID               aR8Gl4-toof-YxoL-Jo2L-X8it-wfEe-B1YxUF
        
可以看到, 原来的volume group已经被我们扩大到了`976.32GB`. 

### 扩大 logical volume
首先, 我们查看目前的logical Volume:

    chi-VirtualBox ~ # lvdisplay 
    
    --- Logical volume ---
    LV Path                /dev/mint-vg/root
    LV Name                root
    VG Name                mint-vg
    LV UUID                2hNBQf-1zGs-3nZF-4feA-uR9W-WVDQ-32lLm4
    LV Write Access        read/write
    LV Creation host, time mint, 2013-07-23 09:26:24 +0800
    LV Status              available
    # open                 1
    LV Size                37.76 GiB
    Current LE             9666
    Segments               1
    Allocation             inherit
    Read ahead sectors     auto
    - currently set to     256
    Block device           252:0

    --- Logical volume ---
    LV Path                /dev/mint-vg/swap_1
    LV Name                swap_1
    VG Name                mint-vg
    LV UUID                fD6Mwy-IYEy-ZxXN-aPPW-WNn7-catg-o1bRsX
    LV Write Access        read/write
    LV Creation host, time mint, 2013-07-23 09:26:24 +0800
    LV Status              available
    # open                 2
    LV Size                2.00 GiB
    Current LE             512
    Segments               1
    Allocation             inherit
    Read ahead sectors     auto
    - currently set to     256
    Block device           252:1

可以看到, 系统中只有一个logical Volume, 它是`/root`的挂载点. 我们现在要做的就是将这个logical
volume 扩大. 在logical volume扩大时, 会自动使用volume group中的空间.

    chi-VirtualBox ~ # lvextend -L 100G /dev/mint-vg/root
    Extending logical volume root to 100.00 GiB
    Logical volume root successfully resized
    
    chi-VirtualBox ~ # lvdisplay 
    --- Logical volume ---
    LV Path                /dev/mint-vg/root
    LV Name                root
    VG Name                mint-vg
    LV UUID                2hNBQf-1zGs-3nZF-4feA-uR9W-WVDQ-32lLm4
    LV Write Access        read/write
    LV Creation host, time mint, 2013-07-23 09:26:24 +0800
    LV Status              available
    # open                 1
    LV Size                100.00 GiB
    Current LE             25600
    Segments               2
    Allocation             inherit
    Read ahead sectors     auto
    - currently set to     256
    Block device           252:0

    --- Logical volume ---
    LV Path                /dev/mint-vg/swap_1
    LV Name                swap_1
    VG Name                mint-vg
    LV UUID                fD6Mwy-IYEy-ZxXN-aPPW-WNn7-catg-o1bRsX
    LV Write Access        read/write
    LV Creation host, time mint, 2013-07-23 09:26:24 +0800
    LV Status              available
    # open                 2
    LV Size                2.00 GiB
    Current LE             512
    Segments               1
    Allocation             inherit
    Read ahead sectors     auto
    - currently set to     256
    Block device           252:1

可以看到, logical volume 已经成功的变大为了100GB.

### 扩大ext4分区
最后一步, 我们需要把logical volume上的ext4分区也扩大, 这样才能真正的存下文件.

    chi-VirtualBox ~ # resize2fs /dev/mint-vg/root 
    resize2fs 1.42.5 (29-Jul-2012)
    Filesystem at /dev/mint-vg/root is mounted on /; on-line resizing required
    old_desc_blocks = 3, new_desc_blocks = 7
    The filesystem on /dev/mint-vg/root is now 26214400 blocks long.

    chi-VirtualBox ~ # df -h
    Filesystem                 Size  Used Avail Use% Mounted on
    /dev/mapper/mint--vg-root   99G   26G   69G  28% /
    none                       4.0K     0  4.0K   0% /sys/fs/cgroup
    udev                       985M  4.0K  985M   1% /dev
    tmpfs                      201M  896K  200M   1% /run
    none                       5.0M     0  5.0M   0% /run/lock
    none                      1002M   72K 1002M   1% /run/shm
    none                       100M   16K  100M   1% /run/user
    /dev/sda1                  228M   75M  141M  35% /boot

可以看到`/`下面的空间已经成功扩大了.

## 关于lvm的一些知识...
相信看了上面的步骤你已经和我一样晕了...下面我来试图解释一下lvm中的各种术语

- 首先是volume group, 这个你可以想成是扩展分区, 不同的是volume group可以跨多块硬盘多个设备(这是lvm的牛逼之处), 用`vgdisplay`可以查看.
- 然后是physical volume, 这个就是传统意义上的分区, 例如/dev/sda5, 用`pvdisplay`可以查看
- 然后是logical volume, 这个就是lvm中的逻辑分区, 地位相当于逻辑分区. 你硬盘上的目录, 就是挂载在logical volume上的. 用`lvdisplay`可以查看.

### lvm有什么好处?

简单的讲, 就是你可以分出多个物理上的逻辑分区(例如 `/dev/sda1 - /dev/sda5`), 然后把它们放到同一个volume group中, 再从volume group中划分出不同的logical volume, 用于不同目录的挂载. 这样, 相当于在物理上的逻辑分区之上又抽象出了新的逻辑分区, 新的逻辑分区可以任意变化大小, 甚至可以横跨不同的硬盘, 非常灵活.

一句话总结一下: 用fdisk创建physical volume, physical volume 组成 volume group, volume group划分为若干logical volume, 目录挂载在logical volume之上.
