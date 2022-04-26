# syntax=docker/dockerfile:1

FROM python:3.8-slim-buster

WORKDIR /app

COPY requirements.txt requirements.txt
# python 3.8-slim-buster is based on Debian:
RUN apt-get update && apt-get install -y git

RUN pip3 install -r requirements.txt

COPY . .

EXPOSE 8080
EXPOSE 5683

ENV CENTRAL_HOST=0.0.0.0
ENV CENTRAL_PORT=5683
ENV CENTRAL_API_PORT=80

CMD python3 ./master.py \
 --host ${CENTRAL_HOST} \
 --api-port ${CENTRAL_API_PORT} \
 --port ${CENTRAL_PORT} \