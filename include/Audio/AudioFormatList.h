#pragma once

#include "Audio/Tools.h"
#include <QObject>
#include <QList>

class AudioFormatList : public QObject {
    Q_OBJECT
  public:
    AudioFormatList(){Reload();}
    ~AudioFormatList() = default;

    AudioFormatList(AudioFormatList &&l) {Reload();}
    AudioFormatList *operator=(AudioFormatList &&l) = delete;

    AudioFormatList(AudioFormatList &l) =delete;
    AudioFormatList *operator=(AudioFormatList &l) = delete;

    size_t Size() const { return test.size(); }

    auto GetElementId(size_t i) const -> QString { return {i == 0 ? "12345" : "7890"}; }

    auto GetElementDescription(size_t i) const -> QString { return {test.at(i)}; }

    auto IsDefaultElement(size_t i) const -> bool { return {}; }

    auto IsEmpty() const noexcept -> bool { return {}; }
signals:
    void beginReload();
    void endReload();

private:
    void Reload(){
        emit beginReload();

        emit endReload();
    }
    QList<QString> test{"First","Second"};
};
