FROM node

RUN git clone https://github.com/AlexeyAB/darknet
RUN cd darknet && make

RUN cd darknet && wget https://raw.githubusercontent.com/AlexeyAB/darknet/master/cfg/yolov4-tiny.cfg
RUN cd darknet && wget https://github.com/AlexeyAB/darknet/releases/download/darknet_yolo_v4_pre/yolov4-tiny.weights

COPY classify.sh ./
RUN chmod +x ./classify.sh

ENTRYPOINT ./classify.sh
