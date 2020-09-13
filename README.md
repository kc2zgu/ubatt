ubatt
=====

ubatt is a command line tool to display battery information (charging
status, mains power connection, charge level) from the UPower daemon.

Features
--------

* Show current battery level and charging status
* Show line power status
* Show UPS battery status (not implemented)
* Print charge level on standard output for parsing in scripts
* Return charge level as exit code
* Return charging status as exit code
* Return line power status as exit code (not implemented)

Building
--------

ubatt uses CMake

License
-------

ubatt is Copyright 2020 by Stephen Cavilia.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.
