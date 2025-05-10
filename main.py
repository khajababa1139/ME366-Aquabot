import subprocess

def main():
    # Run the first Python file in the 'components' folder
    subprocess.run(["python", "components/server.py"])

    # Run the second Python file in the 'components' folder
    subprocess.run(["python", "components/rpy.py"])

    print("Both files have finished executing.")

if __name__ == "__main__":
    main()
