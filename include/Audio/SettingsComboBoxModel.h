#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QVariant>
#include <memory>

class ComboBoxSettings;


// List model of the system's audio outputs (all data lives in C++).
// Roles : name (QString), deviceId (base64 QString), isDefault (bool)
// The user's choice is persisted under "hardware/audioOutput"; while nothing is saved the
// model follows the system default device and re-selects it when devices come and go.

class SettingsComboBoxModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    enum Roles { NameRole = Qt::UserRole + 1, IdRole, IsDefaultRole };

    template <class DataSourcePtr>
    explicit SettingsComboBoxModel(std::shared_ptr<ComboBoxSettings> settings, std::shared_ptr<DataSourcePtr>, QObject *parent = nullptr);

    template <typename T>
    SettingsComboBoxModel(std::shared_ptr<ComboBoxSettings> settings, T &&, QObject *parent = nullptr) {
        static_assert(false, "Type doesn't supported");
    }

    ~SettingsComboBoxModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentIndex() const;
    void setCurrentIndex(int index);
    int count() const;

    // for the future audio pipeline
    // QAudioDevice currentDevice() const;
    // QAudioFormat preferredDeviceFormat() const;

    void Reset();

signals:
    void currentIndexChanged();
    void countChanged();

private:    
    void restoreSelection();

    class ImplBase;
    template <class T> class Impl;
    std::unique_ptr<ImplBase> m_impl;
};



