#include "Audio/AudioFormatList.h"
#include "Audio/AudioDeviceList.h"
#include "Audio/Tools.h"

#include <QAudioDevice>
#include <QAudioFormat>
#include <QDebug>
#include <QSet>
// #include <iterator>

auto AudioFormatList::ListSupportedFormats(const QAudioDevice &device) -> AudioFormatList::FormatMap {
    QAudioFormat prefFormat = device.preferredFormat();
    AudioFormatList::FormatList list{prefFormat};
    return AudioFormatList::FormatMap{{device.id().toBase64(), list}};
}

AudioFormatList::AudioFormatList(const std::shared_ptr<AudioDeviceList> &list, QObject *object) : m_devices{list} {
    Reload();
}

// AudioFormatList::AudioFormatList(AudioFormatList &&l) {Reload(l);}

size_t AudioFormatList::Size() const {
    if (auto currentFormats = m_formats.find(m_currentDeviceId); m_formats.end() == currentFormats)
        return 0;
    else {
        return currentFormats.value().size();
    }
}

auto AudioFormatList::GetElementId(size_t i) const -> QString {
    return EncodeId(m_currentDeviceId);
}

auto AudioFormatList::GetElementDescription(size_t i) const -> QString {
    auto lstIt = m_formats.find(m_currentDeviceId);

    if (lstIt == m_formats.end())
        return {};

    if (i >= lstIt->size()) {
        return {};
    }

    auto format = lstIt->at(i);
    auto rate = format.sampleRate();
    auto channels = format.channelCount();
    auto sampleFormat = format.sampleFormat();

    auto s = QString("%1 Hz, %2 ch, %3").arg(rate).arg(channels).arg(sampleFormat);
    return s;
}

auto AudioFormatList::IsDefaultElement(size_t i) const -> bool {
    return {};
}

auto AudioFormatList::IsEmpty() const noexcept -> bool {
    return {};
}

void AudioFormatList::Reload() {
    emit beginReload();
    for (auto n = 0; n < m_devices->Size(); n++) {
        m_formats.insert(ListSupportedFormats(m_devices->At(n)));
    }
    emit endReload();
}

void AudioFormatList::SetCurrentDeviceId(const QString &id) {
    m_currentDeviceId.clear();

    for (auto c : id) {
        m_currentDeviceId.append(c.toLatin1());
    }
}

QByteArray GetDeviceId(const QAudioDevice &device) {}
