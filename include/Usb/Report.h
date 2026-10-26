#pragma once

#include <stdint.h>

namespace Usb::Hid {

const uint8_t HOST_CMD_REPORT_ID{2};
const uint8_t DEVICE_STATE_REPORT_ID{1};

enum class CommandId : uint8_t {
    Ping = 0x00,
    PowerUp = 0x01,
    PowerDown = 0x02,
    TuneFreq = 0x03,
    GetSignalQuality = 0x04,
    SetVolume = 0x05,
    SeekUp = 0x06,
    SeekDown = 0x07,
    Mute = 0x08,
    Unknown = 0xFF,
};

#pragma pack(push, 1)
struct HostCommandReport {
    uint8_t ReportId{HOST_CMD_REPORT_ID};
    CommandId RawCommnad{0};
    uint8_t SeqId{0};
    uint16_t Value{0};
    uint8_t Reserved[3]{0, 0, 0};
};

struct DeviceTelemetryReport {
    uint8_t ReportId{DEVICE_STATE_REPORT_ID};
    uint8_t SeqId{0};
    uint8_t StatusByte{0};
    uint16_t Freq{0};
    uint8_t Volume{0};
    uint8_t Mute{0};
};
#pragma pack(pop)

} // namespace Usb::Hid