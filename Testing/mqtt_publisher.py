import paho.mqtt.enums
import paho.mqtt.client as mq
import re
import json
from datetime import datetime

# Make a Mock JSON file
# Initialize data structure
payload_db = {
    "filter": {
        "RFID no": None
    },
    "update": {
        "$set": {
            "In Use?": True,
            "Last tapped time": {
                "$date": None
            },
            "Registration time": {
                "$date": None
            }
        }
    }
}

# Assign RFID and current time
RFID = "40401"
curTime = datetime.now().isoformat() + "Z"  # ISO 8601 format with UTC time

payload_db["filter"]["RFID no"] = RFID
payload_db["update"]["$set"]["Last tapped time"]["$date"] = curTime
payload_db["update"]["$set"]["Registration time"]["$date"] = curTime

# Convert dictionary to JSON string
JSONText = "[" + json.dumps(payload_db) + "]"

# Publish JSON string to the topic
topic_dbupdate = "python/dbupdate/RFID Chips"
# client.publish(topic, f"[{JSONText}]")

def arbeit():
    topic = topic_dbupdate
    payload = input(JSONText)

    if (topic == "arduino/rfid"):
        assert (payload == "REGISTRATION") \
            | (payload == "CHECK-IN") \
            | (payload == "DEREGISTRATION")
    
    elif (topic == "arduino/checkout"):
        regex = re.compile("[0-9]{2,2}/[A-Z]/[0-9]{2,2}")
        assert regex.match(payload)

    elif (topic == "python/rfid"):
        regex = re.compile("[0-9]?")
        assert (payload == "RFID REGISTERED") \
        | (payload == "The RFID has not been registered yet.") \
        | (regex.match(payload))
    
    elif (topic == "arduino/rack/checkin"):
        regex = re.compile("[0-9]?")
        assert regex.match(payload)

    elif (topic == "arduino/rack/registration"):
        regex = re.compile("[0-9]?")
        assert regex.match(payload)

    elif (topic == "python/rack"):
        assert payload == "WRONG REMOVAL :("

    return topic, payload

#topic, payload = arbeit()
topic = topic_dbupdate
payload = JSONText

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

def on_msg(client, userdata, mid, reason_code, properties):
    print(mid, "published")


def on_connect2(client, userdata, flags, rc, properties):
    #print("Connection")
    if rc == 0:
        # messagebox.showinfo(title="Connected to MQTT", 0message="Connected to MQTT broker with code " + str(rc))
        print("Connected to MQTT broker with code " + str(rc))
    else:
        print("Error connecting to MQTT broker with code" + str(rc))


client.on_connect = on_connect2
client.on_publish = on_msg
client.username_pw_set(USER, PASS)
client.tls_set(tls_version=TLS_VERSION)
client.connect(URL, PORT)

client.loop_start()
client.publish(topic, payload, qos=1)
#print("finished publishing")
client.loop_stop()