#pragma once

#include <QString>
#include <QAudioDevice>

static inline QString EncodeId(const QAudioDevice &d);

QString EncodeId(const QAudioDevice &d) {
    return QString::fromLatin1(d.id().toBase64());      // ids may be binary on some backends
}
