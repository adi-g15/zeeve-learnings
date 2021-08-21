# Crypto... Learning

A C++ Application, using the ZeroMQ library (a server & a client)... doing these things...

Run `client --help` for help.

## Building

* Uses the CPM.cmake for dependency management,
* argparse for cli args
* zmq for sockets

To build the complete project (those included, don't take more than few seconds)

```sh
cmake -B build
cmake --build build
```

Then to run:
* Server: `./build/server`
* Client: `./build/client --help`

> 2. Basic study of cryptographic concepts and practising them with either javascript/python/rust libraries or Linux command CLI(sha256sum, base64, openssl),
>
>    Encoding
>
>    Hashing
>
>    Digital Signatures
>
>    Encryption
>

