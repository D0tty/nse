# What is this ?

This is a very simple reimplementation of `nsenter(1)`

# Compile

Compiling is very simple (I don't even think this is necessary)

```
make
```

# Usage

## Basic usage

`./mini_nsenter <PID> <COMMAND>`

## Step by step

You'll need two separate console.

One to run a simple docker command:

```
docker run -it --rm bash top
```

Then in the other one 

```
sudo ./mini_nsenter `pidof top` /bin/sh
```

or if you want to be able to use the binaries inside this namespace:

```
PATH=/bin sudo ./mini_nsenter `pidof top` /bin/sh
```