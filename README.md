UFC - Universal Flight Connector
==

UFC is a cross-platform library and tool that connects devices and other tools to flight
simulators and provides a simple cross-sim library for making connecting to flight
simulators easy.

This project also provides a plugin for X-Plane 12 that allows you to easily connect devices
such as the WinWing FCU to any aircraft *.

UFC defines its own Sim/Aircraft-agnostic data and command sets
that mean device and aircraft definitions will work with everything else.

There is also basic support for accessing to navigation data provided by the simulator
such as airport, fixes and procedures. For X-Plane, this also includes Navigraph data.

\* Many aircraft will work out of the box with the default configuration, others
require a definition file.

# Installation

## Required Libraries
* yaml-cpp
* hidapi (libusb version on Linux)
* lua

### Mac
```
$ brew install hidapi yaml-cpp lua 
```

### Ubuntu
```
$ apt install libyaml-cpp-dev libhidapi-dev lua5.4 liblua5.4-dev
```

## Build
```
$ git submodule update --init
$ cmake -B build
$ cmake --build build
```


# Usage

## X-Plane Plugin
I will look in to packaging this up properly, but for now:
* Create ${PATH_TO_XPLANE}/Resources/plugins/ufc
* Copy the libufc-plugin.so/dylib to the plugin directory as mac.xpl or lin.xpl (Depending on platform)
* Copy the contents of the data/x-plane directory to the plugin directory
* ... And that's it! No configuration is required

## ufctool
ufctool is a command line tool for using UFC to connect simulators with devices.

```
Usage: ufctool [OPTIONS]
-c, --config  Specify configuration file. (Default: $HOME/.config/ufc.yaml)
-d, --data    Specify data directory (Defaults to configured directory)
-s, --source  Data Source type (Overrides configured source)
-h, --help    This help text
```

### Configuration
The configuration file uses YAML. I'm removing the need for configuration slowly,
it should figure out most things for itself, except for the X-Plane installation
directory.

The default path for the configuration is $HOME/.config/ufc.yaml

```aiignore
# Where is the data installed?
dataDir: /usr/local/share/ufc
# Default Data Source
dataSource: XPlane

# X-Plane specific 
xplane:
  path: "/Path/To/X-Plane 12"
  host: 127.0.0.1
  port: 49000
```


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

You can connect to X-Plane as a separate process using ufctool or using the plugin. Using the ufctool, you can connect from a different machine which means devices connected to a different computer can still be used in X-Plane.

Aircraft:
  * All aircraft using the default data refs and commands
  * Laminar Research A330
  * ToLiss (Only tested with the A321)
  * FlightFactor 777v2
  * Felis 747-200
  * ... And more coming! (Insert another plea for Pull Requests!)

### Simulated Simulator
A simple simulator is provided for testing. It returns dummy data and responds to some commands.

### Clock
Use your radio or WinWing FCU as a clock :-)

Try:
```aiignore
./ufctool --source Clock
```

### Others
UFC is designed so it can also work with MS Flight Simulator and P3D, but I don't have
a Windows machine available. (Again, Pull Requests are welcome!)


## Devices and Connectors
* WinWingSim FCU
* An example USB HID radio panel
* Console output
  * For testing, disabled by default

Coming soon:
* Generic USB HID Devices
  * Map HID/Joystick devices using configuration files

# How it works
## Data and Commands
UFC defines its own abstract commands and data references. Each simulator/aircraft
can then map these to their specific definitions.

Devices can then read from this common structure to present data and send commands without any simulator/aircraft specific changes.

For X-Plane, the aircraft configuration can be found [here](https://github.com/geekprojects/ufc/tree/main/data/x-plane/aircraft).

Commands can be sent directly to the simulator or handled by a Lua script.


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
