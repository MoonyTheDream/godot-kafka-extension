// src/KafkaConsumer.cpp
#include "KafkaConsumer.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void KafkaConsumer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_bootstrap_servers", "servers"),
                         &KafkaConsumer::set_bootstrap_servers);
    ClassDB::bind_method(D_METHOD("set_topic", "topic"),
                         &KafkaConsumer::set_topic);
    ClassDB::bind_method(D_METHOD("set_security_protocol", "protocol"),
                         &KafkaConsumer::set_security_protocol);
    ClassDB::bind_method(D_METHOD("set_ssl_ca_location", "path"),
                         &KafkaConsumer::set_ssl_ca_location);
    ClassDB::bind_method(D_METHOD("set_group_id", "id"),
                         &KafkaConsumer::set_group_id);
    ClassDB::bind_method(D_METHOD("set_max_queue_size", "size"),
                         &KafkaConsumer::set_max_queue_size);
    ClassDB::bind_method(D_METHOD("change_topic", "new_topic"),
                         &KafkaConsumer::change_topic);
    ClassDB::bind_method(D_METHOD("start"),
                         &KafkaConsumer::start);
    ClassDB::bind_method(D_METHOD("stop"),
                         &KafkaConsumer::stop);
    ClassDB::bind_method(D_METHOD("has_message"),
                         &KafkaConsumer::has_message);
    ClassDB::bind_method(D_METHOD("get_message"),
                         &KafkaConsumer::get_message);
    ClassDB::bind_method(D_METHOD("is_running"),
                         &KafkaConsumer::is_running);

    ADD_SIGNAL(MethodInfo("consumer_ready"));
    ADD_SIGNAL(MethodInfo("consumer_error",
        PropertyInfo(Variant::STRING, "message")));
}

KafkaConsumer::KafkaConsumer() { }

KafkaConsumer::~KafkaConsumer() {
    stop();
}

void KafkaConsumer::set_bootstrap_servers(const String &servers) {
    brokers = servers.utf8().get_data();
}

void KafkaConsumer::set_topic(const String &t) {
    topic = t.utf8().get_data();
}

void KafkaConsumer::set_security_protocol(const String &protocol) {
    security_protocol = protocol.utf8().get_data();
}

void KafkaConsumer::set_ssl_ca_location(const String &path) {
    ssl_ca_location = path.utf8().get_data();
}

void KafkaConsumer::set_group_id(const String &id) {
    group_id = id.utf8().get_data();
}

void KafkaConsumer::set_max_queue_size(int size) {
    if (size > 0) {
        max_queue_size = static_cast<size_t>(size);
    }
}

void KafkaConsumer::change_topic(const String &new_topic) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    pending_topic = new_topic.utf8().get_data();
    topic_change_requested.store(true);
}

void KafkaConsumer::start() {
    if (running.load()) {
        return;
    }
    running.store(true);
    consumer_thread = std::thread(&KafkaConsumer::_consume_loop, this);
}

void KafkaConsumer::stop() {
    running.store(false);
    if (consumer_thread.joinable()) {
        consumer_thread.join();
    }
}

bool KafkaConsumer::is_running() {
    return running.load();
}

bool KafkaConsumer::has_message() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    return !message_queue.empty();
}

String KafkaConsumer::get_message() {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (message_queue.empty()) {
        return String();
    }
    String msg = message_queue.front();
    message_queue.pop();
    return msg;
}

void KafkaConsumer::_consume_loop() {
    try {
        kafka::Properties props({{"bootstrap.servers", brokers}});
        if (!security_protocol.empty()) props.put("security.protocol", security_protocol);
        if (!ssl_ca_location.empty()) props.put("ssl.ca.location", ssl_ca_location);
        if (!group_id.empty()) props.put("group.id", group_id);

        kafka::clients::consumer::KafkaConsumer consumer(props);
        consumer.subscribe({topic});
        call_deferred("emit_signal", "consumer_ready");

        while (running.load()) {
            if (topic_change_requested.load()) {
                std::string new_topic;
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    new_topic = pending_topic;
                }
                consumer.unsubscribe();
                consumer.subscribe({new_topic});
                topic = new_topic;
                topic_change_requested.store(false);
                call_deferred("emit_signal", "consumer_ready");
            }

            auto records = consumer.poll(std::chrono::milliseconds(100));
            for (const auto &record : records) {
                if (!record.error()) {
                    const auto value_view = record.value();
                    String value = String::utf8(
                        static_cast<const char *>(value_view.data()),
                        value_view.size());
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    if (message_queue.size() < max_queue_size) {
                        message_queue.push(value);
                    }
                }
            }
        }
        consumer.close();
    } catch (const kafka::KafkaException &e) {
        call_deferred("emit_signal", "consumer_error", String(e.what()));
        running.store(false);
    }
}
