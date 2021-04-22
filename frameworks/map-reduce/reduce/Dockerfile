FROM node

EXPOSE 3002

COPY server.js ./
COPY package.json ./
COPY scripts /scripts
RUN npm install

ENTRYPOINT node server.js
