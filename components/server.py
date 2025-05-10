import subprocess
import time
import webbrowser

SSID = "khaja-esp"
PASSWORD = "12345678"
GATEWAY_URL = "http://192.168.10.1"

def run_cmd(cmd):
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True)
    return result.stdout.strip()

def disconnect_wifi():
    print("Disconnecting any existing Wi-Fi...")
    run_cmd("netsh wlan disconnect")

def set_hosted_network():
    print(f"Setting hosted network: SSID={SSID}")
    run_cmd(f'netsh wlan set hostednetwork mode=allow ssid="{SSID}" key="{PASSWORD}"')

def connect_to_wifi():
    print("Attempting to connect to Wi-Fi...CHECK IF WIFI IS ENABLED")
    run_cmd(f'netsh wlan connect ssid="{SSID}" name="{SSID}"')

def is_connected():
    output = run_cmd("netsh wlan show interfaces")
    return SSID in output and "State" in output and "connected" in output.lower()

def open_browser():
    print(f"Opening browser at {GATEWAY_URL}")
    webbrowser.open(GATEWAY_URL)

# Main logic
disconnect_wifi()
set_hosted_network()

while True:
    connect_to_wifi()
    print("Waiting for connection...")
    time.sleep(5)
    if is_connected():
        print(f"Connected to {SSID}")
        break
    print("Not connected yet. Retrying...")

# Open browser once connected
open_browser()
