
git clone https://github.com/sygc-pets/emp-ot.git
cd emp-ot
cmake . -DCMAKE_INSTALL_PREFIX=../include
make -j 
make install -j 
cd ..