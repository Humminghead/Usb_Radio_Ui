#include "Audio/AudioDeviceList.h"
#include "Audio/Tools.h"
#include <QList>
#include <QMediaDevices>
#include <qdebug.h>

auto LoadDeviceList(const AudioDeviceRole role) -> QList<QAudioDevice> {
    return (role == AudioDeviceRole::Input ? QMediaDevices::audioInputs() : QMediaDevices::audioOutputs());
}

auto connectDevicesByRole = [](const QMediaDevices *devices, const AudioDeviceRole &role, auto *object, auto slot) {
    return QObject::connect(
        devices,
        (role == AudioDeviceRole::Input ? &QMediaDevices::audioInputsChanged : &QMediaDevices::audioOutputsChanged),
        object,
        slot);
};

class AudioDeviceList::Impl {
    AudioDeviceRole m_role{AudioDeviceRole::Unset};
    QMediaDevices m_devices;
    QList<QAudioDevice> m_devList;

  public:
    explicit Impl(const AudioDeviceRole role) : m_role(role) { m_devList = LoadDeviceList(m_role); }

    auto GetAudioRole() const { return m_role; }

    auto GetDeviceList() const { return &m_devList; }

    auto GetDeviceListCount() const { return m_devList.count(); }

    const QAudioDevice &GetDeviceByIndex(size_t i) const noexcept { return m_devList.at(i); }

    auto IsEmpty() const noexcept { return m_devList.isEmpty(); }

    auto SetDeviceList(QList<QAudioDevice> &&outputs) { m_devList = std::move(outputs); }

    const QMediaDevices *Devices() const noexcept { return &m_devices; }
};

AudioDeviceList::AudioDeviceList(const AudioDeviceRole &role, QObject *parent)
    : QObject(parent), m_impl(new Impl(role), [](auto *p) { delete p; }) {
    Setup();
}

size_t AudioDeviceList::Size() const {
    return m_impl->GetDeviceListCount();
}

const QAudioDevice &AudioDeviceList::At(size_t i) const noexcept {
    return m_impl->GetDeviceByIndex(i);
}

auto AudioDeviceList::GetElementId(size_t i) const -> QString {
    auto dev = At(i);
    return EncodeId(dev);
}

auto AudioDeviceList::GetElementDescription(size_t i) const -> QString {
    return At(i).description();
}

auto AudioDeviceList::IsDefaultElement(size_t i) const -> bool {
    return At(i).isDefault();
}

auto AudioDeviceList::IsEmpty() const noexcept -> bool {
    return m_impl->IsEmpty();
}

void AudioDeviceList::Setup() {
    connectDevicesByRole(m_impl->Devices(), m_impl->GetAudioRole(), this, &AudioDeviceList::Reload);
}

void AudioDeviceList::Reload() {
    emit beginReload();
    m_impl->SetDeviceList(LoadDeviceList(m_impl->GetAudioRole()));
    emit endReload();
}
