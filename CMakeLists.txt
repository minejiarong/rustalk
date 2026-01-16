cmake_minimum_required(VERSION 3.5) 
 
 project(rustalk VERSION 0.1 LANGUAGES CXX) 
 
set(CMAKE_INCLUDE_CURRENT_DIR ON) 
set(CMAKE_AUTOUIC ON) 
set(CMAKE_AUTOMOC ON) 
set(CMAKE_AUTORCC ON) 
set(CMAKE_AUTOMOC_COMPILER_PREDEFINES OFF)
 
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

# Ensure AutoMoc can preprocess CMakeCXXCompilerABI.cpp which includes CMakeCompilerABI.h
# Add CMake Modules directory to global include search path
include_directories(SYSTEM "${CMAKE_ROOT}/Modules")

 
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Network REQUIRED) 
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED) 
 
set(PROJECT_SOURCES 
    main.cpp 
    mainwindow.cpp 
    mainwindow.h 
    MessageListModel.cpp
    MessageListModel.h
    MessageDelegate.cpp
    MessageDelegate.h
) 
 
 if(${QT_VERSION_MAJOR} GREATER_EQUAL 6) 
     qt_add_executable(rustalk 
         MANUAL_FINALIZATION 
         ${PROJECT_SOURCES} 
     ) 
 else() 
     add_executable(rustalk 
         ${PROJECT_SOURCES} 
     ) 
 endif() 
 
target_link_libraries(rustalk PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# ============================== 
# Rust ↔ Qt integration 
# ============================== 
 
 set(RUST_PROJECT_DIR ${CMAKE_SOURCE_DIR}/rust/rustalk_core) 
 set(RUST_TARGET_DIR ${RUST_PROJECT_DIR}/target/debug) 
 
 add_custom_target(rustalk_core_build ALL 
     COMMAND cargo build 
     WORKING_DIRECTORY ${RUST_PROJECT_DIR} 
     COMMENT "Building rustalk_core (Rust backend)" 
 ) 
 
 add_dependencies(rustalk rustalk_core_build) 
 
 target_link_directories(rustalk PRIVATE ${RUST_TARGET_DIR}) 
 
target_link_libraries(rustalk PRIVATE ${RUST_TARGET_DIR}/rustalk_core.dll.lib) 
 
add_custom_command(TARGET rustalk POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${RUST_TARGET_DIR}/rustalk_core.dll 
        $<TARGET_FILE_DIR:rustalk> 
 ) 
 
 # WebSocket server build and integration
 set(SERVER_PROJECT_DIR ${CMAKE_SOURCE_DIR}/rust/rustakl_sever)
 add_custom_target(rustakl_sever_build ALL
     COMMAND cargo build
     WORKING_DIRECTORY ${SERVER_PROJECT_DIR}
     COMMENT "Building rustakl_sever (WS server)"
 )
 add_dependencies(rustalk rustakl_sever_build)
 add_custom_command(TARGET rustalk POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy_if_different
         ${SERVER_PROJECT_DIR}/target/debug/rustakl_sever.exe
         $<TARGET_FILE_DIR:rustalk>
 )
 target_compile_definitions(rustalk PRIVATE RUSTALK_SERVER_DIR="${SERVER_PROJECT_DIR}")
 
 if(QT_VERSION_MAJOR EQUAL 6) 
     qt_finalize_executable(rustalk) 
 endif()
