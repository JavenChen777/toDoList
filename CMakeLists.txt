cmake_minimum_required(VERSION 3.16)
project(TodoList VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置 UI 文件搜索路径
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

# 查找 Qt6 组件
find_package(Qt6 REQUIRED COMPONENTS 
    Core 
    Widgets 
    Network 
    Charts
    Sql
)

# 源文件
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/TaskItem.cpp
    src/TaskModel.cpp
    src/TaskStorage.cpp
    src/SettingsDialog.cpp
    src/StatsDialog.cpp
    src/ApiServer.cpp
    src/NetworkApiServer.cpp
    src/TaskDelegate.cpp
    src/AddTaskDialog.cpp
    src/HelpDialog.cpp
)

# 头文件
set(HEADERS
    src/MainWindow.h
    src/TaskItem.h
    src/TaskModel.h
    src/TaskStorage.h
    src/SettingsDialog.h
    src/StatsDialog.h
    src/ApiServer.h
    src/NetworkApiServer.h
    src/TaskDelegate.h
    src/AddTaskDialog.h
    src/HelpDialog.h
)

# UI 文件
set(UI_FILES
    ui/MainWindow.ui
    ui/SettingsDialog.ui
    ui/StatsDialog.ui
    ui/AddTaskDialog.ui
    ui/HelpDialog.ui
)

# Qt 资源文件
set(QRC_FILES
    resources.qrc
)

# Windows 资源文件
if(WIN32)
    set(RESOURCE_FILES
        resources.rc
    )
endif()

# 创建可执行文件
add_executable(${PROJECT_NAME} 
    ${SOURCES} 
    ${HEADERS}
    ${UI_FILES}
    ${QRC_FILES}
    ${RESOURCE_FILES}
)

# 链接 Qt 库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
    Qt6::Sql
)

# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Windows 特定设置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
