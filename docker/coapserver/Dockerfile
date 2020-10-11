FROM node

EXPOSE 3002

COPY server.js ./
COPY package.json ./
COPY files /files
RUN npm install

ENTRYPOINT node server.js
