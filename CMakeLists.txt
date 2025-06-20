cmake_minimum_required(VERSION 3.14)
project(GodotKafkaExtension)

set(CMAKE_CXX_STANDARD 17)

# Include godot-cpp (assume git submodule placed at godot-cpp/)
add_subdirectory(godot-cpp)
include_directories(godot-cpp/include)

# Find librdkafka
find_package(RdKafka CONFIG REQUIRED)
# include_directories(${RdKafka_INCLUDE_DIRS})

# Plugin source files
file(GLOB PLUGIN_SRC src/*.cpp)
add_library(GodotKafkaExtension SHARED ${PLUGIN_SRC})

target_link_libraries(GodotKafkaExtension
    godot-cpp::godot-cpp-static  # static Godot binding library
    RdKafka::RdKafka
    RdKafka::rdkafka++
)

# Define GDExtension symbol visibility (on Windows, use export directives)
if (WIN32)
  target_compile_definitions(GodotKafkaExtension PRIVATE GDE_EXPORT)
endif()
