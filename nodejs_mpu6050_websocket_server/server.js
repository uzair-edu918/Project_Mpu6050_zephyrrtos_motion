const WebSocket = require('ws');

const server = new WebSocket.Server({ port: 8080 });

server.on('connection', (socket) => {
  console.log('A new client connected');

  socket.on('message', (message) => {
    try {
        if (Buffer.isBuffer(message)) {
            // Convert buffer to string
            const data = message.toString();
            console.log('Received message:', data);
        }
        // Echo the received message back to all the clients

        server.clients.forEach(client => {
          if (client !== socket && client.readyState === WebSocket.OPEN) {
              client.send(message.toString());
          }
      });
        // socket.send(`Echo: ${message}`);
    }catch (error) {
        console.log(error );

    }
  });

  socket.on('close', () => {
    console.log('A client disconnected');
  });
});

console.log('WebSocket server is running on ws://localhost:8080');
