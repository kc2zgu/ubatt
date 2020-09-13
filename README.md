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

ubatt uses CMake:

    $ cd ubatt
    $ mkdir build
    $ cmake ..
    $ make
     -- or --
    $ cmake .. -G Ninja
    $ ninja


Examples
--------

Show a summary of battery information:

    $ ubatt
    On battery power
    Battery: 46%, Discharging 3h 30m remaining

Print just the battery charge level (useful for parsing in shell scripts):

    $ ubatt -p
    46

Return 1 or 0 if on battery or line power:

    if ! ubatt -r; then
        echo "On battery!"
    fi


Return the battery charge level as exit code:

    ubatt -R
    bat=$?
    echo "Battery is ${bat}%"

License
-------

ubatt is Copyright 2020 by Stephen Cavilia.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.
