// include/KafkaProducer.h
#ifndef GODOT_KAFKAPRODUCER_H
#define GODOT_KAFKAPRODUCER_H

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <string>
#include <memory>
#include <kafka/KafkaProducer.h>

namespace godot {

class KafkaProducer : public RefCounted {
    GDCLASS(KafkaProducer, RefCounted);

private:
    std::string brokers;
    std::string topic;
    std::string security_protocol;
    std::string ssl_ca_location;
    std::unique_ptr<kafka::clients::producer::KafkaProducer> producer;

public:
    static void _bind_methods();

    void set_bootstrap_servers(const String &servers);
    void set_topic(const String &topic);
    void set_security_protocol(const String &protocol);
    void set_ssl_ca_location(const String &path);
    void connect_to_broker();
    void send_message(const String &message);

    KafkaProducer();
    ~KafkaProducer();
};

} // namespace godot

#endif // GODOT_KAFKAPRODUCER_H
