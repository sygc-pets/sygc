# SYGC

### Smart, efficient and scalable Yao's Garbled Circuit (GC)

## Installation

- For first time set up, run `./setup.sh`. 
It installs the required packages. 
It also clones and makes the required libraries -
[emp-tool](https://github.com/sygc-pets/emp-tool) and 
[emp-ot](https://github.com/sygc-pets/emp-ot),
which are adapted from the [emp-toolkit](https://github.com/emp-toolkit).

- After the set up, run `./config.sh` followed by `make` if required to build SYGC.

## Test and Evaluation
- Run `./bin/test_SYGC.sh` to test build for correctness.
It runs evaluation on precompiled netlists.

```
./bin/test_SYGC.sh              # test GC back-end in malicious setting
./bin/test_SYGC.sh --sh         # test GC back-end in semi-honest setting
./bin/test_SYGC.sh --pi         # test programming interface in malicious setting
./bin/test_SYGC.sh --pi --sh    # test programming interface in semi-honest setting
```

- Run `./bin/eval_SYGC` to evaluate the GC back-end on benchmark functions.

```
./bin/eval_SYGC
    -h [ --help ]                           produce help message
    -p [ --port ] arg (=1234)               socket port
    -s [ --server_ip ] arg (=127.0.0.1)     server's IP.
    -i [ --input ] arg (=0)                 hexadecimal input (little endian),
                                            without init.
    -j [ --init ] arg (=0)                  hexadecimal init (little endian).
    -r [ --repeat ] arg (=1)                number of times to repeat the run
    -f [ --file ] arg (=bin/benchmarks.txt)
                                            netlist, cycles, output_mode are read from this file
    -t [ --num_eval ] arg (=10)             number of runs to average evaluation over
    --sh                                    semi-honest security model
```

## Execute GC Back-end
Run `./bin/eval_SYGC` to run GC protcol on a netlist. A large library of pre-compiled netlists are provided in `sygc/netlists_pi`

```  
 ./bin/SYGC   
    -h [ --help ]                           produce help message
    -k [ --party ] arg (=1)                 party id: 1 for garbler, 2 for evaluator
    -n [ --netlist ] arg (=sygc/netlists_pi/add_8bit.emp.bin)
                                            circuit file address.
    -p [ --port ] arg (=1234)               socket port
    -s [ --server_ip ] arg (=127.0.0.1)     server's IP.
    -i [ --input ] arg (=0)                 hexadecimal input (little endian), without init.
    -j [ --init ] arg (=0)                  hexadecimal init (little endian).
    -c [ --cycles ] arg (=1)                number of cycles to run
    -r [ --repeat ] arg (=1)                number of times to repeat the run
    -m [ --output_mode ] arg (=0)           0: reveal output at every cycle
                                            1: reveal output at last cycle
                                            2: transfer output to next netlist at every cycle
                                            3: transfer output to next netlist at last cycle
    -f [ --file ] arg                       netlist, input, init, cycles, repeat, output_mode
                                            are read from this file,
                                            ignores command line inputs for these fields
    --sh                                    semi-honest security model 
    --oo                                    report output only
```
The total execution runs for `cycles*repeat` times. The DFFs reset to `init` after every `cycles` run. 


