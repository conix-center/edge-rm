FROM alpine

RUN apk --no-cache add curl

COPY hello.sh ./

RUN chmod +x ./hello.sh

ENTRYPOINT ./hello.sh