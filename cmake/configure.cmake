if(UNIX)
    set(COMMON_LIBS ${COMMON_LIBS} pthread)
endif(UNIX)

find_package(OpenGL REQUIRED)
set(COMMON_LIBS ${COMMON_LIBS} ${OPENGL_gl_LIBRARY})

find_package(GLFW3 REQUIRED)
include_directories(${GLFW3_INCLUDE_DIRS})
set(COMMON_LIBS ${COMMON_LIBS} ${GLFW3_LIBRARIES})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
set(COMMON_LIBS ${COMMON_LIBS} ${GLEW_LIBRARIES})

add_definitions(-DUSE_PNG)
set(COMMON_LIBS ${COMMON_LIBS} png)
set(OPENGP_HEADERONLY OFF)
add_subdirectory(OpenGP EXCLUDE_FROM_ALL)
include_directories(${PROJECT_SOURCE_DIR}/OpenGP/src)
message(STATUS ${PROJECT_SOURCE_DIR}/OpenGP/src)
add_definitions(-DOPENGP_HEADERONLY)
set(COMMON_LIBS ${COMMON_LIBS} OpenGP)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(
    CMAKE_CXX_FLAGS
    "-std=c++14 -Wall"
)

