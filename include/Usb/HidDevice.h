#pragma once
#include <QByteArray>
#include <QObject>
#include <libusb.h>
#include <memory>

namespace Usb::Hid {

/*!
 * \brief USB HID device talking over interrupt endpoints (falls back to SET_REPORT
 * on the control endpoint if the interface has no interrupt OUT endpoint).
 */
class Device : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool Connected READ Connected NOTIFY connectedChanged)
    Q_PROPERTY(double Frequency READ Frequency NOTIFY frequencyChanged)
    Q_PROPERTY(QString LastError READ LastError NOTIFY lastErrorChanged)
  public:
    explicit Device(int vid, int pid, QObject *parent = nullptr);
    ~Device() override;

    bool Connected() const;
    double Frequency() const;
    QString LastError() const;

    // By JavaScript and QML convention, any function or constructor name that begins with an uppercase letter is
    // treated as a constructor function
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

  private:
    bool SendReport(const uint8_t* data, const std::size_t size);
    void ReportHanlerLoop();
    void Fail(const QString &msg);

    void FailAsync(const QString &msg);

    class DataImpl;
    using DataImplPointer = std::unique_ptr<DataImpl, void(*)(DataImpl *)>;
    DataImplPointer m_data;
};
} // namespace Usb::Hid