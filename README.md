kernel source for Micromax Canvas Express 2
=========================================
Basic   | Spec Sheet
-------:|:-------------------------
CPU     | 1.4GHz Octa-Core MT6592
GPU     | Mali - 450 MP4
Memory  | 1GB RAM
Shipped Android Version | 5.1
Storage | 8GB
Battery | 2500 mAh
Display | 5" 720 x 1280 px
Camera  | 13MPx + 2Mpx, LED Flash

![Micromax](https://images-na.ssl-images-amazon.com/images/I/51tSpbcB1xL.jpg "Micromax Canvas Express 2")


cd ~/kernel_longwei_E313

mkdir out

make ARCH=arm ARCH_MTK_PLATFORM=mt6592 O=out longwei_defconfig

make ARCH=arm ARCH_MTK_PLATFORM=mt6592 O=out


