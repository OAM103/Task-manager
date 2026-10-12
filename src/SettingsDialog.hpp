/*
 * SettingsDialog.hpp - интерфейс окна настроек
 *
 * Диалог сам не сохраняет настройки в файл/реестр. Он только показывает элементы управления и позволяет MainWindow прочитать выбранные значения
 */
#pragma once

#include <QDialog>
#include <QString>

class QCheckBox;
class QComboBox;
class QSpinBox;

class SettingsDialog : public QDialog
{
public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    void setAutostartEnabled(bool enabled);

    bool autostartEnabled() const;

    void setMinimizeToTray(bool enabled);

    bool minimizeToTray() const;

    void setNotificationsEnabled(bool enabled);

    bool notificationsEnabled() const;

    void setStartMinimized(bool enabled);

    bool startMinimized() const;

    void setNotificationDuration(int duration);

    int notificationDuration() const;

    void setTheme(const QString &theme);

    QString theme() const;

private:
    QCheckBox *autostartCheckBox;
    QCheckBox *minimizeToTrayCheckBox;
    QCheckBox *notificationsCheckBox;
    QCheckBox *startMinimizedCheckBox;

    QSpinBox *notificationDurationSpinBox;

    QComboBox *themeComboBox;
};