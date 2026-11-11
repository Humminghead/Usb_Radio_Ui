#pragma once

#include <QString>
#include <QAudioDevice>
#include <QByteArray>

static inline QString EncodeId(const QAudioDevice &d);

static QString EncodeId(const QAudioDevice &d) {
    return QString::fromLatin1(d.id().toBase64());      // ids may be binary on some backends
}

static QByteArray EncodeId(const QByteArray &ba) {
    return ba.toBase64();
}