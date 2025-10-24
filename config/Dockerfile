# filepath: ./Dockerfile
FROM ubuntu:22.04

# Install dependencies if needed
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential gdbserver valgrind make systemd-coredump

# Set the working directory to /workspace
WORKDIR /workspace

# Keep container running
CMD ["tail", "-f", "/dev/null"]
