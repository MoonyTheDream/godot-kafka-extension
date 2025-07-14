// src/KafkaProducer.cpp
#include "KafkaProducer.h"
using namespace godot;

// Bind GDScript methods
void KafkaProducer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_bootstrap_servers", "servers"),
                         &KafkaProducer::set_bootstrap_servers);
    ClassDB::bind_method(D_METHOD("set_topic", "topic"),
                         &KafkaProducer::set_topic);
    ClassDB::bind_method(D_METHOD("send_message", "message"),
                         &KafkaProducer::send_message);
}

KafkaProducer::KafkaProducer() { }
KafkaProducer::~KafkaProducer() {
    // Clean up producer if needed
    if (producer) {
        producer->close();
    }
}

void KafkaProducer::set_bootstrap_servers(const String &servers) {
    brokers = servers.utf8().get_data();
}
void KafkaProducer::set_topic(const String &t) {
    topic = t.utf8().get_data();
    print_line("KafkaProducer topic is set to: " + String(topic.c_str()));

}

void KafkaProducer::send_message(const String &message) {
    if (!producer) {
        kafka::Properties props;
        props.put("bootstrap.servers", brokers);
        producer = std::make_unique<kafka::clients::producer::KafkaProducer>(props);
    }

    // Convert Godot String → C string + length
    CharString msg = message.utf8();
    kafka::clients::producer::ProducerRecord record(
        kafka::Topic(topic),
        kafka::NullKey,
        kafka::Value(msg.get_data(), msg.length())
    );

    // No-op delivery callback
    auto delivery_cb = [](const kafka::clients::producer::RecordMetadata & /*meta*/,
                          const kafka::Error & /*error*/) {
        // Intentionally empty
    };

    // Call the overload with:
    //   record, callback, send option, action if queue is full
    producer->send(
        record,
        delivery_cb,
        kafka::clients::producer::KafkaProducer::SendOption::ToCopyRecordValue,
        kafka::clients::producer::KafkaProducer::ActionWhileQueueIsFull::Block
    );
}
