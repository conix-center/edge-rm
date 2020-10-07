FROM node

RUN git clone https://github.com/pjreddie/darknet
RUN cd darknet && make

COPY yolov3.weights ./darknet/

COPY classify.sh ./
RUN chmod +x ./classify.sh

ENTRYPOINT ./classify.sh
