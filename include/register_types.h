#ifndef KAFKA_REGISTER_TYPES_H
#define KAFKA_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void initialize_kafka_module(ModuleInitializationLevel p_level);
void uninitialize_kafka_module(ModuleInitializationLevel p_level);

#endif // KAFKA_REGISTER_TYPES_H