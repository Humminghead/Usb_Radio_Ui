#pragma once
#include <QObject>
#include <QSystemTrayIcon>

class QMenu;
class QAction;

// System tray icon with a Show/Hide + Connect/Disconnect + Quit menu.
// Window show/hide itself is done in QML (Main.qml) via the signals below.
class TrayController : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool windowVisible READ windowVisible WRITE setWindowVisible NOTIFY windowVisibleChanged)
    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
public:
    explicit TrayController(QObject *parent = nullptr);
    ~TrayController() override;

    bool available() const { return QSystemTrayIcon::isSystemTrayAvailable(); }
    bool windowVisible() const { return m_windowVisible; }
    bool connected() const { return m_connected; }
    void setWindowVisible(bool v);
    void setConnected(bool c);

signals:
    void windowVisibleChanged();
    void connectedChanged();
    void toggleRequested();         // left click / "Show|Hide"
    void connectToggleRequested();  // "Connect|Disconnect"
    void quitRequested();

private:
    void updateTexts();

    QSystemTrayIcon *m_tray = nullptr;
    QMenu *m_menu = nullptr;
    QAction *m_showAction = nullptr;
    QAction *m_connectAction = nullptr;
    bool m_windowVisible = true;
    bool m_connected = false;
};
