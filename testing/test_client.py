import requests
import psutil
import time

URL = "http://localhost:4000"
INTERVAL = 60  # seconds
CLIENT_ID = "client_1"

def send_metrics():
    try:
        # CPU usage as a percentage
        cpu_usage = psutil.cpu_percent(interval=1)
        
        # Memory usage in percentage
        memory = psutil.virtual_memory()
        mem_usage = memory.percent
        
        # Gather system metrics
        metrics = {
            "client_id": CLIENT_ID,
            "cpuUsage": round(cpu_usage, 4),
            "memoryUsage": round(mem_usage, 4),
            "cpus": psutil.cpu_count(),
            "freeMemoryInMB": round(memory.available / (1024 * 1024), 4),
            "totalMemory": round(memory.total / (1024 * 1024), 4),
            "systemUptime": round(time.time() - psutil.boot_time(), 4)
        }
        
        # Send metrics
        response = requests.post(f"{URL}/ping", json=metrics)
        print(f"Status Code: {response.status_code} | CPU: {cpu_usage:.2f}%")
    except Exception as error:
        # Send error details
        error_data = {
            "client_id": CLIENT_ID,
            "error": str(error)
        }
        requests.post(f"{URL}/ping_error", json=error_data)
        print(f"Error: {error}")

# Run the metrics function initially
send_metrics()

# Schedule the function to run periodically
while True:
    time.sleep(INTERVAL)
    send_metrics()
