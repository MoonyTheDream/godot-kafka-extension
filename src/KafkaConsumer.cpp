// src/KafkaConsumer.cpp
#include "KafkaConsumer.h"
using namespace godot;

// Bind GDScript methods
void KafkaConsumer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_bootstrap_servers", "servers"),
                         &KafkaConsumer::set_bootstrap_servers);
    ClassDB::bind_method(D_METHOD("set_topic", "topic"),
                         &KafkaConsumer::set_topic);
    ClassDB::bind_method(D_METHOD("start"),
                         &KafkaConsumer::start);
    ClassDB::bind_method(D_METHOD("stop"),
                         &KafkaConsumer::stop);
    ClassDB::bind_method(D_METHOD("has_message"),
                         &KafkaConsumer::has_message);
    ClassDB::bind_method(D_METHOD("get_message"),
                         &KafkaConsumer::get_message);
}

KafkaConsumer::KafkaConsumer() : running(false) { }
KafkaConsumer::~KafkaConsumer() {
    stop();
}

void KafkaConsumer::set_bootstrap_servers(const String &servers) {
    brokers = servers.utf8().get_data();
}
void KafkaConsumer::set_topic(const String &t) {
    topic = t.utf8().get_data();
}

void KafkaConsumer::start() {
    if (running) {
        return;
    }
    running = true;
    // Launch background thread for polling
    consumer_thread = std::thread(&KafkaConsumer::_consume_loop, this);
}

void KafkaConsumer::stop() {
    running = false;
    if (consumer_thread.joinable()) {
        consumer_thread.join();
    }
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
    // Prepare Kafka consumer configuration
    kafka::Properties props({ {"bootstrap.servers", {brokers}} });
    // (By default enable.auto.commit=true)
    kafka::clients::consumer::KafkaConsumer consumer(props);

    // Subscribe to the topic
    consumer.subscribe({ topic });

    while (running) {
        // Poll for new records (timeout 10ms)
        auto records = consumer.poll(std::chrono::milliseconds(10));
        for (const auto &record : records) {
            if (!record.error()) {
                // Push the message value onto the queue
                String value = String(record.value().toString().c_str());
                std::lock_guard<std::mutex> lock(queue_mutex);
                message_queue.push(value);
            }
        }
    }
    consumer.close();
}
