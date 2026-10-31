#pragma once
#include <QObject>
#include <QSettings>
#include <QVariant>

// Thin QML-friendly wrapper around QSettings.
// Keys use "group/name" (e.g. "audio/volume"). value() converts the stored value to the
// type of the default you pass, so bools/ints come back typed even from an INI file.
class AppSettings : public QObject {
    Q_OBJECT
public:
    explicit AppSettings(QObject *parent = nullptr);

    Q_INVOKABLE QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value);
    Q_INVOKABLE void reset();   // clears everything; emits valueChanged("")

signals:
    void valueChanged(const QString &key);   // key is empty after reset()

private:
    mutable QSettings m_settings;
};
