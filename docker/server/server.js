const http = require('http');
const fs = require('fs');
const url = require('url');
const { parse } = require('querystring');
const { exec } = require('child_process');

//const hostname = '127.0.0.1';
const hostname = '0.0.0.0';
// const port = 3000;
//const hostname = '172.17.40.64';
const port = process.env.SERVER_PORT;
//const port = 80;

const server = http.createServer((req, res) => {
    // res.statusCode = 200;

    var requestURL = url.parse(req.url, true);

    console.log(requestURL.pathname)

    if(requestURL.pathname.startsWith('/latest')) {
        fs.createReadStream('./latest.jpg').pipe(res);
    } else if(requestURL.pathname.startsWith('/pushprediction')) {
        let body = [];
        req.on('data', (chunk) => {
                body.push(chunk);
        }).on('end', () => {
            body = Buffer.concat(body);
            fs.writeFile('./predictions.jpg',body, function(err, result) {
                if (err) return res.end(err.toString());
                res.end("received predictions!");
            });
        });
    } else if(requestURL.pathname.startsWith('/image')){
        let body = [];
        req.on('data', (chunk) => {
                body.push(chunk);
        }).on('end', () => {
            body = Buffer.concat(body);
            fs.writeFile('./latest.jpg',body, function(err, result) {
                if (err) return res.end(err.toString());
                res.end("received image!");
            });
        });
    } else if(requestURL.pathname.startsWith('/predictions')) {
        try {
            if (fs.existsSync('./predictions.jpg')) {
                return fs.createReadStream('./predictions.jpg').pipe(res);
            }
        } catch(err) {
        }
        return res.end("Predictions not available yet");
    } else {
        res.writeHead(404);
        res.end("How'd you get here?");
    }

});

server.listen(port, hostname, () => {
    console.log(`Server running at http://${hostname}:${port}/`);
});

