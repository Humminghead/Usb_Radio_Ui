#pragma once

#include <QObject>
#include <QAudioDevice>
#include <memory>

enum class AudioDeviceRole { Input, Output, Unset };

/*!
 * \brief Loads device list by it role
 * \param List of devices as QList<QAudioDevice>
 */
static inline auto LoadDeviceList(const AudioDeviceRole role) -> QList<QAudioDevice>;

/*!
 * \brief The AudioDeviceList class
 */
class AudioDeviceList : public QObject {
    Q_OBJECT

  public:
    explicit AudioDeviceList(const AudioDeviceRole &role, QObject *parent = nullptr);
    ~AudioDeviceList() = default;

    AudioDeviceList(AudioDeviceList &&l);
    AudioDeviceList *operator=(AudioDeviceList &&l) = delete;

    AudioDeviceList(AudioDeviceList &) = delete;
    AudioDeviceList *operator=(AudioDeviceList &) = delete;

    size_t Size() const;
    const QAudioDevice &At(size_t i) const noexcept;

    auto GetElementId(size_t i) const -> QString;

    auto GetElementDescription(size_t i) const -> QString;

    auto IsDefaultElement(size_t i) const -> bool;

    auto IsEmpty() const noexcept -> bool;

  signals:
    void beginReload();
    void endReload();

  private:
    void Setup();
    void Reload();

    class Impl;
    std::unique_ptr<Impl, void (*)(Impl *)> m_impl;
};
