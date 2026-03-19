extends Node

var producer = KafkaProducer.new()
var consumer = KafkaConsumer.new()

func _ready() -> void:
	producer.set_bootstrap_servers("localhost:9092")
	producer.set_topic("test-topic")
	producer.producer_ready.connect(_on_producer_ready)
	producer.producer_error.connect(_on_error)
	producer.connect_to_broker()

	consumer.set_bootstrap_servers("localhost:9092")
	consumer.set_topic("test-topic")
	consumer.set_group_id("godot-demo")
	consumer.consumer_ready.connect(_on_consumer_ready)
	consumer.consumer_error.connect(_on_error)
	consumer.start()

func _process(_delta: float) -> void:
	while consumer.has_message():
		print("Received: ", consumer.get_message())

func _on_producer_ready():
	producer.send_message(JSON.stringify({"hello": "world"}))

func _on_consumer_ready():
	print("Consumer subscribed")

func _on_error(msg: String):
	printerr("Kafka error: ", msg)
