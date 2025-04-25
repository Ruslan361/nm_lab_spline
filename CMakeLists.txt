cmake_minimum_required(VERSION 3.5)

project(splines VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем директорию headers в список путей для поиска
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Charts) # Added Charts
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Charts) # Added Charts

set(TS_FILES spl_ru_RU.ts)

# Определяем пути к исходным файлам
set(SOURCE_FILES
    src/Functions.cpp
    src/Problems.cpp
    src/Solver.cpp
    src/Spline.cpp
    src/tridiagonal_matrix_algorithm.cpp
)

# Включаем файлы в проект
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
        ${SOURCE_FILES}
)

# Улучшаем передачу путей включения компилятору
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC_COMPILER_PREDEFINES ON)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(spl
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )

    # Явно указываем пути включения для целевого исполняемого файла
    target_include_directories(spl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/headers)

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(spl SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(spl
            ${PROJECT_SOURCES}
        )

        # Явно указываем пути включения для целевого исполняемого файла
        target_include_directories(spl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/headers)
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(spl PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts) # Added Charts

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.spl)
endif()
set_target_properties(spl PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS spl
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(spl)
endif()
