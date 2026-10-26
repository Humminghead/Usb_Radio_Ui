#include "Usb/HidDevice.h"
#include "Usb/Report.h"

#include <QMetaObject>
#include <QtMath>
#include <chrono>
namespace Usb::Hid {
Device::Device(int vid, int pid, QObject *parent) : QObject(parent), m_Vid{vid}, m_Pid{pid} {
    libusb_init(&m_ctx);
}
Device::~Device() {
    close();
    if (m_ctx)
        libusb_exit(m_ctx);
}

void Device::fail(const QString &msg) {
    if (m_error == msg)
        return;
    m_error = msg;
    emit lastErrorChanged();
}

bool Device::open() {
    m_handle = libusb_open_device_with_vid_pid(m_ctx, m_Vid, m_Pid);
    if (!m_handle) {
        fail("Device not found");
        return false;
    }

    // Locate HID interface (class 0x03) and its interrupt endpoints
    libusb_config_descriptor *cfg = nullptr;
    libusb_get_active_config_descriptor(libusb_get_device(m_handle), &cfg);
    m_iface = -1;
    m_outEp = m_inEp = 0;
    if (cfg) {
        for (int i = 0; i < cfg->bNumInterfaces && m_iface < 0; ++i) {
            const libusb_interface_descriptor &d = cfg->interface[i].altsetting[0];
            if (d.bInterfaceClass != LIBUSB_CLASS_HID)
                continue;
            m_iface = d.bInterfaceNumber;
            for (int e = 0; e < d.bNumEndpoints; ++e) {
                const auto &ep = d.endpoint[e];
                if ((ep.bmAttributes & 3) != LIBUSB_TRANSFER_TYPE_INTERRUPT)
                    continue;
                (ep.bEndpointAddress & LIBUSB_ENDPOINT_IN ? m_inEp : m_outEp) = ep.bEndpointAddress;
            }
        }
        libusb_free_config_descriptor(cfg);
    }
    if (m_iface < 0) {
        fail("HID interface not found");
        close();
        return false;
    }

    libusb_set_auto_detach_kernel_driver(m_handle, 1); // = detach_kernel_driver
    if (libusb_claim_interface(m_handle, m_iface) != 0) {
        fail("Cannot claim interface (permissions? udev rule?)");
        close();
        return false;
    }

    if (!m_error.isEmpty()) { // successful open: no stale error text
        m_error.clear();
        emit lastErrorChanged();
    }
    m_running = true;
    if (m_inEp)
        m_reader = std::thread(&Device::readLoop, this);
    emit connectedChanged();
    return true;
}

void Device::close() {
    m_running = false;
    if (m_reader.joinable())
        m_reader.join();
    if (m_handle) {
        libusb_release_interface(m_handle, m_iface);
        libusb_close(m_handle);
        m_handle = nullptr;
        emit connectedChanged();
    }
}

bool Device::sendReport(const QByteArray &data) {
    if (!m_handle) {
        fail("Not connected");
        return false;
    }
    int rc;
    if (m_outEp) {
        int n = 0;
        rc = libusb_interrupt_transfer(m_handle, m_outEp, (uint8_t *)data.data(), data.size(), &n, 1000);
    } else { // SET_REPORT, report type Output (0x02), report ID = data[0]
        rc = libusb_control_transfer(
            m_handle, 0x21, 0x09, 0x0200 | uint8_t(data[0]), m_iface, (uint8_t *)data.data(), data.size(), 1000);
    }
    if (rc < 0) {
        fail(QString("Write error: %1").arg(libusb_error_name(rc)));
        return false;
    }
    return true;
}

bool Device::setFrequency(double mhz) {
    mhz = qBound(87.5, qRound(mhz * 10) / 10.0, 108.0);
    // Output report: ID 2, command 1, uint16 LE freq in 100 kHz units, 4 pad bytes
    const quint16 f = quint16(qRound(mhz * 10));
    QByteArray r(8, 0);
    r[0] = 0x02;
    r[1] = 0x01;
    r[2] = char(f & 0xff);
    r[3] = char(f >> 8);
    if (!sendReport(r))
        return false;
    if (!qFuzzyCompare(mhz, m_frequency)) {
        m_frequency = mhz;
        emit frequencyChanged();
    }
    return true;
}

bool Device::seek(int dir) {
    return setFrequency(m_frequency + 0.1 * (dir > 0 ? 1 : -1));
}

void Device::readLoop() {
    uint8_t buf[64];
    while (m_running) {
        int n = 0;
        int rc = libusb_interrupt_transfer(m_handle, m_inEp, buf, 7, &n, 200);
        if (rc == 0 && n > 0) {
            QByteArray ba(reinterpret_cast<char *>(buf), n);
            QMetaObject::invokeMethod(this, [this, ba] { emit reportReceived(ba); }, Qt::QueuedConnection);
        } else if (rc != 0 && rc != LIBUSB_ERROR_TIMEOUT && rc != LIBUSB_ERROR_NO_DEVICE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // avoid busy-spin on errors
        } else if (rc == LIBUSB_ERROR_NO_DEVICE) {
            m_running = false;
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