#pragma once
#include <QAbstractListModel>
#include <QAudioDevice>
#include <memory>

class AppSettings;

// List model of the system's audio outputs (all data lives in C++).
// Roles : name (QString), deviceId (base64 QString), isDefault (bool)
// The user's choice is persisted under "hardware/audioOutput"; while nothing is saved the
// model follows the system default device and re-selects it when devices come and go.
class AudioOutputModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    enum Roles { NameRole = Qt::UserRole + 1, IdRole, IsDefaultRole };

    explicit AudioOutputModel(AppSettings *settings, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentIndex() const;
    void setCurrentIndex(int index);
    int count() const;

    // for the future audio pipeline
    QAudioDevice currentDevice() const;

signals:
    void currentIndexChanged();
    void countChanged();

private:
    void reload();
    void restoreSelection();
    static QString encodeId(const QAudioDevice &d);


    class Impl;
    std::unique_ptr<Impl, void(*)(Impl*)> m_impl;
};
