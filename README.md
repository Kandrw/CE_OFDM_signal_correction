







## Установка зависимостей

### Linux
    sudo apt-get install build-essential git libxml2-dev bison flex
    
    git clone --branch v0.23 https://github.com/analogdevicesinc/libiio.git
    cd libiio
    mkdir build
    cd build
    cmake -DPYTHON_BINDINGS=ON ..
    make -j$(nproc)
    sudo make install
    sudo ldconfig

    cd ~
    git clone https://github.com/analogdevicesinc/libad9361-iio.git
    cd libad9361-iio
    mkdir build
    cd build
    cmake ..
    make -j$(nproc)
    sudo make install

    cd ~
    git clone --branch v0.0.14 https://github.com/analogdevicesinc/pyadi-iio.git
    cd pyadi-iio
    pip3 install --upgrade pip
    pip3 install -r requirements.txt
    sudo python3 setup.py install





