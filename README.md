UFC - Universal Flight Connector
==

* This is a very early work in progress! *

UFC is a cross-platform tool to connect devices and other tools to flight simulators.

It defines it's own commands and data references. Each simulator/aircraft can then map these to their specific definitions.

Usage
===
*Coming soon*

Compatibility
===

Platforms
--
* MacOS
* Linux

There's no reason this couldn't work on Windows, but I haven't got a Windows machine to work with.

Flight Simulators
--
At the moment there is only support for:
* X-Plane 12
  * Aircraft using the default datarefs and commands
  * Laminar Research A330
  * ToLISS (Only tested with the A321)
  * FlightFactor 777v2
* Simulated Simulator

Again, there's no reason this couldn't also be used for MS Flight Simulator and P3D, but I don't have a suitable computer.

Devices and Connectors
--
* SDL display
  * A simple PF-style flight display
* Console output
* Example Android device with inputs and outputs

Coming soon:
* WinWingSim FCU

Contributions
==
Contributions of code and aircraft definitions are more than welcome! Send me a PR!

License
==

These libraries are available under the LGPL v3 license. You can use them
in any open or closed source projects, but if you make changes to any of
my code, you have to make those changes available.

ufc is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ufc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
