#ifndef QOPENHD_H
#define QOPENHD_H

#include <QFont>
#include <QObject>
#include <QQmlApplicationEngine>
#include <QTranslator>

#ifdef ENABLE_SPEECH
#include <QTextToSpeech>
#endif

#include "../../lib/lqtutils_master/lqtutils_prop.h"

/**
 * Dirty, but for some reason stephen made translation(s) and a bit more work this way.
 * This singleton is for handling Appplication (QOpenHD, NOT OpenHD) - specific things that
 * need the QT Engine or some other quirks.
 */
class QOpenHD : public QObject
{
    Q_OBJECT
public:
    explicit QOpenHD(QObject *parent = nullptr);
    static QOpenHD& instance();
    Q_INVOKABLE void switchToLanguage(const QString &language);
    void setEngine(QQmlApplicationEngine *engine);
    Q_PROPERTY(QString fontFamily MEMBER m_fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    void setFontFamily(QString fontFamily);
    void textToSpeech_sayMessage(QString message);

    // This only terminates the App, on most OpenHD images the system service will then restart
    // QOpenHD. Can be usefully for debugging, if something's wrong with the app and you need to restart it
    Q_INVOKABLE void quit_qopenhd();
    // This not only quits qopenhd, but also disables the autostart service
    // (until next reboot)
    Q_INVOKABLE void disable_service_and_quit();
    // Develoment only
    Q_INVOKABLE void restart_local_oenhd_service();
    Q_INVOKABLE void run_dhclient_eth0();
    // Save/Backup QOpenHD local settings to a file, such that they can be reused after a reflash
    // The backup file is stored in /boot/openhd/QOpenHD.conf
    // returns true on success, false otherwise
    Q_INVOKABLE bool backup_settings_locally();
    // Overwrite the local QOpenHD settings with a previous packed up settings file
    // returns true on success, false otherwise
    // To apply, QOpenHD needs to be restarted
    Q_INVOKABLE bool overwrite_settings_from_backup_file();
    // Deletes the local QOpenHD.conf file, such that QOpenHD re-creates all settings after a restart
    Q_INVOKABLE bool reset_settings();
    // only works on linux, dirty helper to get local IP address
    Q_INVOKABLE QString show_local_ip();
    Q_INVOKABLE QString write_local_log();
    // returns true if the platform qopenhd is running on is linux (embedded or x86)
    // some settings an stuff depend on that, called from .qml
    // NOTE: android is not linux in this definition !
    Q_INVOKABLE bool is_linux();
    Q_INVOKABLE bool is_mac();
    Q_INVOKABLE bool is_android();
    Q_INVOKABLE bool is_windows();
    Q_INVOKABLE void android_open_tethering_settings();
    // runs systemctl start/stop/enable/disable openhd
    // opens error message if the openhd service file does not exist (e.g. false on all non linux platforms)
    Q_INVOKABLE void sysctl_openhd(int task);

    Q_INVOKABLE bool is_valid_ip(QString ip);
    Q_INVOKABLE bool is_platform_rpi();
    Q_INVOKABLE bool is_platform_rock();
    //
    // Tries to mimic android toast as much as possible
    //
    Q_INVOKABLE void show_toast(QString message,bool long_toast=false);
    L_RO_PROP(QString,version_string,set_version_string,"2.6.3-evo");
    //
    // Shows a message popup to the user that needs to be clicked away - use sparingly
    //
    Q_INVOKABLE void show_error_message(QString message);
    L_RO_PROP(QString,error_message_text,set_error_message_text,"");
    // Notify that something is going on for a specified amount of time
public:
    Q_INVOKABLE void set_busy_for_milliseconds(int milliseconds,QString reason);
    L_RO_PROP(bool,is_busy,set_is_busy,false);
    L_RO_PROP(QString,busy_reason,set_busy_reason,"");
public:
    L_RO_PROP(QString,toast_text,set_toast_text,"NONE");
    L_RO_PROP(bool,toast_visible,set_toast_visible,false);
public:
signals:
    void fontFamilyChanged(QString fontFamily);
private:
     QQmlApplicationEngine *m_engine = nullptr;
     QTranslator m_translator;
     QString m_fontFamily;
     QFont m_font;
#if defined(ENABLE_SPEECH)
    QTextToSpeech *m_speech;
#endif
public:
    // We always want the screen to be kept "On" while QOpenHD is running -
    // but how to do that depends highly on the platform
    Q_INVOKABLE void keep_screen_on(bool on);
private:
    struct ToastMessage{
        QString text;
        bool long_toast;
    };
    std::list<ToastMessage> m_toast_message_queue;
    void handle_toast_timeout();
    void handle_busy_timeout();
public:
signals:
    void signal_toast_add(QString text,bool long_toast);
private:
    void do_not_call_toast_add(QString text,bool long_toast);
    void show_toast_and_add_remove_timer(QString text,bool long_toast);
};

#endif // QOPENHD_H
