import pymongo
import datetime
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi

uri = "mongodb+srv://oscarsun:<igemhardware24>@wetlab-fridge-managemen.bzs7csj.mongodb.net/?retryWrites=true&w=majority&appName=Wetlab-Fridge-Management"
client = MongoClient(uri, server_api = ServerApi('1'))
db = client["wet_lab_fridge"]
collection = db["samples"]

try:
    client.admin.command('ping')
    print("Pinged your deployment. You successfully connected to MongoDB!")
except Exception as e:
    print(e)


collection.insert_one({"RFID":"12345195",
                       "Description":"plasmid", 
                       "Check-In Date": datetime.datetime.now().replace(microsecond=0), 
                       "Location": "A2"})

# tod = datetime.datetime.now()
# d = datetime.timedelta(days = 2)
# correct_check_in_time = tod - d
# collection.insert_one({"RFID":"12345196", 
#                        "Description":"toxin-antitoxin experiment", 
#                        "Check-In Date": correct_check_in_time.replace(microsecond=0), 
#                        "Location": "B3"})

# d = datetime.timedelta(days = 1)
# correct_check_out_time = tod + d
# collection.insert_one({"RFID":"12345197", 
#                        "Description":"OriR experiment", 
#                        "Check-In Date": datetime.datetime().replace(microsecond=0), 
#                        "Last Checked Out Date": correct_check_out_time.replace(microsecond = 0),
#                        "Location": "A4"})
