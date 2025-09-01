cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(CtxMenuDLL VERSION 1.0.0 LANGUAGES CXX)

if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    add_definitions(-DUNICODE -D_UNICODE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/MP)
endif()

set(THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty")

set(THIRDPARTY_SRCS
    "${THIRDPARTY_DIR}/tinyxml2/11.0.0/tinyxml2.cpp"
    "${THIRDPARTY_DIR}/tinyxml2/11.0.0/tinyxml2.h"
)

add_library(CtxMenuDLL SHARED
    ${THIRDPARTY_SRCS}
    src/ClassFactory.cpp
    src/ClassFactory.h
    src/ContextMenuHandler.cpp
    src/ContextMenuHandler.h
    src/CtxMenu.cpp
    src/CtxMenu.h
    src/CtxMenuItem.cpp
    src/CtxMenuItem.h
    src/dllmain.cpp
    src/GlobalDef.h
    src/IconManager.cpp
    src/IconManager.h
    src/InstanceGlobal.cpp
    src/InstanceGlobal.h
    src/Logger.cpp
    src/Logger.h
    resource/CtxMenu.rc
)

if(MSVC)
    target_compile_options(CtxMenuDLL PRIVATE /W4)
    target_include_directories(CtxMenuDLL PRIVATE ${THIRDPARTY_DIR})
    source_group("3rdparty" FILES ${THIRDPARTY_SRCS})
endif()

target_link_libraries(CtxMenuDLL
    Ole32
    OleAut32
    shell32
    Uuid
    ShLwApi
)

set_target_properties(CtxMenuDLL PROPERTIES
    OUTPUT_NAME "CtxMenu"
    SUFFIX ".dll"
    PREFIX ""
)

if(WIN32)
    set_target_properties(CtxMenuDLL PROPERTIES
        LINK_FLAGS "${LINK_FLAGS} /DEF:${CMAKE_SOURCE_DIR}/src/CtxMenuExport.def"
    )
endif()

target_compile_definitions(CtxMenuDLL PRIVATE
    _USRDLL
    CTXMENUDLL_EXPORTS
)

set_target_properties(CtxMenuDLL PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

install(TARGETS CtxMenuDLL
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    scripts/install.bat
    scripts/uninstall.bat
    samples/sample.xml
    DESTINATION bin
)
