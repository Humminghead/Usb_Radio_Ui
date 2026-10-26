#pragma once

#include <stdint.h>

namespace Usb::Hid {

const uint8_t HOST_CMD_REPORT_ID{2};
const uint8_t DEVICE_STATE_REPORT_ID{1};

struct HostCommandReport {
    uint8_t RawCommnad{0};
    uint8_t SeqId{0};
    uint16_t Freq{0};
    uint8_t Reserved[4]{0, 0, 0, 0};
};

struct DeviceTelemetryReport {
    uint8_t ReportId{0};
    uint8_t SeqId{0};
    uint8_t StatusByte{0};
    uint16_t Freq{0};
    uint8_t Volume{0};
    uint8_t Mute{0};
};
} // namespace Usb::Hid