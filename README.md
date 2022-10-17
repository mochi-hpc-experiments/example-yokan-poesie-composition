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
$ mkdir build
$ cd build
$ cmake ..
$ make
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

### Step-by-step explanation

The *config.json* file describes how our service should look like on one node.
Our service contains a [Yokan](https://mochi.readthedocs.io/en/latest/yokan.html)
provider, which is the component that provides key/value storage.
In the configuration of this component, we add a database called *my_kv_store*,
of type *map*. This type corresponds to an in-memory, ordered map.

The configuration then lists a Poesie provider, which is the component that will
provide us with an embedded Python interpreter. Its configuration lists
*my_python_vm* for this purpose.

In the configuration of this VM, the *preamble* entry lets us provide some code
to execute upon initializing the VM (i.e., when the server starts). We take
advantage of this preamble to import `pyyokan_client`, the Python binding for
Yokan's client library, and create the `my_kv_store` variable of type
`pyyokan_client.Database` to be a handle to the database managed by Yokan.
The `__mid__` and `__address__` variables are added by Poesie automatically
and respectively represents the Margo instance in use and the process' own
address.

Moving over to the *test.c* file, the `write_with_yokan` function shows
an example of using the Yokan C API to look up the database by its name
to get its id, building a `yk_database_handle_t` to interact with the database,
then putting "some_value" associated with the key "my_key" into the database.

The `read_with_poesie` function uses the Poesie C API to create a
`poesie_provider_handle_t`, look up the VM id by its name, then
send over some Python code to be executed on the server.
The Python code in question uses the `my_kv_store` variable, initialized
in the preamble of the VM, to interact with the local database using
Yokan's Python API. Here it first gets the length of the value associated
with "my_key", before actually fetching it into a `bytearray` buffer.

The `__poesie_output__` variable is a special variable that the VM will
lookup after executing the user code. Any object placed in this variable
will be transformed into a string (using the object's `__str__` method),
before being sent back to the caller in as output. Poesie handles
execution results this way because `return` cannot be called outside of
functions. In the present exemple, the content of the key, which is a
`bytearray`, is transformed into a unicode string using `.decode("utf-8")`.

Note that if an exception is raised from the user code, Poesie will
ignore the content of `__poesie_output__` and return the exception
(converted into a string) as output instead.
