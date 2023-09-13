# Directory-check-search

This project, that consists of pfind.c and spfind.c is designed to search for files in a directory hierarchy based
on specified permissions.

### Overview
pfind searches for files with a specified permission string in a given directory and its subdirectories.
spfind extends pfind by allowing users to sort the output based on file names or file paths.

### Usage:
This program is designed to work in a Linux environment. To build, run the commands:  

    cd <repository's directory> 
    make
  
To run the program, use the following command:
 
For pfind:
    
    pfind -d <directory> -p <permissions string> [-h]


    -d: <directory>: Specifies the directory to search in.
    -p: <permissions string>: Specifies the permission string to search for.
    -h: Displays usage information.

For spfind:

    spfind -d <directory> -p <permissions string> [-h]


    -d: <directory>: Specifies the directory to search in.
    -p: <permissions string>: Specifies the permission string to search for.
    -h: Displays usage information.


### Running the Programs:

Compile the programs using the provided compilation commands.
Run pfind or spfind from the command line with appropriate arguments.

### Note:

Both programs assume that the sort command is available in the system. If it is not available, spfind will not function correctly.

### Author:

Aleksandra Dubrovina

### Contact

For inquiries or feedback, please contact aleks.dubrovina@gmail.com.
