import json
import paho.mqtt.client as mqtt
from datetime import datetime
from pymongo import MongoClient
from dotenv import load_dotenv
import os

load_dotenv()

MQTT_BROKER = os.getenv('MQTT_BROKER_ADDRESS')
MQTT_PORT = int(os.getenv('MQTT_PORT', 1883))
MQTT_TOPIC = os.getenv('MQTT_TOPIC', 'sensor')
MQTT_USERNAME = os.getenv('MQTT_USERNAME')
MQTT_PASSWORD = os.getenv('MQTT_PASSWORD')

# MongoDB connection details
# MONGO_URI = os.getenv('MONGO_URI')
# DB_NAME = os.getenv('DB_NAME', 'halo')

# mongo_client = MongoClient(MONGO_URI)
# db = mongo_client[DB_NAME]

# def save_to_database(sensor_type, data):
#     collection = db[sensor_type]
#     document = {
#         "data": data,
#         "createdAt": datetime.now()
#     }
#     collection.insert_one(document)
#     print(f"Success: Inserted data into {sensor_type}")

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        message = json.loads(msg.payload.decode())
        sensor_type = message['type']
        data = message['data']
        # save_to_database(sensor_type, data)
    except json.JSONDecodeError:
        print("Error decoding JSON")

client = mqtt.Client()
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)  
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()  
