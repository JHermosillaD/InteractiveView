#pragma once
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

//========================================================================
class V4L2CameraSettings {
public:

    bool open(const std::string& device = "/dev/video0") {
        fd = ::open(device.c_str(), O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            return false;
        }
        return true;
    }

    void close() {
        if (fd >= 0) { ::close(fd); fd = -1; }
    }

    bool setCtrl(__u32 id, __s32 value) {
        v4l2_control ctrl{};
        ctrl.id    = id;
        ctrl.value = value;

        if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
            return false;
        }
        return true;
    }

    void applyProfile() {
        setCtrl(V4L2_CID_AUTO_WHITE_BALANCE,        1);
        setCtrl(V4L2_CID_EXPOSURE_AUTO,             3);
        setCtrl(V4L2_CID_WHITE_BALANCE_TEMPERATURE, 4354);
        setCtrl(V4L2_CID_EXPOSURE_AUTO_PRIORITY,    1);
        setCtrl(V4L2_CID_BRIGHTNESS,                128); 
        setCtrl(V4L2_CID_CONTRAST,                  37);
        setCtrl(V4L2_CID_SATURATION,                65);
        setCtrl(V4L2_CID_HUE,                       0);
        setCtrl(V4L2_CID_GAMMA,                     120);
        setCtrl(V4L2_CID_SHARPNESS,                 7);
        setCtrl(V4L2_CID_BACKLIGHT_COMPENSATION,    1);
        setCtrl(V4L2_CID_POWER_LINE_FREQUENCY,      2);
        setCtrl(0x00981ae2,                         1);
    }

    __s32 getCtrl(__u32 id) {
        v4l2_control ctrl{};
        ctrl.id = id;
        if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) return -1;
        return ctrl.value;
    }

private:
    int fd = -1;
};