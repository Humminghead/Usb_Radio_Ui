#include "Audio/AudioModel.h"
#include "Settings/AppSettings.h"

#include <QList>
#include <QMediaDevices>

static const QString kKey = QStringLiteral("hardware/audioOutput");

auto LoadDeviceList(const AudioDeviceRole role) {
    return (role == AudioDeviceRole::Input ? QMediaDevices::audioInputs() : QMediaDevices::audioOutputs());
}

class AudioModel::Impl {
    AppSettings *m_settings;
    QMediaDevices m_devices;
    QList<QAudioDevice> m_devList;
    int m_current = -1;
    AudioDeviceRole m_role{AudioDeviceRole::Unset};

  public:
    explicit Impl(AppSettings *settings, AudioDeviceRole role) : m_settings(settings), m_role(role) {
        m_devList = LoadDeviceList(role);
    };

    auto Devices() const { return &m_devices; }
    auto Settings() { return m_settings; }
    auto GetDeviceList() const { return &m_devList; }
    auto CurrentIndex() const { return m_current; }

    auto SetDeviceList(QList<QAudioDevice> &&outputs) { m_devList = std::move(outputs); }
    auto SetCurrentIndex(int idx) { m_current = idx; }

    auto GetAudioRole() const { return m_role;}
};

AudioModel::AudioModel(AppSettings *settings, AudioDeviceRole role, QObject *parent)
    : QAbstractListModel(parent), m_impl{new AudioModel::Impl(settings, role), [](auto *p) { delete p; }} {

    connect(
        m_impl->Devices(),
        (m_impl->GetAudioRole() == AudioDeviceRole::Input ? &QMediaDevices::audioInputsChanged
                                                          : &QMediaDevices::audioOutputsChanged),
        this,
        &AudioModel::reload);

    // "Reset settings" clears the saved choice -> fall back to the system default
    connect(m_impl->Settings(), &AppSettings::valueChanged, this, [this](const QString &key) {
        if (key.isEmpty()) restoreSelection();
    });
    restoreSelection();
}

int AudioModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_impl->GetDeviceList()->size();
}

QVariant AudioModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_impl->GetDeviceList()->size())
        return {};
    const QAudioDevice &d = m_impl->GetDeviceList()->at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:      return d.description();
    case IdRole:        return encodeId(d);
    case IsDefaultRole: return d.isDefault();
    }
    return {};
}

QHash<int, QByteArray> AudioModel::roleNames() const {
    return { { NameRole, "name" }, { IdRole, "deviceId" }, { IsDefaultRole, "isDefault" } };
}

int AudioModel::currentIndex() const { return m_impl->CurrentIndex(); }

void AudioModel::setCurrentIndex(int index)
{
    m_impl->SetCurrentIndex(index);
}

int AudioModel::count() const { return m_impl->GetDeviceList()->size(); }

QString AudioModel::encodeId(const QAudioDevice &d) {
    return QString::fromLatin1(d.id().toBase64());      // ids may be binary on some backends
}

QAudioDevice AudioModel::currentDevice() const {
    return (m_impl->CurrentIndex() >= 0 && m_impl->CurrentIndex() < m_impl->GetDeviceList()->size()) ? m_impl->GetDeviceList()->at(m_impl->CurrentIndex()) : QAudioDevice();
}

void AudioModel::reload() {
    const int oldCount = m_impl->GetDeviceList()->size();
    beginResetModel();
    m_impl->SetDeviceList(LoadDeviceList(m_impl->GetAudioRole()));
    endResetModel();
    if (oldCount != m_impl->GetDeviceList()->size())
        emit countChanged();
    restoreSelection();
}

void AudioModel::restoreSelection() {
    const QString saved = m_impl->Settings()->value(kKey, QString()).toString();
    int idx = -1;
    if (!saved.isEmpty()) {
        for (int i = 0; i < m_impl->GetDeviceList()->size(); ++i)
            if (encodeId(m_impl->GetDeviceList()->at(i)) == saved) { idx = i; break; }
    }
    if (idx < 0) {                                       // nothing saved / device unplugged
        for (int i = 0; i < m_impl->GetDeviceList()->size(); ++i)
            if (m_impl->GetDeviceList()->at(i).isDefault()) { idx = i; break; }
        if (idx < 0 && !m_impl->GetDeviceList()->isEmpty()) idx = 0;
    }
    m_impl->SetCurrentIndex(idx);
    emit currentIndexChanged();   // always: lets bound ComboBoxes re-sync after a model reset
}
