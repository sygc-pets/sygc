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
Run `./bin/SYGC` to run GC protcol on a netlist. A large library of pre-compiled netlists are provided in `sygc/netlists_pi`. These netlists are required for the programming interface. It is also possible to run them directly through the GC back-end with the following command. 

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

## Program Interface

- [millionaire.cpp](exec/millionaire.cpp) presents a minimal working example with the programming interface of SYGC. 
    It is an implementation of the Yao's Millionaire's Problem through SYGC. 
    In the following, we explain different components of the code.

    First, instantiate the program interface for the desired security model. 
    This is the only place of the code where the security model is specified. 
    `party` is either `ALICE` (garbler) or `BOB` (evaluator) -- taken as input from the terminal. 

    ```cpp
    #if SEC_SH
        SYGCPI_SH* TGPI = new SYGCPI_SH(io, party);
    #else
        SYGCPI* TGPI = new SYGCPI(io, party);
    #endif
    ```
    
    Then we take respective input from the terminal. 
    This part is specific to this particular problem, not dependent on the SYGC structure. 
    ```cpp
    if (TGPI->party == ALICE) cin >> a;
    else cin >> b;
    ```
    
    The next task is to create secret variables corresponding to the inputs.
    This is done through three steps as shown: 
    (i) regsiter the respective values (the bit width of each variable is set at this step), 
    (ii) generate and transfer the corresponding labels, 
    (iii) retreieve the labels to the respective secret variable. 
    Note that in the code snippet above, each party inputs only one number.
    In the code snippet that follows, 
    the value of `a` is read only if the party is ALICE and 
    the value of `b` is read only if the party is BOB;
    otherwise they are ignored by the program.
    The first argument of `TGPI->TG_int_init( ,  ,  )` refers to the *owner* of that particular secret variable. 
    
    ```cpp
    auto a_x = TGPI->TG_int_init(ALICE, bit_width, a);
    auto b_x = TGPI->TG_int_init(BOB, bit_width, b);

    TGPI->gen_input_labels();

    TGPI->retrieve_input_labels(a_x, ALICE, bit_width);
    TGPI->retrieve_input_labels(b_x, BOB, bit_width);
    ```
    
    Once the input labels are created and transferred, we can perform computaions on them.
    In this particular example, 
    we will compare the values of the two secret variables `a_x` and `b_x` and 
    store the Boolean result in a new secret variable `res_x`.
    Note that the declaration of `res_x` do not specify a owner since this is a secret variable, 
    the value of which is *shared* between ALICE and BOB.
    Moreover, the labels of `res_x` are computed through GC, 
    therefore its declaraion does not need to be followed by `gen_input_labels()`
    
    ```cpp
    auto res_x = TGPI->TG_int(1);
    TGPI->lt(res_x, a_x, b_x, bit_width);
    ```
    
    Since `res_x` is the final output in this example, ALICE and BOB combines the shares to reveal its input.
    ```cpp
    int64_t res = TGPI->reveal(res_x, 1, false);
    ```

    To run the program,

    On Alice's terminal

    ```
    ./bin/millionaire
    ```

    On Bob's terminal

    ```
    ./bin/millionaire -k 2
    ```

    It will prompt to input the respective wealths. Upon receiving the input, the output will be displayed. 

- To see the usage of all the available functions of SYGC, have a look at [unit_test](exec/unit_test.h). To run the unit tests follow instructions at [Test and Evaluation](/README.md#test-and-evaluation).

- [cnn_layers](sygc/cnn_layers.h) presents more complex examples based on the programming interface. It provides implementations of basic building blocks required for CNN inference. Currently it supports only the honest-but-curious security model. 

- Please see [LeNet.cpp](exec/LeNet.cpp) for an example of CNN inference through SYGC. To run the program,

    On Alice's terminal

    ```
    ./bin/LeNet < test/LenetSmall_mnist_img_A.inp
    ```

    On Bob's terminal

    ```
    ./bin/LeNet -k 2 < test/LenetSmall_mnist_img_B.inp
    ```
    [LenetSmall_mnist_img_A.inp](test/LenetSmall_mnist_img_A.inp) holds the trained weight parameters of the CNN and [LenetSmall_mnist_img_B.inp](test/LenetSmall_mnist_img_B.inp) holds an example image. This particular image belongs to class 2. 
    



