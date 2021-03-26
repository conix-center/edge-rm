Assemblyscript test program
==========================

1) Install

```
$ npm install
$ curl https://wasmtime.dev/install.sh -sSf | bash
```

2) Modify

Edit assembly/index.ts

2) Build

```
$ npm run asbuild:minimal
```

3) Run

Pass in directory if your program needs a file

```
$ wasmtime build/minimal.wasm --dir .
```
