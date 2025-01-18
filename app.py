from flask import Flask, jsonify, render_template, request
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db

from flask_socketio import SocketIO
import paho.mqtt.client as mqtt
import json


from datetime import datetime

app = Flask(__name__)
socketio = SocketIO(app)

cred = credentials.Certificate('sensorstation-63898-firebase-adminsdk-qku2d-727c5da021.json')

firebase_admin.initialize_app(cred, {

    'databaseURL': 'https://sensorstation-63898-default-rtdb.firebaseio.com/'

})


sensor_data = {
    "luminosidade": 0,
    "umidade": 0,
    "temperatura": 0,
    "pressao": 0,
    "altitude": 0,
    "sensacao_termica": 0,
    "data_hora": ""
}


def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode('utf-8')

        data = json.loads(payload)

        luminosidade = data["Luminosidade"]
        umidade = data["Umidade"]
        temperatura = data["Temperatura"]
        pressao = data["Pressão"]
        altitude = data["altitude"]
        sensacao_termica = data["SensacaoTermica"]

        data_hora = datetime.now().strftime("%d/%m/%Y %H:%M:%S") 


        print(f"Luminosidade: {luminosidade}, Umidade: {umidade}, Temperatura: {temperatura}, Pressão: {pressao}, Altitude: {altitude}, Sensação Térmica: {sensacao_termica}")

        global sensor_data
        sensor_data = {
            "luminosidade": luminosidade,
            "umidade": umidade,
            "temperatura": temperatura,
            "pressao": pressao,
            "altitude": altitude,
            "sensacao_termica": sensacao_termica,
            "data_hora": data_hora
        }
        socketio.emit('update_data', sensor_data)
        db.reference('/sensorReadings').push(sensor_data) 
    except json.JSONDecodeError:
        print(f"Erro ao decodificar JSON: {msg.payload}")
    except KeyError as e:
        print(f"Erro: Chave ausente no JSON - {e}")
mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message
mqtt_client.connect("10.128.77.242", 1883, 60)
mqtt_client.subscribe("sensores/dadosTrabalho2342345234523452345234523452345234")
mqtt_client.loop_start()


@app.route("/mqtt-ativacao", methods=["POST"])
def enviar_notificacao():
    try:
        data = request.json
        ativacao = data.get("ativacao", 0)

        mensagem = json.dumps({"Ativacao": ativacao})
        mqtt_client.publish("sensores/ativacaoTrabalho2342345234523452345234523452345234", mensagem)

        print(f"Mensagem enviada ao broker: {mensagem}")
        return jsonify({"status": "success", "message": "Mensagem enviada ao broker com sucesso"})
    except Exception as e:
        print(f"Erro ao enviar mensagem ao broker: {e}")
        return jsonify({"status": "error", "message": str(e)}), 500


# Rota principal
@app.route("/")
def index():
    return render_template("index.html")

if __name__ == "__main__":
    socketio.run(app, host="0.0.0.0", port=5000)
