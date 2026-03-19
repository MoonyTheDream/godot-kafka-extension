// src/KafkaProducer.cpp
#include "KafkaProducer.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void KafkaProducer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_bootstrap_servers", "servers"),
                         &KafkaProducer::set_bootstrap_servers);
    ClassDB::bind_method(D_METHOD("set_topic", "topic"),
                         &KafkaProducer::set_topic);
    ClassDB::bind_method(D_METHOD("set_security_protocol", "protocol"),
                         &KafkaProducer::set_security_protocol);
    ClassDB::bind_method(D_METHOD("set_ssl_ca_location", "path"),
                         &KafkaProducer::set_ssl_ca_location);
    ClassDB::bind_method(D_METHOD("connect_to_broker"),
                         &KafkaProducer::connect_to_broker);
    ClassDB::bind_method(D_METHOD("send_message", "message"),
                         &KafkaProducer::send_message);

    ADD_SIGNAL(MethodInfo("producer_ready"));
    ADD_SIGNAL(MethodInfo("producer_error",
        PropertyInfo(Variant::STRING, "message")));
}

KafkaProducer::KafkaProducer() { }

KafkaProducer::~KafkaProducer() {
    if (producer) {
        producer->close(std::chrono::milliseconds(5000));
    }
}

void KafkaProducer::set_bootstrap_servers(const String &servers) {
    brokers = servers.utf8().get_data();
}

void KafkaProducer::set_topic(const String &t) {
    topic = t.utf8().get_data();
}

void KafkaProducer::set_security_protocol(const String &protocol) {
    security_protocol = protocol.utf8().get_data();
}

void KafkaProducer::set_ssl_ca_location(const String &path) {
    ssl_ca_location = path.utf8().get_data();
}

void KafkaProducer::connect_to_broker() {
    if (producer) {
        return;
    }
    try {
        kafka::Properties props;
        props.put("bootstrap.servers", brokers);
        if (!security_protocol.empty()) props.put("security.protocol", security_protocol);
        if (!ssl_ca_location.empty()) props.put("ssl.ca.location", ssl_ca_location);

        producer = std::make_unique<kafka::clients::producer::KafkaProducer>(props);
        emit_signal("producer_ready");
    } catch (const kafka::KafkaException &e) {
        emit_signal("producer_error", String(e.what()));
    }
}

void KafkaProducer::send_message(const String &message) {
    if (!producer) {
        emit_signal("producer_error",
            String("Not connected. Call connect_to_broker() first."));
        return;
    }
    try {
        CharString msg = message.utf8();
        kafka::clients::producer::ProducerRecord record(
            kafka::Topic(topic),
            kafka::NullKey,
            kafka::Value(msg.get_data(), msg.length()));

        auto delivery_cb = [this](
                const kafka::clients::producer::RecordMetadata &,
                const kafka::Error &error) {
            if (error) {
                call_deferred("emit_signal", "producer_error",
                    String(error.message().c_str()));
            }
        };

        producer->send(
            record,
            delivery_cb,
            kafka::clients::producer::KafkaProducer::SendOption::ToCopyRecordValue,
            kafka::clients::producer::KafkaProducer::ActionWhileQueueIsFull::NoBlock);
    } catch (const kafka::KafkaException &e) {
        emit_signal("producer_error", String(e.what()));
    }
}
