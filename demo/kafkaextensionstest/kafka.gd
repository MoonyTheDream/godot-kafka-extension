extends Node
var producer = KafkaProducer.new()
var consumer = KafkaConsumer.new()
func _ready() -> void:
	producer.set_bootstrap_servers("109.241.128.160:9092")
	producer.set_topic("serverGeneralTopic")
	var message = JSON.stringify({"foo":42})
	producer.send_message(message)
	
	consumer.set_bootstrap_servers("109.241.128.160:9092")
	consumer.set_topic("serverGeneralTopic")
	consumer.start()
	
func _process(_delta: float) -> void:
	if consumer.has_message():
		var dostalem = consumer.get_message()
		print(dostalem)
