find_package(Qt4 4.6 REQUIRED)

add_definitions(${QT_DEFINITIONS})
include_directories(${QT_INCLUDES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(qml2image_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Qml2Image.cpp
)

qt4_automoc(${qml2image_SRCS})
add_executable(qml2image ${qml2image_SRCS})
target_link_libraries(qml2image ${QT_QTDECLARATIVE_LIBRARY} ${QT_QTSVG_LIBRARY})
