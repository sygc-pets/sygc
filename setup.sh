cd ..
mkdir -p include

echo "---------- installing dependencies ----------"
sudo ./sygc/install_scripts/install_dependencies.sh

echo echo "---------- installing emp-tool ----------"
./sygc/install_scripts/install_emp-tool.sh

echo echo "---------- installing emp-ot ----------"
./sygc/install_scripts/install_emp-ot.sh

echo echo "---------- building sygc ----------"
cd sygc
cmake . -DCMAKE_INSTALL_PREFIX=../include
make -j 