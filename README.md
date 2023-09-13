# directory-check-search

Directory-check-search prints the files that match the permission string provided by the user for all
the files starting in the given directory, also provided by the user. First it validates the permsitions
and the directory name, and then recurses into the directory and all its files.

This program is designed to work in a Linux environment. To build, run the commands
cd <repository's directory>
make
