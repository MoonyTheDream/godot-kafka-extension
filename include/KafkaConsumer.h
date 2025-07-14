// include/KafkaConsumer.h
#ifndef GODOT_KAFKACONSUMER_H
#define GODOT_KAFKACONSUMER_H

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>
#include <atomic>
#include <thread>
#include <queue>
#include <mutex>
#include <kafka/KafkaConsumer.h>

namespace godot {

class KafkaConsumer : public RefCounted {
    GDCLASS(KafkaConsumer, RefCounted);

private:
    std::string brokers;
    std::string topic;
    std::string pending_topic;
    bool topic_changed = false;
    std::atomic<bool> running;
    std::thread consumer_thread;
    std::queue<String> message_queue;
    std::mutex queue_mutex;

    void _consume_loop();  // Thread entry point

public:
    static void _bind_methods();

    void set_bootstrap_servers(const String &servers);
    void set_topic(const String &topic);
    void change_topic(const String &new_topic);
    void start();
    void stop();
    bool has_message();
    String get_message();

    KafkaConsumer();
    ~KafkaConsumer();
};

} // namespace godot

#endif // GODOT_KAFKACONSUMER_H
