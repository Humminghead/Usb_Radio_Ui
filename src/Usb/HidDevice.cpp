#include "Usb/HidDevice.h"
#include "Usb/Report.h"

#include <QMetaObject>
#include <QtMath>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

namespace Usb::Hid {

// handle is only mutated while m_reader is not running
struct DeviceConnection{
    libusb_context *context{nullptr};
    libusb_device_handle *handle{nullptr};
    int iface{-1};
    int vid{-1};
    int pid{-1};
    uint8_t outEp{0};
    uint8_t inEp{0};
};

class ReportStorage{
public:
    template<typename ... Args>
    auto EmplaceBack(Args&&... args){
        std::lock_guard<std::mutex>g(m_Mutex);
        return m_Queue.emplace_back(std::forward<Args>(args)...);
    }

    auto TryFront() -> std::optional<Usb::Hid::HostCommandReport> {
        std::lock_guard<std::mutex> g(m_Mutex);
        if (m_Queue.empty()) {
            return std::nullopt;
        }
        return m_Queue.front();
    }

    auto AckFront(const uint8_t seqId) {
        std::lock_guard<std::mutex> g(m_Mutex);

        if (m_Queue.empty())
            return false;

        return m_Queue.front().SeqId == seqId ? m_Queue.pop_front(), true : false;
    }

