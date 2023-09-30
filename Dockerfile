FROM mcr.microsoft.com/devcontainers/base:ubuntu

RUN sudo apt update &&\
    sudo sudo apt install build-essential -y &&\
    sudo apt install libfftw3-dev -y &&\
    sudo apt install libeigen3-dev -y &&\
    sudo apt install liblua5.4-dev -y &&\
    sudo apt install zlib1g-dev -y &&\
    sudo apt install libpng-dev -y &&\
    sudo apt install libfreetype-dev -y &&\
    sudo apt install libwxgtk3.0-gtk3-dev -y