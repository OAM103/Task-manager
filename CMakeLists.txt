# CMakeLists.txt - сценарий сборки проекта
#
# CMake НЕ является частью работы самого Task Manager во время запуска
# Он описывает, как из исходников получить TaskManager.exe, как установить Qt-зависимости и как CPack/NSIS собрать Windows-инсталлятор
#
# Основная цепочка:
#   C++/Qt исходники - cmake --build
#   TaskManager.exe - cmake --install + windeployqt
#   папка с exe + Qt DLL - cpack -G NSIS
#   Task Manager-1.0.0-win64.exe

cmake_minimum_required(VERSION 3.21)

project(DesktopApp
VERSION 1.0.0
LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
Core
Gui
Widgets
)

qt6_add_executable(DesktopApp
WIN32


src/app.rc

src/main.cpp

src/MainWindow.cpp
src/MainWindow.hpp

src/SettingsDialog.cpp
src/SettingsDialog.hpp

src/Task.hpp

src/TaskDialog.cpp
src/TaskDialog.hpp

src/TaskModel.cpp
src/TaskModel.hpp

src/TaskFilterProxyModel.cpp
src/TaskFilterProxyModel.hpp

src/TaskDelegate.cpp
src/TaskDelegate.hpp

src/ReminderManager.cpp
src/ReminderManager.hpp

src/TaskStorage.cpp
src/TaskStorage.hpp

src/resources.qrc


)

target_link_libraries(DesktopApp
PRIVATE
Qt6::Core
Qt6::Gui
Qt6::Widgets
)

set_target_properties(DesktopApp PROPERTIES
OUTPUT_NAME "TaskManager"
)

include(GNUInstallDirs)

install(
TARGETS DesktopApp
RUNTIME DESTINATION .
)

# Qt deployment

include(
"${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployQt.cmake"
)

# CPack / NSIS

set(CPACK_PACKAGE_NAME "Task Manager")
set(CPACK_PACKAGE_VENDOR "Task Manager")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Desktop task manager for Windows" ) 
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}" ) 
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Task Manager" ) 
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Task Manager" ) 
set(CPACK_PACKAGE_FILE_NAME "Task Manager-${PROJECT_VERSION}-win64" ) 
set(CPACK_GENERATOR "NSIS" ) 
set(CPACK_NSIS_DISPLAY_NAME "Task Manager" ) 
set(CPACK_NSIS_PACKAGE_NAME "Task Manager" ) 
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64" )

# ВАЖНО:

# Не используем встроенный механизм

# CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL.

# Вместо него ниже реализован собственный

# проверяемый механизм предварительного удаления.

#

set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL OFF)
set(CPACK_NSIS_MODIFY_PATH OFF)

# Исполняемый файл.

set(CPACK_PACKAGE_EXECUTABLES "TaskManager""Task Manager"
)

# Запуск приложения после установки

set(CPACK_NSIS_MUI_FINISHPAGE_RUN
"TaskManager.exe"
)

# Каталог исполняемых файлов

set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")

# Ярлык на рабочем столе.

set(CPACK_CREATE_DESKTOP_LINKS"TaskManager")

# Ярлык в меню Пуск.

set(CPACK_NSIS_MENU_LINKS "TaskManager.exe" "Task Manager")

# Иконка установщика.

set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/src/app.ico")

# Иконка деинсталлятора.

set(CPACK_NSIS_MUI_UNIICON"${CMAKE_CURRENT_SOURCE_DIR}/src/app.ico")

# Предварительное удаление старой версии.

# Здесь НЕ используется CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL.

# Сначала ищем существующую запись Task Manager в Windows Uninstall registry

# Затем проверяем, существует ли настоящий Uninstall.exe.

# Если он существует:

# Да -> запускаем его и ждём завершения.

# Нет -> удаляем устаревшую запись и продолжаем.

# Это предотвращает ситуацию:

# "Task Manager is already installed" -- "Uninstall failed" когда приложение фактически уже отсутствует

set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS
"
SetRegView 64


ReadRegStr $0 HKLM 'Software/Microsoft/Windows/CurrentVersion/Uninstall/Task Manager' 'InstallLocation'

StrCmp $0 '' old_install_not_found

IfFileExists '$0\\\\Uninstall.exe' old_uninstaller_found old_install_not_found

old_uninstaller_found:

    MessageBox MB_YESNO|MB_ICONQUESTION 'Task Manager is already installed.\\n\\nDo you want to uninstall the old version before installing the new one?' IDYES uninstall_old_version IDNO continue_install

uninstall_old_version:

    ExecWait '\"$0\\\\Uninstall.exe\" _?=$0' $1

    IntCmp $1 0 uninstall_success uninstall_failed uninstall_success

uninstall_success:

    Goto continue_install

uninstall_failed:

    MessageBox MB_OK|MB_ICONSTOP 'Uninstall failed. The new version will not be installed.'

    Abort

old_install_not_found:

    DeleteRegKey HKLM 'Software/Microsoft/Windows/CurrentVersion/Uninstall/Task Manager'

    Goto continue_install

continue_install:
"


)

# Дополнительные действия после установки.

#

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
"
CreateShortCut '$DESKTOP/Task Manager.lnk' '$INSTDIR/TaskManager.exe'

CreateDirectory '$SMPROGRAMS/Task Manager'

CreateShortCut '$SMPROGRAMS/Task Manager/Task Manager.lnk' '$INSTDIR/TaskManager.exe'
"


)

# Полная очистка при обычном удалении.

# 1. Завершаем TaskManager.exe.

# 2. Удаляем автозапуск.

# 3. Удаляем настройки Qt/QSettings.

# 4. Удаляем сохранённые задачи.

# 5. Удаляем данные DesktopApp.

# 6. Удаляем ярлыки.

# 7. Удаляем папку программы.


set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
"
SetShellVarContext current


ExecWait '\\\"$SYSDIR/taskkill.exe\\\" /F /T /IM TaskManager.exe'

Sleep 1000

DeleteRegValue HKCU 'Software/Microsoft/Windows/CurrentVersion/Run' 'TaskManager'

DeleteRegKey /ifempty HKCU 'Software/TaskManager/Task Manager'

DeleteRegKey /ifempty HKCU 'Software/TaskManager'

Delete '$APPDATA/TaskManager/Task Manager/tasks.json'

RMDir /r '$APPDATA/TaskManager/Task Manager'

Delete '$APPDATA/DesktopApp/tasks.json'

RMDir /r '$APPDATA/DesktopApp'

Delete '$DESKTOP/Task Manager.lnk'

Delete '$SMPROGRAMS/Task Manager/Task Manager.lnk'

RMDir '$SMPROGRAMS/Task Manager'
"

)

include(CPack)
