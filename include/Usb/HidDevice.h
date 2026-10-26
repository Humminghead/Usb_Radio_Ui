#pragma once
#include <QByteArray>
#include <QObject>
#include <atomic>
#include <libusb.h>
#include <thread>

namespace Usb::Hid {

/*!
 * \brief USB HID device talking over interrupt endpoints (falls back to SET_REPORT
 * on the control endpoint if the interface has no interrupt OUT endpoint).
 */
class Device : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(double frequency READ frequency NOTIFY frequencyChanged)
    Q_PROPERTY(bool powered READ powered NOTIFY powerStateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
  public:
    explicit Device(int vid, int pid, QObject *parent = nullptr);
    ~Device() override;

    bool connected() const { return m_handle != nullptr; }
    double frequency() const { return m_frequency; }
    QString lastError() const { return m_error; }
    bool powered() const { return m_hwIsPowered;}

    Q_INVOKABLE bool open();
    Q_INVOKABLE void close();
    Q_INVOKABLE bool setFrequency(double mhz); // 87.5 .. 108.0
    Q_INVOKABLE bool seek(int direction);      // +1 / -1 (0.1 MHz steps)
    Q_INVOKABLE bool powerUp();

  signals:
    void connectedChanged();
    void frequencyChanged();
    void lastErrorChanged();
    void reportReceived(const QByteArray &data); // raw 7-byte input report
    void powerStateChanged();

  private:
    bool sendReport(const QByteArray &data);
    void readLoop();
    void fail(const QString &msg);

    int m_Vid{-1};
    int m_Pid{-1};
    libusb_context *m_ctx = nullptr;
    libusb_device_handle *m_handle = nullptr;
    int m_iface = -1;
    uint8_t m_outEp = 0, m_inEp = 0;
    std::atomic<bool> m_running{false};
    std::thread m_reader;
    double m_frequency = 90.1;
    QString m_error;
    bool m_hwIsPowered{false};
};
} // namespace Usb::Hid