import subprocess

def main():
    subprocess.run(["python", "components/server.py"])
    subprocess.run(["python", "components/rpy.py"])

    print("Both files have finished executing.")

if __name__ == "__main__":
    main()
