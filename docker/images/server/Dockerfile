FROM node

EXPOSE 3002

COPY server.js ./
COPY classify.sh ./
COPY latest.jpg ./

RUN git clone https://github.com/pjreddie/darknet
RUN cd darknet && make

COPY yolov3.weights ./darknet/

RUN chmod +x ./classify.sh

ENTRYPOINT node server.js
