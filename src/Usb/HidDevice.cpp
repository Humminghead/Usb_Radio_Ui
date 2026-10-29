#include "Usb/HidDevice.h"
#include "Usb/Report.h"

#include <QMetaObject>
#include <QtMath>
#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <thread>

namespace Usb::Hid {

struct DeviceConnection{
    libusb_context *context{nullptr};
    libusb_device_handle *handle{nullptr};
    int iface{-1};
    int vid{-1};
    int pid{-1};
    uint8_t outEp{0};
    uint8_t inEp{0};
};

struct Device::DataImpl{
    DeviceConnection usb{};
    std::atomic<bool> m_running{false};
    std::thread m_reader;
    double m_frequencyCache{0};
    QString m_error;
    bool m_hwIsPowered{false};
    uint8_t m_SeqId{0};
    std::mutex m_ReportQueueMutex{};
    std::queue<Usb::Hid::HostCommandReport> m_ReportQueue{};
};

Device::Device(int vid, int pid, QObject *parent) : QObject(parent)
    , m_Data{new Device::DataImpl, [](auto * p){ delete p; }}
{
    m_Data->usb.vid = vid;
    m_Data->usb.pid = pid;
    libusb_init(&m_Data->usb.context);
}

Device::~Device() {
    close();
    if (m_Data->usb.context)
        libusb_exit(m_Data->usb.context);
}

bool Device::connected() const { return m_Data->usb.handle != nullptr; }

double Device::frequency() const { return m_Data->m_frequencyCache; }

QString Device::lastError() const { return m_Data->m_error; }

bool Device::powered() const { return m_Data->m_hwIsPowered;}

void Device::fail(const QString &msg) {
    if (m_Data->m_error == msg)
        return;
    m_Data->m_error = msg;
    emit lastErrorChanged();
}

bool Device::open() {
    m_Data->usb.handle = libusb_open_device_with_vid_pid(m_Data->usb.context, m_Data->usb.vid, m_Data->usb.pid);
    if (!m_Data->usb.handle) {
        fail("Device not found");
        return false;
    }

    // Locate HID interface (class 0x03) and its interrupt endpoints
    libusb_config_descriptor *cfg = nullptr;
    libusb_get_active_config_descriptor(libusb_get_device(m_Data->usb.handle), &cfg);
    m_Data->usb.iface = -1;
    m_Data->usb.outEp = m_Data->usb.inEp = 0;
    if (cfg) {
        for (int i = 0; i < cfg->bNumInterfaces && m_Data->usb.iface < 0; ++i) {
            const libusb_interface_descriptor &d = cfg->interface[i].altsetting[0];
            if (d.bInterfaceClass != LIBUSB_CLASS_HID)
                continue;
            m_Data->usb.iface = d.bInterfaceNumber;
            for (int e = 0; e < d.bNumEndpoints; ++e) {
                const auto &ep = d.endpoint[e];
                if ((ep.bmAttributes & 3) != LIBUSB_TRANSFER_TYPE_INTERRUPT)
                    continue;
                (ep.bEndpointAddress & LIBUSB_ENDPOINT_IN ? m_Data->usb.inEp : m_Data->usb.outEp) = ep.bEndpointAddress;
            }
        }
        libusb_free_config_descriptor(cfg);
    }
    if (m_Data->usb.iface < 0) {
        fail("HID interface not found");
        close();
        return false;
    }

    libusb_set_auto_detach_kernel_driver(m_Data->usb.handle, 1); // = detach_kernel_driver
    if (libusb_claim_interface(m_Data->usb.handle, m_Data->usb.iface) != 0) {
        fail("Cannot claim interface (permissions? udev rule?)");
        close();
        return false;
    }

    if (!m_Data->m_error.isEmpty()) { // successful open: no stale error text
        m_Data->m_error.clear();
        emit lastErrorChanged();
    }
    m_Data->m_running = true;
    if (m_Data->usb.inEp)
        m_Data->m_reader = std::thread(&Device::ReportHanlerLoop, this);
    emit connectedChanged();
    return true;
}

void Device::close() {
    m_Data->m_running = false;
    if (m_Data->m_reader.joinable())
        m_Data->m_reader.join();
    if (m_Data->usb.handle) {
        libusb_release_interface(m_Data->usb.handle, m_Data->usb.iface);
        libusb_close(m_Data->usb.handle);
        m_Data->usb.handle = nullptr;
        emit connectedChanged();
    }
}

bool Device::sendReport(const QByteArray &data) {
    if (!m_Data->usb.handle) {
        fail("Not connected");
        return false;
    }
    int rc;
    if (m_Data->usb.outEp) {
        int n = 0;
        rc = libusb_interrupt_transfer(m_Data->usb.handle, m_Data->usb.outEp, (uint8_t *)data.data(), data.size(), &n, 1000);
    } else { // SET_REPORT, report type Output (0x02), report ID = data[0]
        rc = libusb_control_transfer(
            m_Data->usb.handle, 0x21, 0x09, 0x0200 | uint8_t(data[0]), m_Data->usb.iface, (uint8_t *)data.data(), data.size(), 1000);
    }
    if (rc < 0) {
        fail(QString("Write error: %1").arg(libusb_error_name(rc)));
        return false;
    }
    return true;
}

bool Device::setFrequency(double mhz) {
    if (!m_Data->usb.handle) {
        fail("Not connected");
        return false;
    }

    // Check that a given value stays in a given range
    mhz = qBound(87.5, qRound(mhz * 10) / 10.0, 108.0);

    {
        std::lock_guard g{m_Data->m_ReportQueueMutex};
        m_Data->m_ReportQueue.emplace(HostCommandReport{.RawCommnad = CommandId::TuneFreq, .SeqId = m_Data->m_SeqId++, .Value = quint16(qRound(mhz * 10))});
    }

    return true;
}

bool Device::seek(int dir) {
    return setFrequency(m_Data->m_frequencyCache + 0.1 * (dir > 0 ? 1 : -1));
}

bool Device::powerUp(){
    if (!m_Data->usb.handle) {
        fail("Not connected");
        return false;
    }

    static_assert(std::is_nothrow_move_constructible_v<HostCommandReport>);

    {
        std::lock_guard g{m_Data->m_ReportQueueMutex};
        m_Data->m_ReportQueue.emplace(HostCommandReport {.RawCommnad = CommandId::PowerUp, .SeqId = m_Data->m_SeqId++, .Value = quint16(875)});
    }

    return true;
}

void Device::ReportHanlerLoop() {
    uint8_t buf[64];
    while (m_Data->m_running) {

        // Out (to device)
        if(!m_Data->m_ReportQueue.empty()){
            std::lock_guard g{m_Data->m_ReportQueueMutex};
            auto &report = m_Data->m_ReportQueue.front();
            sendReport(QByteArray(reinterpret_cast<char *>(&report), sizeof(HostCommandReport)));
            m_Data->m_ReportQueue.pop();
        }

        //In (from device)
        if (int n = 0, rc = libusb_interrupt_transfer(m_Data->usb.handle, m_Data->usb.inEp, buf, 7, &n, 200);
            rc == 0 && n > 0) {
            QByteArray ba(reinterpret_cast<char *>(buf), n);

            if (auto id = ba.at(0); DEVICE_TELEMETRY_REPORT_ID == id && n==sizeof(DeviceTelemetryReport)){
                const DeviceTelemetryReport *const telemetry{reinterpret_cast<DeviceTelemetryReport *const>(ba.data())};

                if (auto mhz = static_cast<double>(telemetry->Freq/10.0); !qFuzzyCompare(mhz, m_Data->m_frequencyCache)) {
                        m_Data->m_frequencyCache = mhz;
                        emit frequencyChanged();
                }

            }
            else{
                ///\todo do nothing. maybe print some error in UI
                QMetaObject::invokeMethod(this, [this, ba] { emit reportReceived(ba); }, Qt::QueuedConnection);
            }

        } else if (rc != 0 && rc != LIBUSB_ERROR_TIMEOUT && rc != LIBUSB_ERROR_NO_DEVICE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // avoid busy-spin on errors
        } else if (rc == LIBUSB_ERROR_NO_DEVICE) {
            m_Data->m_running = false;
            QMetaObject::invokeMethod(
                this,
                [this] {
                    fail("Device unplugged");
                    close();
                },
                Qt::QueuedConnection);
        }
    }
}
} // namespace Usb::Hid
