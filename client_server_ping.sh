#!/bin/bash
sudo npm install forever -g
cat << 'EOF' > ping_lb.js
const axios = require('axios');
const os = require('os-utils');
const URL = "http://[2401:4900:1c20:6e5c:41cf:4da2:5ac2:321a]:4000/ping"; # for dev mode
const INTERVAL = 9,00,000; # 15 Mins, 900 Seconds
const CLIENT_ID = "client_1"; # would assigned based on the db entry id
const sendMetrics = async () => {
    os.cpuUsage(async (cpuUsage) => {
        try {
            const response = await axios.post(URL, {
                client_id: CLIENT_ID,
                systemCheckStatus:"OK",
                cpu: cpuUsage * 100,
                mem: (1 - os.freememPercentage()) * 100,
                cpus: os.cpuCount(),
                memoryInMB: os.freemem()
            });            
            console.log(`Status Code: ${response.status} | CPU: ${(cpuUsage * 100).toFixed(2)}% | Memory: ${memUsage.toFixed(2)}%`);
        } catch (error) {
            await axios.post(URL, {
                client_id: CLIENT_ID,
                systemCheckStatus:"ERROR",
            });
            console.error(`Error: ${error.message}`);
        }
    });
};
setInterval(sendMetrics, INTERVAL);
EOF
sudo npm install axios os-utils
chmod +x ping_lb.js
forever start ping_lb.js