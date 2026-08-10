from flask import Flask, request
import serial
import threading

PORT = '/dev/ttyACM0'
app = Flask(__name__)
ser = serial.Serial(PORT, 9600)

@app.route('/update', methods=['POST'])
def update():
    data = request.get_json()
    message = data.get('message', '')
    ser.write(f"{message}\n".encode())
    print(message)
    return "OK", 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
