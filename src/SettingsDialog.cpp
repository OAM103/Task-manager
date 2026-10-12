/*
 * SettingsDialog.cpp - создание и чтение элементов окна настроек
 *
 * Этот класс можно воспринимать как форму: MainWindow заполняет форму текущими значениями, пользователь изменяет их, затем MainWindow забирает новые значения через методы-геттеры
 */
#include "SettingsDialog.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

// Создаём форму настроек. Сохранением занимается MainWindow, а не этот диалог
SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Настройки");
    setMinimumWidth(500);

    autostartCheckBox = new QCheckBox("Запускать приложение вместе с Windows", this);
    minimizeToTrayCheckBox = new QCheckBox( "Сворачивать приложение в системный трей", this);
    notificationsCheckBox = new QCheckBox("Показывать уведомления о просроченных задачах", this);
    startMinimizedCheckBox = new QCheckBox("Запускать приложение свёрнутым в трей", this);
    notificationDurationSpinBox = new QSpinBox(this);
    notificationDurationSpinBox->setRange(1000, 60000);
    notificationDurationSpinBox->setSingleStep(1000);
    notificationDurationSpinBox->setSuffix(" мс");
    themeComboBox = new QComboBox(this);
    themeComboBox->addItem("Светлая", "light");
    themeComboBox->addItem("Тёмная", "dark");

    auto *appearanceGroup = new QGroupBox("Внешний вид", this);
    auto *appearanceLayout = new QFormLayout(appearanceGroup);

    appearanceLayout->addRow("Тема приложения:", themeComboBox);

    auto *behaviorGroup = new QGroupBox("Поведение", this);
    auto *behaviorLayout = new QVBoxLayout(behaviorGroup);

    behaviorLayout->addWidget(autostartCheckBox);
    behaviorLayout->addWidget(minimizeToTrayCheckBox);
    behaviorLayout->addWidget(startMinimizedCheckBox);

    auto *notificationGroup = new QGroupBox("Уведомления",this);
    auto *notificationLayout = new QFormLayout(notificationGroup);

    notificationLayout->addRow(notificationsCheckBox);
    notificationLayout->addRow("Длительность уведомления:", notificationDurationSpinBox);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    buttons->button(QDialogButtonBox::Ok)->setText("Сохранить");
    buttons->button(QDialogButtonBox::Cancel)->setText("Отмена");

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(appearanceGroup);
    layout->addWidget(behaviorGroup);
    layout->addWidget(notificationGroup);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

/* Методы set... заполняют форму текущими значениями перед открытием диалога. */
void SettingsDialog::setAutostartEnabled(
    bool enabled
)
{
    autostartCheckBox->setChecked(
        enabled
    );
}

bool SettingsDialog::autostartEnabled() const
{
    return autostartCheckBox->isChecked();
}

void SettingsDialog::setMinimizeToTray(
    bool enabled
)
{
    minimizeToTrayCheckBox->setChecked(
        enabled
    );
}

bool SettingsDialog::minimizeToTray() const
{
    return minimizeToTrayCheckBox->isChecked();
}

void SettingsDialog::setNotificationsEnabled(
    bool enabled
)
{
    notificationsCheckBox->setChecked(
        enabled
    );
}

bool SettingsDialog::notificationsEnabled() const
{
    return notificationsCheckBox->isChecked();
}

void SettingsDialog::setStartMinimized(
    bool enabled
)
{
    startMinimizedCheckBox->setChecked(
        enabled
    );
}

bool SettingsDialog::startMinimized() const
{
    return startMinimizedCheckBox->isChecked();
}

void SettingsDialog::setNotificationDuration(
    int duration
)
{
    notificationDurationSpinBox->setValue(
        duration
    );
}

int SettingsDialog::notificationDuration() const
{
    return notificationDurationSpinBox->value();
}

void SettingsDialog::setTheme(
    const QString &theme
)
{
    const int index =
        themeComboBox->findData(
            theme
        );

    if (index >= 0)
    {
        themeComboBox->setCurrentIndex(
            index
        );
    }
}

/* Возвращаем внутреннее значение темы: light или dark, а не текст, который видит пользователь. */
QString SettingsDialog::theme() const
{
    return themeComboBox
        ->currentData()
        .toString();
}