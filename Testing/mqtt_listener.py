import paho.mqtt.enums
import paho.mqtt.client as mq

# This file should always be running 

# QUARANTINE THIS - SECRET INFO!!!
# URL = "ddaa1f93a61a4de7aaf7465c2984155c.s1.eu.hivemq.cloud"
URL = "e23af596b178452b9f37309e8fbf3193.s1.eu.hivemq.cloud"
USER = "igemfridge"
PASS = "Suffering1"
# QUARANTINE THIS - SECRET INFO!!!

PORT = 8883
VERSION = paho.mqtt.enums.CallbackAPIVersion.VERSION2
TLS_VERSION = mq.ssl.PROTOCOL_TLS

client = mq.Client(VERSION)

def on_connect(client, userdata, flags, reason_code, properties):
    print("Connected with result code", reason_code)

def on_msg(client, userdata, message:mq.MQTTMessage):
    topic = message.topic
    payload = message.payload.decode("utf-8")
    
    print("topic", topic, "message", payload)
    return (topic, payload)

def on_sub(client, userdata, mid, granted_qos, _):
    print("granted qos", granted_qos)


client.on_message = on_msg
client.on_connect = on_connect
client.on_subscribe = on_sub
client.username_pw_set(USER, PASS)
client.tls_set(tls_version=TLS_VERSION)
client.connect(URL, PORT)

client.subscribe("arduino/rfid", qos=1)
client.subscribe("arduino/checkout", qos=1)
client.subscribe("python/rfid", qos=1)
client.subscribe("arduino/rack/checkin", qos=1)
client.subscribe("arduino/rack/registration", qos=1)
client.subscribe("python/rack", qos=1)
client.subscribe("python/dbupdate/Rack Well", qos=1)
client.subscribe("python/dbupdate/RFID Chips", qos=1)

client.subscribe("arduino/out", qos=1)

client.loop_forever()