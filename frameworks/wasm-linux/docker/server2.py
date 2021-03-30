# Example of instantiating two modules which link to each other.
# Reference: https://github.com/bytecodealliance/wasmtime-py/blob/main/examples/linking.py

from wasmtime import Store, Module, Linker, WasiConfig, WasiInstance

store = Store()

config = WasiConfig()
config.stdin_file = 'test.txt'
config.stdout_file = 'out.txt'
config.preopen_dir('wasm','.')

# First set up our linker which is going to be linking modules together. We
# want our linker to have wasi available, so we set that up here as well.
linker = Linker(store)
wasi = WasiInstance(store, "wasi_snapshot_preview1", config)
linker.define_wasi(wasi)

# Load and compile our two modules
module = Module.from_file(store.engine, "minimal.wasm")

# And with that we can perform the final link and the execute the module.
linking1 = linker.instantiate(module)
run = linking1.exports["start"]

print(run())
print(run())
print(run())
