from flask import Flask, render_template, Response
import paho.mqtt.client as mqtt
import cv2
import threading

app = Flask(__name__)

# Configura la conexión MQTT y camara
cap = cv2.VideoCapture('http://192.168.41.123:81/stream')
mqtt_broker = "192.168.68.229"
topicVel = "car/vel"
topicDir = "car/dir"
topicMov = "car/mov"
topicAutomatico = "automatico"

def on_message(client, userdata, message):
    print(message.payload.decode())

client = mqtt.Client()
client.connect(mqtt_broker)
client.subscribe(topicVel)
client.subscribe(topicDir)
client.subscribe(topicMov)
client.subscribe(topicAutomatico)
client.on_message = on_message
client.loop_start()

@app.route('/')
def index():
    return render_template("index.html")

# Manejo de botones y respuestas al ESP8266 via MQTT #
# VELOCIDADES #
@app.route('/lento', methods=['POST'])
def enviarVel_1():
    client.publish(topicVel, "1")
    return render_template("index.html")
@app.route('/medio', methods=['POST'])
def enviarVel_2():
    client.publish(topicVel, "2")
    return render_template("index.html")
@app.route('/rapido', methods=['POST'])
def enviarVel_3():
    client.publish(topicVel, "3")
    return render_template("index.html")

# DIRECCION #
@app.route('/izquierda', methods=['POST'])
def enviarDir_izq():
    client.publish(topicDir, "izq")
    return render_template("index.html")
@app.route('/derecha', methods=['POST'])
def enviarDir_der():
    client.publish(topicDir, "der")
    return render_template("index.html")

# MOVIMIENTOS #
@app.route('/avanzar', methods=['POST'])
def enviarMov_avanzar():
    client.publish(topicMov, "avanzar")
    return render_template("index.html")
@app.route('/retroceder', methods=['POST'])
def enviarMov_retroceder():
    client.publish(topicMov, "retroceder")
    return render_template("index.html")
@app.route('/reiniciar', methods=['POST'])
def enviarMov_reiniciar():
    client.publish(topicMov, "reiniciar")
    return render_template("index.html")



if __name__ == '__main__':
    t1 = threading.Thread(target=app.run, kwargs={'host':'0.0.0.0', 'port':8080, 'debug':False})
    t1.start()

    while(True):
        ret, frame = cap.read()
        cv2.imshow('Imagen desde la camara',frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            cv2.destroyAllWindows()
            break
