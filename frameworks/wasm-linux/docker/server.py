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
    return flask.jsonify({'hello': 'world'})

##
## POST /task
## QUERY: 'name' = the task ID
## BODY: a wasm binary file
## 
## stores the posted wasm binary and loads a module with name
##
@app.route('/task', methods=['POST'])
def create_task():
    print("POST /task")

    # ensure query params
    # TODO: need better sanitization
    name = flask.request.args.get('name')
    if name is None or '/' in name:
        return flask.jsonify({'error': "please provide valid task name"})

    exportFunction = flask.request.args.get('func') or "start"

    # store binary into file
    wasmFilePath = 'wasm/' + name + '.wasm'
    with open(wasmFilePath, 'wb') as f:
        f.write(flask.request.get_data())

    # load module
    ActiveModules[name] = WasmModule(wasmFilePath, exportFunction)

    # write input to file
    return "OK!"

##
## POST /task/data
## QUERY: 'name'
## BODY: data to send to wasm task
##
## executes the module's function export with BODY as console input
##
@app.route('/task/data', methods=['POST'])
def post_data_to_wasm():
    name = flask.request.args.get('name')
    if name is None:
        return flask.jsonify({'error': "please provide valid task name"})
    if name not in ActiveModules:
        return flask.jsonify({"error": 'task not found'})
    try:
        ActiveModules[name].run(flask.request.get_data())
        return flask.send_file('wasm/' + name + '/console.log')
    except Exception as e:
        return e

PORT = os.environ.get('PORT') or 3002

app.run(host='0.0.0.0',port=PORT)