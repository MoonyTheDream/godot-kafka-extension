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