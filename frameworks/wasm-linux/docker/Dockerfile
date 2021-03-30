FROM python:3.8-slim-buster

EXPOSE 3002

RUN curl https://wasmtime.dev/install.sh -sSf | bash
RUN pip install wasmtime

COPY server.js ./
COPY package.json ./
COPY wasm /wasm
RUN npm install

ENTRYPOINT node server.js
