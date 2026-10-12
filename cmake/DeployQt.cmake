if(NOT WIN32)

    return()

endif()

find_program(

    WINDEPLOYQT_EXECUTABLE

    NAMES windeployqt.exe windeployqt

    HINTS
        "${Qt6_DIR}/../../../bin"
        "C:/Qt/6.11.2/mingw_64/bin"

)

if(NOT WINDEPLOYQT_EXECUTABLE)

    message(FATAL_ERROR
        "windeployqt.exe не найден."
    )

endif()

message(STATUS
    "windeployqt найден: ${WINDEPLOYQT_EXECUTABLE}"
)

set(DEPLOYQT_SCRIPT
    "${CMAKE_CURRENT_BINARY_DIR}/DeployQtInstall.cmake"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployQtInstall.cmake.in"
    "${DEPLOYQT_SCRIPT}"
    @ONLY
)

install(
    SCRIPT
    "${DEPLOYQT_SCRIPT}"
)