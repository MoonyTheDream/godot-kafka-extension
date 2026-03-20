# godot-kafka-extension

A Godot GDExtension that adds Apache Kafka producer and consumer support to Godot 4.6+.

## Prerequisites

- **Godot 4.6+**
- **SCons** (Python build tool)
- **CMake** (for auto-building librdkafka)
- **C++ compiler** with C++17 support (MSVC on Windows, GCC/Clang on Linux)

## Building

Clone with submodules:

```sh
git clone --recursive https://github.com/MoonyTheDream/godot-kafka-extension.git
cd godot-kafka-extension
```

Build the extension (librdkafka is built automatically on first run):

```sh
# Windows
scons platform=windows target=template_debug

# Linux
scons platform=linux target=template_debug

# Release build
scons platform=windows target=template_release
```

The built library and required runtime DLLs are placed in `demo/kafkaextensionstest/bin/`.

## Usage

### Producer

```gdscript
var producer = KafkaProducer.new()

func _ready():
    producer.set_bootstrap_servers("localhost:9092")
    producer.set_topic("my-topic")
    producer.producer_ready.connect(_on_ready)
    producer.producer_error.connect(_on_error)
    producer.connect_to_broker()

func _on_ready():
    producer.send_message("Hello Kafka!")

func _on_error(msg: String):
    printerr("Producer error: ", msg)
```

**Methods:**
| Method | Description |
|---|---|
| `set_bootstrap_servers(servers: String)` | Set broker address(es) |
| `set_topic(topic: String)` | Set the target topic |
| `connect_to_broker()` | Connect to the Kafka broker |
| `send_message(message: String)` | Send a message (must call `connect_to_broker()` first) |
| `set_security_protocol(protocol: String)` | Set security protocol (e.g. `"SSL"`) |
| `set_ssl_ca_location(path: String)` | Set path to CA certificate file |

**Signals:**
| Signal | Description |
|---|---|
| `producer_ready` | Emitted after successful connection |
| `producer_error(message: String)` | Emitted on connection or send failure |

### Consumer

```gdscript
var consumer = KafkaConsumer.new()

func _ready():
    consumer.set_bootstrap_servers("localhost:9092")
    consumer.set_topic("my-topic")
    consumer.set_group_id("my-group")
    consumer.consumer_ready.connect(_on_ready)
    consumer.consumer_error.connect(_on_error)
    consumer.start()

func _process(_delta):
    while consumer.has_message():
        print(consumer.get_message())

func _on_ready():
    print("Consumer subscribed")

func _on_error(msg: String):
    printerr("Consumer error: ", msg)
```

**Methods:**
| Method | Description |
|---|---|
| `set_bootstrap_servers(servers: String)` | Set broker address(es) |
| `set_topic(topic: String)` | Set the topic to consume |
| `set_group_id(id: String)` | Set consumer group ID (for offset tracking) |
| `start()` | Start consuming in a background thread |
| `stop()` | Stop consuming |
| `has_message() -> bool` | Check if messages are queued |
| `get_message() -> String` | Pop the next message from the queue |
| `is_running() -> bool` | Check if consumer is active |
| `change_topic(new_topic: String)` | Switch topic while running |
| `set_max_queue_size(size: int)` | Set max queued messages (default: 10000) |
| `set_security_protocol(protocol: String)` | Set security protocol |
| `set_ssl_ca_location(path: String)` | Set path to CA certificate file |

**Signals:**
| Signal | Description |
|---|---|
| `consumer_ready` | Emitted after successful subscription |
| `consumer_error(message: String)` | Emitted on connection or polling failure |

## SSL/TLS

To connect to a Kafka broker over SSL:

```gdscript
producer.set_security_protocol("SSL")
producer.set_ssl_ca_location("/path/to/cacert.pem")
producer.connect_to_broker()
```

**Windows note:** librdkafka on Windows does not use the system certificate store. You must provide an explicit CA certificate file. Download one from https://curl.se/ca/cacert.pem and pass the path via `set_ssl_ca_location()`.

**Linux note:** If `set_ssl_ca_location()` is not called, librdkafka uses the system CA bundle automatically.

## Updating to a newer Godot version

1. In `.gitmodules`, the godot-cpp submodule tracks `master`. Just pass the target API version when building:
   ```sh
   scons api_version=4.6
   ```

2. Update `compatibility_minimum` in `demo/kafkaextensionstest/KafkaExtension.gdextension` to match.

3. Rebuild and verify in the Godot editor.


# Correcting for Windows

Install OpenSSL for Windows (MSVC-compatible, not MinGW):

winget install FireDaemon.OpenSSL
This installs to C:\Program Files\FireDaemon-OpenSSL-3 with the .lib files CMake needs.
Alternatively, download the Win64 installer directly from: https://slproweb.com/products/Win32OpenSSL.html

Clean and rebuild librdkafka:

cd libs/godot-kafka-extension
rm -rf thirdparty/librdkafka/build thirdparty/librdkafka/install
Update SConstruct to point CMake to OpenSSL on Windows:

File: libs/godot-kafka-extension/SConstruct (lines 22-24)

if env['platform'] == 'windows':
    cmake_args += ['-A', 'x64']
    openssl_root = os.environ.get('OPENSSL_ROOT_DIR', r'C:\Program Files\OpenSSL-Win64')
    if os.path.exists(openssl_root):
        cmake_args += [f'-DOPENSSL_ROOT_DIR={openssl_root}']
Rebuild everything:

git submodule update --init --recursive
scons platform=windows target=template_debug
Copy new DLLs to morphoLogicClient/bin/:

rdkafka.dll (now linking OpenSSL)
libssl-3-x64.dll and libcrypto-3-x64.dll (OpenSSL runtime)