#include "Tray/TrayController.h"

#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QPixmap>

// Icon is drawn in code, so no resource files are needed.
static QIcon makeIcon() {
    QPixmap pm(64, 64);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QColor("#141414"));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(QRectF(2, 2, 60, 60), 14, 14);
    p.setBrush(QColor("#d400c8"));                       // accent-coloured "play" triangle
    p.drawPolygon(QPolygonF({QPointF(24, 16), QPointF(24, 48), QPointF(50, 32)}));
    return QIcon(pm);
}

TrayController::TrayController(QObject *parent) : QObject(parent) {
    if (!available())
        return;

    m_menu = new QMenu();                                 // owned here, not by a widget parent
    m_showAction = m_menu->addAction(QString(), this, &TrayController::toggleRequested);
    m_connectAction = m_menu->addAction(QString(), this, &TrayController::connectToggleRequested);
    m_menu->addSeparator();
    m_menu->addAction(tr("Quit"), this, &TrayController::quitRequested);
    connect(m_menu, &QMenu::aboutToShow, this, &TrayController::updateTexts);

    m_tray = new QSystemTrayIcon(makeIcon(), this);
    m_tray->setToolTip("HID Radio");
    m_tray->setContextMenu(m_menu);
    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason r) {
        if (r == QSystemTrayIcon::Trigger)                // left click
            emit toggleRequested();
    });
    updateTexts();
    m_tray->show();
}

TrayController::~TrayController() { delete m_menu; }

void TrayController::setWindowVisible(bool v) {
    if (m_windowVisible == v) return;
    m_windowVisible = v;
    updateTexts();
    emit windowVisibleChanged();
}

void TrayController::setConnected(bool c) {
    if (m_connected == c) return;
    m_connected = c;
    updateTexts();
    emit connectedChanged();
}

void TrayController::updateTexts() {
    if (!m_tray) return;
    m_showAction->setText(m_windowVisible ? tr("Hide") : tr("Show"));
    m_connectAction->setText(m_connected ? tr("Disconnect") : tr("Connect"));
    m_tray->setToolTip(m_connected ? "HID Radio (connected)" : "HID Radio");
}
