#bin/bash

sudo cp ../systemd/x86_ubuntu_h264_decode.service /etc/systemd/system/h264_decode.service

sudo cp ../systemd/x86_ubuntu_h265_decode.service /etc/systemd/system/h265_decode.service

sudo cp ../systemd/x86_ubuntu_mjpeg_decode.service /etc/systemd/system/mjpeg_decode.service


# make sure they are not enabled / not running (qopenhd enables / starts them if needed)
sudo systemctl stop h264_decode
#sudo systemctl disable h264_decode

sudo systemctl stop h265_decode
#sudo systemctl disable h265_decode

sudo systemctl stop mjpeg_decode
#sudo systemctl disable mjpeg_decode


