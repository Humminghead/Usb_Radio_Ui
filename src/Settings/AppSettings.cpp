#include "Settings/AppSettings.h"

AppSettings::AppSettings(QObject *parent) : QObject(parent) {}

QVariant AppSettings::value(const QString &key, const QVariant &defaultValue) const {
    QVariant v = m_settings.value(key, defaultValue);
    if (defaultValue.isValid() && v.metaType() != defaultValue.metaType())
        v.convert(defaultValue.metaType());
    return v;
}

void AppSettings::setValue(const QString &key, const QVariant &value) {
    m_settings.setValue(key, value);
    emit valueChanged(key);
}

void AppSettings::reset() {
    m_settings.clear();
    emit valueChanged(QString());
}
