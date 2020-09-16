FROM arm32v6/python:3.7-alpine

RUN apk update
RUN apk add raspberrypi
RUN apk --no-cache add curl

COPY hello.sh ./

RUN chmod +x ./hello.sh

ENTRYPOINT ./hello.sh