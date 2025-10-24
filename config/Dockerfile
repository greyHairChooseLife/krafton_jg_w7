# filepath: ./Dockerfile
FROM ubuntu:22.04

# Install dependencies if needed
# install libc6-dev-i386 hence it provides /usr/include/bits/wordsize.h which is required building mdriver
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential gdbserver valgrind make systemd-coredump libc6-dev-i386

# Set the working directory to /workspace
WORKDIR /workspace

# Keep container running
CMD ["tail", "-f", "/dev/null"]
