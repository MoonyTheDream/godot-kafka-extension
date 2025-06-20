#ifndef KAFKA_PRODUCER_H
#define KAFKA_PRODUCER_H

#include <godot_cpp/classes/reference.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <librdkafka/rdkafkacpp.h>
#include <memory>

namespace godot {

class KafkaProducer : public Reference {
    GDCLASS(KafkaProducer, Reference)

private:
    std::string _topic;
    std::unique_ptr<RdKafka::Producer> _producer;

public:
    static void _bind_methods();

    KafkaProducer();
    ~KafkaProducer();

    void set_topic(const String &topic);
    void send_message(const String &message);
};

} // namespace godot

#endif // KAFKA_PRODUCER_H
