# syntax=docker/dockerfile:1

FROM python:3.8-slim-buster

WORKDIR /app

COPY requirements.txt requirements.txt
# python 3.8-slim-buster is based on Debian:
RUN apt-get update && apt-get install -y git

RUN pip3 install -r requirements.txt

COPY . .

EXPOSE 8080
# not sure this is needed:
EXPOSE 5683

CMD [ "python3", "./master.py", "--host=0.0.0.0"]
