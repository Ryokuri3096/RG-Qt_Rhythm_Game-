cmake_minimum_required(VERSION 3.16)

project(RG VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(RG
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        res.qrc
        songwindow.h songwindow.cpp songwindow.ui
        playwindow.h playwindow.cpp playwindow.ui
        gamemanager.h gamemanager.cpp
<<<<<<< HEAD
        settingswindow.h settingswindow.cpp settingswindow.ui
=======
        resultoverlay.h resultoverlay.cpp resultoverlay.ui
        pauseoverlay.h pauseoverlay.cpp pauseoverlay.ui



>>>>>>> 522e69526fbc281bc7e7977a8067995a35ecdaf4



    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET RG APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(RG SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(RG
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(RG PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(RG PRIVATE Qt6::Multimedia)
target_include_directories(RG PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}   # 因为 mywidget.h 在项目根目录
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.RG)
endif()
set_target_properties(RG PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS RG
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RG)
endif()

# 由于程序只能读取.exe文件所在目录（\RG\build\Desktop_x86_windows_msys_pe_64bit-Debug）的文件 所以要先把charts文件夹复制过去
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    # 先删除目标目录（如果存在）
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/charts"
    # 再复制整个源目录过去
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/charts"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/charts"
    COMMENT "Syncing charts folder to output directory..."
)
