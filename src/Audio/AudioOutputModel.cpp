#include "Audio/AudioOutputModel.h"
#include "Settings/AppSettings.h"

#include <QList>
#include <QMediaDevices>

static const QString kKey = QStringLiteral("hardware/audioOutput");

class AudioOutputModel::Impl{
    AppSettings *m_settings;
    QMediaDevices m_devices;
    QList<QAudioDevice> m_outputs;
    int m_current = -1;
public:
    explicit Impl(AppSettings *settings) : m_settings(settings) { m_outputs = QMediaDevices::audioOutputs(); };

    auto Devices() const { return &m_devices; }
    auto Settings() { return m_settings; }
    auto Outputs() const { return &m_outputs; }
    auto CurrentIndex() const { return m_current; }

    auto SetOutputs(QList<QAudioDevice> &&outputs) { m_outputs = std::move(outputs); }
    auto SetCurrentIndex(int idx) { m_current = idx; }
};

AudioOutputModel::AudioOutputModel(AppSettings *settings, QObject *parent)
    : QAbstractListModel(parent), m_impl{new AudioOutputModel::Impl(settings),[](auto *p){delete p;}} {

    connect(m_impl->Devices(), &QMediaDevices::audioOutputsChanged, this, &AudioOutputModel::reload);

    // "Reset settings" clears the saved choice -> fall back to the system default
    connect(m_impl->Settings(), &AppSettings::valueChanged, this, [this](const QString &key) {
        if (key.isEmpty()) restoreSelection();
    });
    restoreSelection();
}

int AudioOutputModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_impl->Outputs()->size();
}

QVariant AudioOutputModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_impl->Outputs()->size())
        return {};
    const QAudioDevice &d = m_impl->Outputs()->at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:      return d.description();
    case IdRole:        return encodeId(d);
    case IsDefaultRole: return d.isDefault();
    }
    return {};
}

QHash<int, QByteArray> AudioOutputModel::roleNames() const {
    return { { NameRole, "name" }, { IdRole, "deviceId" }, { IsDefaultRole, "isDefault" } };
}

int AudioOutputModel::currentIndex() const { return m_impl->CurrentIndex(); }

void AudioOutputModel::setCurrentIndex(int index)
{
    m_impl->SetCurrentIndex(index);
}

int AudioOutputModel::count() const { return m_impl->Outputs()->size(); }

QString AudioOutputModel::encodeId(const QAudioDevice &d) {
    return QString::fromLatin1(d.id().toBase64());      // ids may be binary on some backends
}

QAudioDevice AudioOutputModel::currentDevice() const {
    return (m_impl->CurrentIndex() >= 0 && m_impl->CurrentIndex() < m_impl->Outputs()->size()) ? m_impl->Outputs()->at(m_impl->CurrentIndex()) : QAudioDevice();
}

void AudioOutputModel::reload() {
    const int oldCount = m_impl->Outputs()->size();
    beginResetModel();
    m_impl->SetOutputs(QMediaDevices::audioOutputs());
    endResetModel();
    if (oldCount != m_impl->Outputs()->size())
        emit countChanged();
    restoreSelection();
}

void AudioOutputModel::restoreSelection() {
    const QString saved = m_impl->Settings()->value(kKey, QString()).toString();
    int idx = -1;
    if (!saved.isEmpty()) {
        for (int i = 0; i < m_impl->Outputs()->size(); ++i)
            if (encodeId(m_impl->Outputs()->at(i)) == saved) { idx = i; break; }
    }
    if (idx < 0) {                                       // nothing saved / device unplugged
        for (int i = 0; i < m_impl->Outputs()->size(); ++i)
            if (m_impl->Outputs()->at(i).isDefault()) { idx = i; break; }
        if (idx < 0 && !m_impl->Outputs()->isEmpty()) idx = 0;
    }
    m_impl->SetCurrentIndex(idx);
    emit currentIndexChanged();   // always: lets bound ComboBoxes re-sync after a model reset
}
