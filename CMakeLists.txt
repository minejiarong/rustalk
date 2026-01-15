cmake_minimum_required(VERSION 3.5) 
 
 project(rustalk VERSION 0.1 LANGUAGES CXX) 
 
 set(CMAKE_INCLUDE_CURRENT_DIR ON) 
 set(CMAKE_AUTOUIC ON) 
 set(CMAKE_AUTOMOC ON) 
 set(CMAKE_AUTORCC ON) 
 
 set(CMAKE_CXX_STANDARD 11) 
 set(CMAKE_CXX_STANDARD_REQUIRED ON) 
 
 find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED) 
 find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED) 
 
 set(PROJECT_SOURCES 
     main.cpp 
     mainwindow.cpp 
     mainwindow.h 
     mainwindow.ui 
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
 
 target_link_libraries(rustalk PRIVATE Qt${QT_VERSION_MAJOR}::Widgets) 
 
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
 
 target_link_libraries(rustalk PRIVATE rustalk_core) 
 
 add_custom_command(TARGET rustalk POST_BUILD 
     COMMAND ${CMAKE_COMMAND} -E copy_if_different 
         ${RUST_TARGET_DIR}/rustalk_core.dll 
         $<TARGET_FILE_DIR:rustalk> 
 ) 
 
 if(QT_VERSION_MAJOR EQUAL 6) 
     qt_finalize_executable(rustalk) 
 endif()