    auto Clear() {
        std::lock_guard<std::mutex> g(m_Mutex);
        return m_Queue.clear();
    }

private:
    mutable std::mutex m_Mutex{};
    std::deque<Usb::Hid::HostCommandReport> m_Queue{};
};

struct Device::DataImpl{
    DeviceConnection m_usb{};
    std::atomic<bool> m_running{false};
    std::thread m_reader;
    std::atomic<double> m_frequencyCurrent{0};
    std::atomic<double> m_frequencyRequested{0};
    QString m_error;    
    uint8_t m_seqId{1};
    ReportStorage m_reports{};
    std::size_t m_consecutiveSendErrors{0};
};

Device::Device(int vid, int pid, QObject *parent) : QObject(parent)
    , m_data{new Device::DataImpl, [](auto * p){ delete p; }}
{
    m_data->m_usb.vid = vid;
    m_data->m_usb.pid = pid;
    libusb_init(&m_data->m_usb.context);
}

Device::~Device() {
    Close();
    if (m_data->m_usb.context)
        libusb_exit(m_data->m_usb.context);
}

bool Device::Connected() const { return m_data->m_usb.handle != nullptr; }

double Device::Frequency() const { return m_data->m_frequencyCurrent.load(); }

QString Device::LastError() const { return m_data->m_error; }

void Device::Fail(const QString &msg) {
    if (m_data->m_error == msg)
        return;
    m_data->m_error = msg;
    emit lastErrorChanged();
}

void Device::FailAsync(const QString &msg) {
    QMetaObject::invokeMethod(this, [this, msg] { Fail(msg); }, Qt::QueuedConnection);
}

bool Device::Open() {
    if (m_data->m_usb.handle) {
        Fail("Device already open");
        return false;
    }

    m_data->m_usb.handle = libusb_open_device_with_vid_pid(m_data->m_usb.context, m_data->m_usb.vid, m_data->m_usb.pid);

    if (!m_data->m_usb.handle) {
        Fail("Device not found");
        return false;
    }

    // Locate HID interface (class 0x03) and its interrupt endpoints
    libusb_config_descriptor *cfg = nullptr;
    libusb_get_active_config_descriptor(libusb_get_device(m_data->m_usb.handle), &cfg);
    m_data->m_usb.iface = -1;
    m_data->m_usb.outEp = m_data->m_usb.inEp = 0;
    if (cfg) {
        for (int i = 0; i < cfg->bNumInterfaces && m_data->m_usb.iface < 0; ++i) {
            const libusb_interface_descriptor &d = cfg->interface[i].altsetting[0];
            if (d.bInterfaceClass != LIBUSB_CLASS_HID)
                continue;
            m_data->m_usb.iface = d.bInterfaceNumber;
            for (int e = 0; e < d.bNumEndpoints; ++e) {
                const auto &ep = d.endpoint[e];
                if ((ep.bmAttributes & 3) != LIBUSB_TRANSFER_TYPE_INTERRUPT)
                    continue;
                (ep.bEndpointAddress & LIBUSB_ENDPOINT_IN ? m_data->m_usb.inEp : m_data->m_usb.outEp) = ep.bEndpointAddress;
            }
        }
        libusb_free_config_descriptor(cfg);
    }
    if (m_data->m_usb.iface < 0) {
        Fail("HID interface not found");
        Close();
        return false;
    }

    libusb_set_auto_detach_kernel_driver(m_data->m_usb.handle, 1); // = detach_kernel_driver
    if (libusb_claim_interface(m_data->m_usb.handle, m_data->m_usb.iface) != 0) {
        Fail("Cannot claim interface (permissions? udev rule?)");
        Close();
        return false;
    }

    if (!m_data->m_error.isEmpty()) { // successful open: no stale error text
        m_data->m_error.clear();
        emit lastErrorChanged();
    }

    m_data->m_consecutiveSendErrors = 0;
    m_data->m_running = true;

    if (m_data->m_usb.inEp)
        m_data->m_reader = std::thread(&Device::ReportHanlerLoop, this);

    emit connectedChanged();
    return true;
}

void Device::Close() {
    m_data->m_running = false;
    if (m_data->m_reader.joinable())
        m_data->m_reader.join();

    m_data->m_reports.Clear();
    m_data->m_seqId = 1;
    m_data->m_consecutiveSendErrors = 0;

    if (m_data->m_usb.handle) {
        libusb_release_interface(m_data->m_usb.handle, m_data->m_usb.iface);
        libusb_close(m_data->m_usb.handle);
        m_data->m_usb.handle = nullptr;
        emit connectedChanged();
    }
}

bool Device::SendReport(const uint8_t* data, const std::size_t size) {
    if (!m_data->m_usb.handle) {
        FailAsync("Not connected");
        return false;
    }
    int rc{-1};
    if (m_data->m_usb.outEp) {
        int n = 0;
        rc = libusb_interrupt_transfer(m_data->m_usb.handle, m_data->m_usb.outEp, const_cast<uint8_t*>(data), size, &n, 1000);
    } else { // SET_REPORT, report type Output (0x02), report ID = data[0]
        rc = libusb_control_transfer(
            m_data->m_usb.handle, 0x21, 0x09, 0x0200 | uint8_t(data[0]), m_data->m_usb.iface, const_cast<uint8_t*>(data), size, 1000);
    }
    if (rc < 0) {
        FailAsync(QString("Write error: %1").arg(libusb_error_name(rc)));
        return false;
    }
    return true;
}

bool Device::SetFrequency(double mhz) {
    if (!m_data->m_usb.handle) {
        FailAsync("Not connected");
        return false;
    }

    // Check that a given value stays in a given range
    mhz = qBound(87.5, qRound(mhz * 10) / 10.0, 108.0);

    m_data->m_frequencyRequested.store(mhz);

    m_data->m_reports.EmplaceBack(HostCommandReport{
        .RawCommnad = CommandId::TuneFreq, .SeqId = m_data->m_seqId++, .Value = quint16(qRound(mhz * 10))});

    return true;
}

bool Device::seek(int dir) {    
    auto tFreq = m_data->m_frequencyRequested.load() + 0.1 * (dir > 0 ? 1 : -1);
    return SetFrequency(tFreq);
}

bool Device::powerUp(){
    if (!m_data->m_usb.handle) {
        FailAsync("Not connected");
        return false;
    }    

    m_data->m_reports.EmplaceBack(
        HostCommandReport{.RawCommnad = CommandId::PowerUp, .SeqId = m_data->m_seqId++, .Value = quint16(875)});

    return true;
}

void Device::ReportHanlerLoop() {
    static auto constexpr bufferSize = std::max(sizeof(DeviceTelemetryReport), sizeof(HostCommandReport));

    std::vector<uint8_t> buf(bufferSize);

    static_assert(sizeof(DeviceTelemetryReport) <= 64, "HID IN report exceeds 64-byte full-speed packet");
    static_assert(sizeof(HostCommandReport) <= 64, "HID OUT report exceeds 64-byte full-speed packet");

    while (m_data->m_running) {
        // Out (to device)
        if (auto report = m_data->m_reports.TryFront()) {
            if (!SendReport(reinterpret_cast<const uint8_t *>(&*report), sizeof(HostCommandReport))) {
                // retransmit next iteration; clear the stall so we can make progress
                if (m_data->m_usb.outEp)
                    libusb_clear_halt(m_data->m_usb.handle, m_data->m_usb.outEp);

                if (++m_data->m_consecutiveSendErrors >= 5) {
                    m_data->m_running = false;
                    QMetaObject::invokeMethod(
                        this,
                        [this] {
                            Fail("Command timed out");
                            Close();
                        },
                        Qt::QueuedConnection);
                    break; // leave the loop now, don't keep running with m_running == false
                }
            } else {
                m_data->m_consecutiveSendErrors = 0;
            }
        }

        // In (from device)
        if (int n = 0, rc = libusb_interrupt_transfer(m_data->m_usb.handle, m_data->m_usb.inEp, buf.data(), sizeof(DeviceTelemetryReport), &n, 200);
            rc == 0 && n > 0) {

            if (auto id = buf.at(0); DEVICE_TELEMETRY_REPORT_ID == id && n==sizeof(DeviceTelemetryReport)){
                const DeviceTelemetryReport *const telemetry{reinterpret_cast<const DeviceTelemetryReport *>(buf.data())};

                if (m_data->m_reports.AckFront(telemetry->SeqId)) {
                    if (auto mhz = static_cast<double>(telemetry->Freq / 10.0);
                        !qFuzzyCompare(mhz, m_data->m_frequencyCurrent)) {
                        m_data->m_frequencyCurrent.store(mhz);
                        emit frequencyChanged();
                    }
                }
            }
            else{
                ///\todo do nothing. maybe print some error in UI
                QMetaObject::invokeMethod(this, [this, ba = QByteArray(reinterpret_cast<char*>(buf.data()),n)] { emit reportReceived(ba); }, Qt::QueuedConnection);
            }

        } else if (rc != 0 && rc != LIBUSB_ERROR_TIMEOUT && rc != LIBUSB_ERROR_NO_DEVICE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // avoid busy-spin on errors
        } else if (rc == LIBUSB_ERROR_NO_DEVICE) {
            m_data->m_running = false;
            QMetaObject::invokeMethod(
                this,
                [this] {
                    Fail("Device unplugged");
                    Close();
                },
                Qt::QueuedConnection);
        }
    }
}
} // namespace Usb::Hid
