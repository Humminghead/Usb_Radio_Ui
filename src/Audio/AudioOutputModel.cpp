#include "Audio/AudioOutputModel.h"
#include "Settings/AppSettings.h"

static const QString kKey = QStringLiteral("hardware/audioOutput");

AudioOutputModel::AudioOutputModel(AppSettings *settings, QObject *parent)
    : QAbstractListModel(parent), m_settings(settings) {
    connect(&m_devices, &QMediaDevices::audioOutputsChanged, this, &AudioOutputModel::reload);
    // "Reset settings" clears the saved choice -> fall back to the system default
    connect(m_settings, &AppSettings::valueChanged, this, [this](const QString &key) {
        if (key.isEmpty()) restoreSelection();
    });
    m_outputs = QMediaDevices::audioOutputs();
    restoreSelection();
}

int AudioOutputModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_outputs.size();
}

QVariant AudioOutputModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_outputs.size())
        return {};
    const QAudioDevice &d = m_outputs.at(index.row());
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

QString AudioOutputModel::encodeId(const QAudioDevice &d) {
    return QString::fromLatin1(d.id().toBase64());      // ids may be binary on some backends
}

QAudioDevice AudioOutputModel::currentDevice() const {
    return (m_current >= 0 && m_current < m_outputs.size()) ? m_outputs.at(m_current) : QAudioDevice();
}

void AudioOutputModel::setCurrentIndex(int index) {
    if (index < 0 || index >= m_outputs.size() || index == m_current)
        return;
    m_current = index;
    m_settings->setValue(kKey, encodeId(m_outputs.at(index)));
    emit currentIndexChanged();
}

void AudioOutputModel::reload() {
    const int oldCount = m_outputs.size();
    beginResetModel();
    m_outputs = QMediaDevices::audioOutputs();
    endResetModel();
    if (oldCount != m_outputs.size())
        emit countChanged();
    restoreSelection();
}

void AudioOutputModel::restoreSelection() {
    const QString saved = m_settings->value(kKey, QString()).toString();
    int idx = -1;
    if (!saved.isEmpty()) {
        for (int i = 0; i < m_outputs.size(); ++i)
            if (encodeId(m_outputs.at(i)) == saved) { idx = i; break; }
    }
    if (idx < 0) {                                       // nothing saved / device unplugged
        for (int i = 0; i < m_outputs.size(); ++i)
            if (m_outputs.at(i).isDefault()) { idx = i; break; }
        if (idx < 0 && !m_outputs.isEmpty()) idx = 0;
    }
    m_current = idx;
    emit currentIndexChanged();   // always: lets bound ComboBoxes re-sync after a model reset
}
