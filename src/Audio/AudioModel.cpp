#include "Audio/AudioModel.h"
#include "Audio/AudioDevice.h"
#include "Settings/AppSettings.h"

#include <QList>
#include <QMediaDevices>
#include <type_traits>

static const QString kKey = QStringLiteral("hardware/audioOutput");

template <class T>
void isT(T&&);

class SettingsComboBoxModel::ImplBase {
public:
    virtual ~ImplBase() = default;

    virtual ComboBoxSettings* Settings() = 0;
    virtual size_t GetCurrentIndex() const =0;
    virtual size_t ElementCount() const = 0;
    virtual void SetCurrentIndex(int idx) = 0;
    virtual QString ElementId(size_t i) const = 0;
    virtual QString ElementDescription(size_t i) const =0;
    virtual bool IsDefaultElement(size_t i) const = 0;
    virtual bool IsEmpty() const noexcept = 0;
    virtual void* DataRaw() noexcept = 0;
};

template <class T> struct SettingsComboBoxModel::Impl : public SettingsComboBoxModel::ImplBase {
    std::shared_ptr<ComboBoxSettings> m_settings;
    int m_current = -1;
    T m_data;

  public:
    template <class U>
    Impl(std::shared_ptr<ComboBoxSettings> settings, U &&list) : m_settings(settings), m_data(std::forward<U>(list)) {}

    auto Settings() -> ComboBoxSettings * override { return m_settings.get(); }

    auto GetCurrentIndex() const -> size_t override { return m_current; }

    auto SetCurrentIndex(int idx) -> void override { m_current = idx; }

    auto ElementCount() const -> size_t override { return m_data.Size(); }

    auto ElementId(size_t i) const -> QString override { return m_data.GetElementId(i); }

    auto ElementDescription(size_t i) const -> QString override { return m_data.GetElementDescription(i); }

    auto IsDefaultElement(size_t i) const -> bool override { return m_data.IsDefaultElement(i); }

    auto IsEmpty() const noexcept -> bool override { return m_data.IsEmpty(); }

    void* DataRaw() noexcept override { return reinterpret_cast<void*>(&m_data); }
};

template <>
SettingsComboBoxModel::SettingsComboBoxModel(std::shared_ptr<ComboBoxSettings> settings,
                                             AudioDeviceList&& list,
                                             QObject *parent)
    : QAbstractListModel(parent)
    , m_impl(std::make_unique<Impl<std::remove_cv_t<std::remove_reference_t<decltype(list)>>>>(
          settings, std::forward<AudioDeviceList>(list)))
{
    // Setting up the connections after the move operation has been made
    if (auto storage = static_cast<AudioDeviceList *>(m_impl->DataRaw()); storage != nullptr) {
        connect(storage, &AudioDeviceList::beginReload, [this]() {
            beginResetModel();
        });

        connect(storage, &AudioDeviceList::endReload, [this]() {
            restoreSelection();
            endResetModel();
            emit countChanged();
        });
    }

    // "Reset settings" clears the saved choice -> fall back to the system default
    connect(m_impl->Settings(), &ComboBoxSettings::valueChanged, this, [this](const QString &key) {
        if (key.isEmpty()) restoreSelection();
    });
    restoreSelection();
}

SettingsComboBoxModel::~SettingsComboBoxModel() = default;

int SettingsComboBoxModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_impl->ElementCount();
}

QVariant SettingsComboBoxModel::data(const QModelIndex &index, int role) const {
    if (auto size = m_impl->ElementCount(); !index.isValid() || index.row() < 0 || index.row() >= size)
        return {};
    const auto row = index.row();
    switch (role) {
        case Qt::DisplayRole:
        case NameRole:
            return m_impl->ElementDescription(row);
        case IdRole:
            return m_impl->ElementId(row);
        case IsDefaultRole:
            return m_impl->IsDefaultElement(row);
    }
    return {};
}

QHash<int, QByteArray> SettingsComboBoxModel::roleNames() const {
    return { { NameRole, "name" }, { IdRole, "deviceId" }, { IsDefaultRole, "isDefault" } };
}

int SettingsComboBoxModel::currentIndex() const { return m_impl->GetCurrentIndex(); }

void SettingsComboBoxModel::setCurrentIndex(int index)
{
    m_impl->SetCurrentIndex(index);
}

int SettingsComboBoxModel::count() const { return m_impl->ElementCount(); }

/*
QAudioDevice SettingsComboBoxModel<AudioDevice>currentDevice() const {
    return (m_impl->CurrentIndex() >= 0 && m_impl->CurrentIndex() < m_impl->GetDeviceList()->size()) ? m_impl->GetDeviceList()->at(m_impl->CurrentIndex()) : QAudioDevice();
}

QAudioFormat SettingsComboBoxModel<AudioDevice>preferredDeviceFormat() const {
    return currentDevice().preferredFormat();
}
*/

void SettingsComboBoxModel::restoreSelection() {
    const QString saved = m_impl->Settings()->value(kKey, QString()).toString();
    int idx = -1;

    for (int i = 0, sz = m_impl->ElementCount(); i < sz; ++i) {
        if (!saved.isEmpty()) {
            if (m_impl->ElementId(i) == saved) {
                idx = i;
                break;
            }
        }
        if (idx < 0) { // nothing saved / device unplugged
            if (m_impl->IsDefaultElement(i)) {
                idx = i;
                break;
            }
        }
    }

    if (idx < 0 && !m_impl->IsEmpty())
        idx = 0;

    m_impl->SetCurrentIndex(idx);
    emit currentIndexChanged(); // always: lets bound ComboBoxes re-sync after a model reset
}
