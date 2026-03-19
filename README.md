# godot-kafka-extension
## Usage

### Producer
Creating a producer:
`producer = KafkaProducer.new()`

Setting up the producer:
`producer.set_bootstrap_servers(<string>)`
`producer.set_topic(<string>)`

Producing a message:
`producer.send_message(<string>)`

### Consumer
Creating a consumer:
`consumer = KafkaConsumer.new()`

Setting up the consumer:
`consumer.set_bootstrap_servers(<string>)`
`consumer.set_topic(<string>)`

Starting the consumer:
`consumer.start()`

Linking consumer messages to a signal (probably in _process()):

## Updating to a newer Godot version

Steps to upgrade from one Godot version to another (e.g. 4.4 → 4.6):

**1. Update the `godot-cpp` submodule**

In `.gitmodules`, change the `branch` field to match the target Godot version:
```
[submodule "godot-cpp"]
    path = godot-cpp
    url = https://github.com/godotengine/godot-cpp.git
    branch = 4.6   # <-- change this
```

Then pull the new version:
```sh
cd godot-cpp
git fetch
git checkout 4.6   # match the branch above
cd ..
git add godot-cpp .gitmodules
git commit -m "Bump godot-cpp to 4.6"
```

**2. Update `compatibility_minimum` in the `.gdextension` file**

In [demo/kafkaextensionstest/KafkaExtension.gdextension](demo/kafkaextensionstest/KafkaExtension.gdextension), change:
```ini
compatibility_minimum = "4.6"
```

**3. Rebuild the extension**

```sh
scons api_version=4.6
```

**4. Verify the extension loads in the new Godot version**

Open the demo project in the target version of the Godot editor and confirm there are no errors in the output log.


---

## Step 1 — C++ extension: add SSL methods

Repo: https://github.com/MoonyTheDream/godot-kafka-extension

Both `KafkaProducer` and `KafkaConsumer` need **identical** changes.

### KafkaProducer.h / KafkaConsumer.h — add private members + public declarations
```cpp
private:
    String security_protocol = "";
    String ssl_ca_location = "";  // empty = use system CA bundle (Linux/Mac)

public:
    void set_security_protocol(const String &protocol);
    void set_ssl_ca_location(const String &path);
```

### KafkaProducer.cpp / KafkaConsumer.cpp — implement setters
```cpp
void KafkaProducer::set_security_protocol(const String &protocol) {
    security_protocol = protocol;
}
void KafkaProducer::set_ssl_ca_location(const String &path) {
    ssl_ca_location = path;
}
```

Apply to rdkafka conf **before `rd_kafka_new`** (in the lazy-init / `start()` block):
```cpp
if (!security_protocol.is_empty()) {
    conf->set("security.protocol",
              std::string(security_protocol.utf8().get_data()), errstr);
}
if (!ssl_ca_location.is_empty()) {
    conf->set("ssl.ca.location",
              std::string(ssl_ca_location.utf8().get_data()), errstr);
}
// Let's Encrypt → hostname verification works → do NOT disable it
```

### register_types.cpp — bind methods (×2, once per class)
```cpp
ClassDB::bind_method(D_METHOD("set_security_protocol", "protocol"),
                     &KafkaProducer::set_security_protocol);
ClassDB::bind_method(D_METHOD("set_ssl_ca_location", "path"),
                     &KafkaProducer::set_ssl_ca_location);
// Repeat for KafkaConsumer
```

After changes: **rebuild Linux `.so` + Windows `.dll`**, copy into `morphoLogicClient/bin/`.

**Windows clients:** librdkafka on Windows doesn't use the system cert store.
- Download `cacert.pem` from https://curl.se/ca/cacert.pem
- Save as `morphoLogicClient/cacert.pem`
- Windows clients set `sslCaLocation = "res://cacert.pem"` in their config

---