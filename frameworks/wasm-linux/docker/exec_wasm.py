# Example of instantiating two modules which link to each other.
# Reference: https://github.com/bytecodealliance/wasmtime-py/blob/main/examples/linking.py
import os
from wasmtime import Store, Module, Linker, WasiConfig, WasiInstance

class WasmModule:
    directory = ''
    _call = None

    def __init__(self, path, env={}, function="start"):
        # create base WASM elements
        store = Store()
        config = WasiConfig()

        # create I/O directory
        base = os.path.basename(path)
        self.directory = 'wasm/' + os.path.splitext(base)[0]
        if not os.path.exists(self.directory):
            os.makedirs(self.directory)

        # write input to file
        with open(self.directory + '/input.txt', 'w') as f:
            f.write('')

        # set I/O
        config.stdin_file = self.directory + '/input.txt'
        config.stdout_file = self.directory + '/console.log'
        config.preopen_dir(self.directory, '.')

        # set environment variables
        config.env = env.items()

        # First set up our linker which is going to be linking modules together. We
        # want our linker to have wasi available, so we set that up here as well.
        linker = Linker(store)
        wasi = WasiInstance(store, "wasi_snapshot_preview1", config)
        linker.define_wasi(wasi)

        # Load and compile our two modules
        module = Module.from_file(store.engine, path)

        # And with that we can perform the final link and then execute the module.
        linking1 = linker.instantiate(module)
        self._call = linking1.exports[function]

    def run(self, inputString):
        # append input to input file
        mode = 'ab' if isinstance(inputString, bytes) else 'a'
        with open(self.directory + '/input.txt', mode) as f:
            f.write(inputString)
        # execute wasm export
        self._call()

# module = WasmModule('wasm/minimal.wasm')
# module.run("testing123")