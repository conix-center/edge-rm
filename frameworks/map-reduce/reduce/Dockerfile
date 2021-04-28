FROM node

EXPOSE 3002

COPY coap.js ./
COPY package.json ./
COPY scripts /scripts
RUN npm install

ENTRYPOINT node coap.js
