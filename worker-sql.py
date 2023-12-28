import json
import paho.mqtt.client as mqtt
import mysql.connector
from datetime import datetime
from dotenv import load_dotenv
import os

load_dotenv()

MQTT_BROKER = os.getenv('MQTT_BROKER_ADDRESS')
MQTT_PORT = int(os.getenv('MQTT_PORT', 1883))
MQTT_TOPIC = os.getenv('MQTT_TOPIC', 'sensor')
MQTT_USERNAME = os.getenv('MQTT_USERNAME')
MQTT_PASSWORD = os.getenv('MQTT_PASSWORD')

# MySQL connection details
db_config = {
    'user': os.getenv('SQL_USER'),
    'password': os.getenv('SQL_PASSWORD'),
    'host': os.getenv('SQL_URI'),
    'database': os.getenv('SQL_URI'),
}

def save_to_database(sensor_type, data):
    conn = mysql.connector.connect(**db_config)
    cursor = conn.cursor()
    query = "INSERT INTO {} (data, createdAt) VALUES (%s, %s)".format(sensor_type)
    cursor.execute(query, (json.dumps(data), datetime.now()))
    print(f"Success Insert into {sensor_type}")
    conn.commit()
    cursor.close()
    conn.close()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        message = json.loads(msg.payload.decode())
        sensor_type = message['type']
        data = message['data']
        save_to_database(sensor_type, data)
    except json.JSONDecodeError:
        print("Error decoding JSON")

client = mqtt.Client()
client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)  
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()  