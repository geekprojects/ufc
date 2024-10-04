UFC - Universal Flight Connector
==

^ [!WARNING]
^ This is a very early work in progress!

UFC is a cross-platform library and tool that connects devices and other tools to flight
simulators, and provides a simple cross-sim library for making connecting to flight
simulators easy. UFC defines its own Sim/Aircraft-agnostic data and command sets
that mean device and aircraft definitions will work with everything else.


# Installation

## Required libraries
* yaml-cpp
* hidapi (libusb version on Linux)
* lua

## Build
```
$ cmake -B build
$ cmake --build build
```


# Usage

## ufctool
ufctool is the main tool for using UFC.

```
Usage: ufctool [OPTIONS]
-c, --config  Specify configuration file. (Default: $HOME/.config/ufc.yaml)
-d, --data    Specify data directory (Defaults to configured directory)
-s, --source  Data Source type (Overrides configured source)
-h, --help    This help text
```

### Configuration
The configuration file uses YAML.


# Compatibility

## Platforms
* MacOS
* Linux

There's no reason UFC couldn't work on Windows, but I haven't got a Windows machine to
work with. (Pull Requests are welcome!)


## Flight Simulators
At the moment there is only support for:

### X-Plane 12
(And probably 11, but I haven't tested it)

You can connect to X-Plane as a separate process using ufctool but I will also work
on a plugin.

Aircraft:
  * All aircraft using the default data refs and commands
  * Laminar Research A330
  * ToLiss (Only tested with the A321)
  * FlightFactor 777v2

### Simulated Simulator
A simple simulator is provided for testing.

### Others
UFC is designed so it can also work with MS Flight Simulator and P3D, but I don't have
a Windows machine available. (Again, Pull Requests are welcome!)


## Devices and Connectors
* WinWingSim FCU
* SDL display
  * A simple PF-style flight display
* Console output

Coming soon:
* Generic USB HID Devices


# How it works
## Data and Commands
UFC defines its own abstract commands and data references. Each simulator/aircraft
can then map these to their specific definitions.

The ability to use Lua to transform data and commands to and from the common format
is coming soon!


# Contributions
Contributions of code and aircraft definitions are more than welcome! Send me a PR!


# License

These libraries are available under the LGPL v3 license. You can use them
in any open or closed source projects, but if you make changes to any of
my code, you have to make those changes available.

UFC is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

UFC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
