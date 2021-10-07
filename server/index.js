const http = require("http");

const requestListener = async function (req, res) {
  console.log(req.url);

  const buffers = [];

  for await (const chunk of req) {
    buffers.push(chunk);
  }

  const data = Buffer.concat(buffers).toString();

  res.writeHead(200);
  console.log(req.headers);
  console.log(new Date().toISOString());
  console.log(data.toString());
  res.end("OK");
};

const server = http.createServer(requestListener);
server.listen(3000);
