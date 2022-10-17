## Example Yokan/Poesie composition

This repository contains an example of a [Bedrock](https://mochi.readthedocs.io/en/latest/bedrock.html)
configuration that spins up a Yokan provider for key/value storage and a Poesie provider for
embedding a Python interpreter.

The *test.cpp* file shows an example of first putting a key/value pair into the database
using the Yokan API, then using the Poesie API to send a python code that retrieves said value.

### Building the code

The *spack.yaml* file in this repository allows creating a Spack environment with the
required dependencies as follows, from inside the cloned repository (note that you must have installed
[mochi-spack-packages](https://github.com/mochi-hpc/mochi-spack-packages) as
instructed [here](https://mochi.readthedocs.io/en/latest/installing.html#installing-spack-and-the-mochi-repository)).

```
$ spack env create -d .
$ spack env activate .
$ spack install
```

You can then build the code as follows.

```
mkdir build
cd build
cmake ..
make
```

### Running the code

Open two terminals, making sure the Spack environment is activated in both.
In the first termina, run the Bedrock daemon as follows.

```
$ bedrock na+sm -c src/config.json -v trace
```

Make a note of the address that the Bedrock server is reporting to be running on
(e.g., `na+sm://15865-0`).

In the second, run the `yokan-poesie-test` executable with the address as
command-line argument, as follows.

```
$ ./yokan-poesie-test na+sm://15865-0
```

If everything goes well, the test program will print "some_value" before
terminating.
