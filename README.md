### Computer Networks Project
--- 
##### Two applications, named "**client**" and "**server**", that communicate with each other based on a protocol that has the following specifications:
+ communication is done by executing commands read from the keyboard in the client and executed in the child processes created by the server

+ the result obtained after the execution of any command will be displayed by the client

+ the child processes in the server do not communicate directly with the client, but only with the parent process

--- 

The protocol includes the following commands:
  - **login : <username>** - the list of accepted usernames is found in "users.txt"
  - **get-logged-users** - displays information (username, hostname for remote login, time entry was made) about users logged into the operating system; the user must be logged in to use this command
  - **get-proc-info : <pid>** - displays information (name, state, ppid, uid, vmsize) about the indicated process; the user must be logged in to use this command
  - **logout** - logs the client out
  - **quit**
  - **help**
