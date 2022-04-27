# syntax=docker/dockerfile:1

FROM python:3.8-slim-buster

WORKDIR /app

COPY requirements.txt requirements.txt
RUN apt-get update && apt-get install -y gcc

RUN python3 -m pip install -r requirements.txt

COPY . .

COPY --from=docker:dind /usr/local/bin/docker /usr/local/bin/

ENV CENTRAL_API_PORT=80
ENV CENTRAL_HOST=localhost
ENV CONFIG="./config.yaml"

CMD python3 ./agent.py --port=${CENTRAL_API_PORT} --host=${CENTRAL_HOST} --config=${CONFIG}