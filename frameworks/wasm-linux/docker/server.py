import os

from exec_wasm import WasmModule

import flask
app = flask.Flask(__name__)

ActiveModules = {}

@app.route('/')
def hello_world():
    return "Hello, World!"

@app.route('/test', methods=['GET'])
def get_test():
    return flask.jsonify({1: 2})

@app.route('/task', methods=['POST'])
def create_task():
    print("POST /task")

    # ensure query params
    # TODO: need better sanitization
    name = flask.request.args.get('name')
    if name is None or '/' in name:
        return flask.jsonify({'error': "please provide valid task name"})

    # store binary into file
    wasmFilePath = 'wasm/' + name + '.wasm'
    with open(wasmFilePath, 'wb') as f:
        f.write(flask.request.get_data())

    # load module
    ActiveModules[name] = WasmModule()

    # write input to file
    return "OK!"

@app.route('/task/data', methods=['POST'])
def post_data_to_wasm():
    if name is None:
        return flask.jsonify({'error': "please provide valid task name"})
    if name not in ActiveModules:
        return flask.jsonify({"error": 'task not found'})
    ActiveModules[name].run(flask.request.get_data())
    return "OK!"

app.run(host='127.0.0.1',port=3333)