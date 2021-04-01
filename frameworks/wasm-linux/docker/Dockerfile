FROM python:3.8-slim-buster

EXPOSE 3002

RUN curl https://wasmtime.dev/install.sh -sSf | bash
RUN pip install wasmtime
RUN pip install flask

COPY server.py ./
COPY exec_wasm.py ./
COPY wasm /wasm

ENTRYPOINT python server.py
