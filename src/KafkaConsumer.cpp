// src/KafkaConsumer.cpp
#include "KafkaConsumer.h"
using namespace godot;

// Bind GDScript methods
void KafkaConsumer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_bootstrap_servers", "servers"),
                         &KafkaConsumer::set_bootstrap_servers);
    ClassDB::bind_method(D_METHOD("set_topic", "topic"),
                         &KafkaConsumer::set_topic);
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

bool KafkaConsumer::is_running() {
    return running;
}

void KafkaConsumer::change_topic(const String &new_topic) {
    std::lock_guard<std::mutex> lock(queue_mutex);
    pending_topic = new_topic.utf8().get_data();
    topic_changed = true;
}

void KafkaConsumer::start() {
    if (running) {
        return;
    }
    // running = true;
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
    kafka::Properties props({ {"bootstrap.servers", {brokers}} });
    kafka::clients::consumer::KafkaConsumer consumer(props);

    consumer.subscribe({ topic });

    call_deferred("emit_signal", "consumer_ready");
    print_line("KafkaConsumer starting _consume_loop on topic: " + String(topic.c_str()));
    running = true;

    while (running) {
        if (topic_changed) {
            running = false;
            consumer.unsubscribe();
            consumer.subscribe({ pending_topic });
            topic = pending_topic;
            topic_changed = false;
            call_deferred("emit_signal", "consumer_ready");
            print_line("KafkaConsumer switched to topic: " + String(topic.c_str()));
            running = true; 
        }

        auto records = consumer.poll(std::chrono::milliseconds(10));
        for (const auto &record : records) {
            if (!record.error()) {
                std::string raw_value = record.value().toString();
                String value = String::utf8(raw_value.data(), raw_value.size());
                
                std::lock_guard<std::mutex> lock(queue_mutex);
                message_queue.push(value);
            }
        }
    }
    consumer.close();
}

