#pragma once

#include <string>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include "ofLog.h"

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
        setCtrl(V4L2_CID_AUTO_WHITE_BALANCE,         1);
        setCtrl(V4L2_CID_EXPOSURE_AUTO,              3);
        setCtrl(V4L2_CID_EXPOSURE_ABSOLUTE,          33);
        setCtrl(V4L2_CID_EXPOSURE_AUTO_PRIORITY,     1);
        setCtrl(V4L2_CID_BRIGHTNESS,                 63); 
        setCtrl(V4L2_CID_CONTRAST,                   40);
        setCtrl(V4L2_CID_SATURATION,                 74);
        setCtrl(V4L2_CID_HUE,                        10);
        setCtrl(V4L2_CID_GAMMA,                      128);
        setCtrl(V4L2_CID_SHARPNESS,                  0);
        setCtrl(V4L2_CID_BACKLIGHT_COMPENSATION,     2);
        setCtrl(V4L2_CID_POWER_LINE_FREQUENCY,       2);
        setCtrl(0x00981ae2,                          1);
    }

    __s32 getCtrl(__u32 id) {
        v4l2_control ctrl{};
        ctrl.id = id;
        if (ioctl(fd, VIDIOC_G_CTRL, &ctrl) < 0) return -1;
        return ctrl.value;
    }

    void logCurrentValues() {
        struct { __u32 id; const char* name; } ctrls[] = {
            { V4L2_CID_BRIGHTNESS,                "Brightness"             },
            { V4L2_CID_CONTRAST,                  "Contrast"               },
            { V4L2_CID_SATURATION,                "Saturation"             },
            { V4L2_CID_GAMMA,                     "Gamma"                  },
            { V4L2_CID_SHARPNESS,                 "Sharpness"              },
            { V4L2_CID_WHITE_BALANCE_TEMPERATURE, "WB Temperature"         },
            { V4L2_CID_EXPOSURE_ABSOLUTE,         "Exposure Absolute"      },
        };
        for (auto& c : ctrls)
            ofLogNotice("V4L2CameraSettings") << c.name << " = " << getCtrl(c.id);
    }

private:
    int fd = -1;
};