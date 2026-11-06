#pragma once

#include "Audio/Tools.h"
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>

class AudioDeviceList;

static QByteArray GetDeviceId(const QAudioDevice &device);

class AudioFormatList : public QObject {
    Q_OBJECT

    using DeviceId = QByteArray;
    using FormatList = QList<QAudioFormat>;
    using FormatMap = QMap<DeviceId, FormatList>;
public:

    explicit AudioFormatList(const std::shared_ptr<AudioDeviceList> &list, QObject *object = nullptr);
    ~AudioFormatList() = default;

    AudioFormatList(AudioFormatList &&l) = delete;
    AudioFormatList *operator=(AudioFormatList &&l) = delete;

    AudioFormatList(AudioFormatList &l) = delete;
    AudioFormatList *operator=(AudioFormatList &l) = delete;

    size_t Size() const;

    auto GetElementId(size_t i) const -> QString;

    auto GetElementDescription(size_t i) const -> QString;

    auto IsDefaultElement(size_t i) const -> bool;

    auto IsEmpty() const noexcept -> bool;

    void Reload();

    void SetCurrentDeviceId(const QString &id);
  signals:
    void beginReload();
    void endReload();

  private:
    FormatMap ListSupportedFormats(const QAudioDevice &device);

    QByteArray m_currentDeviceId{};
    FormatMap m_formats;
    std::shared_ptr<AudioDeviceList> m_devices;
};
