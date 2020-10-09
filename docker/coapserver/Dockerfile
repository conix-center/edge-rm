FROM node

EXPOSE 3002

COPY server.js ./
COPY package.json ./
RUN npm install

ENTRYPOINT node server.js
