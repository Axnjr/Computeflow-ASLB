const axios = require('axios');
const os = require('os-utils');
const URL = "http://localhost:4000";
const INTERVAL = 1000; // 60 seconds
const CLIENT_ID = "client_1";
const sendMetrics = async () => {
    os.cpuUsage(async (cpuUsage) => {
        const totalMem = os.totalmem();
        const usedMem = totalMem - os.freemem();
        const memUsage = (usedMem / totalMem) * 100;
        try {
            const response = await axios.post(URL+"/ping", {
                client_id: CLIENT_ID,
                cpuUsage: Number((cpuUsage * 100).toFixed(4)),
                memoryUsage: Number(memUsage.toFixed(4)),
                cpus: os.cpuCount(),
                freeMemoryInMB: Number(os.freemem().toFixed(4)),
                totalMemory: Number(totalMem.toFixed(4)),
                systemUptime: Number(os.sysUptime().toFixed(4))
            });            
            console.log(`Status Code: ${response.status} | CPU: ${(cpuUsage * 100).toFixed(2)}%`);
        } catch (error) {
            await axios.post(URL+"/ping_error", {
                client_id: CLIENT_ID,
                error:error.message
            });
            console.error(`Error: ${error.message}`);
        }
    });
};
sendMetrics();
// setInterval(sendMetrics, INTERVAL);