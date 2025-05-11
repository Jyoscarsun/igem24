from pymongo import MongoClient

# QUARANTINE THIS - THIS IS SECRET INFO
DB_URL = "mongodb+srv://hardware:hardwareuser@wetlab-fridge-managemen.bzs7csj.mongodb.net/"
MQTT_URL = "ddaa1f93a61a4de7aaf7465c2984155c.s1.eu.hivemq.cloud"
PORT = 8883
USER = "igemfridge"
PWD = "Suffering1"
# QUARANTINE THIS - THIS IS SECRET INFO

client = MongoClient(DB_URL)
db = client["igemhardware"]

try:
    print("Attempting to ping")
    client.admin.command('ping')
    print("Pinged your deployment. You successfully connected to MongoDB!")
except Exception as e:
    print(e)

RFID_Collection = db["RFID Chips"]
Rack_Entry_Collection = db["Rack Well"]
Rack_Entry_Collection.update_one(
                {'Rack': "Rack", 'Well': "Well"},
                dat,
            )


