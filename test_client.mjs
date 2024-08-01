import express from 'express';
import { createServer } from 'node:http';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { Server } from 'socket.io';

const app = express();
const server = createServer(app);
const io = new Server(server);

const __dirname = dirname(fileURLToPath(import.meta.url));

app.get('/', (req, res) => {
  res.send("dfghjk")
});

io.on('connection', (socket) => {
  console.log('a user connected');
});

io.on("JOIN", (mes) => {
	console.log(mes)
})

io.on("PING ASLB", (mes) => {
	console.log(mes)
})

server.listen(3000, () => {
  console.log('server running at http://localhost:3000');
});