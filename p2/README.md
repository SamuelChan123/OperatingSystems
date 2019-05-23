1. What is the vulnerability?

  The vulnerability in the webserver is the check_file_length method, which takes in bytes. However, an int was passed in, which means that in C, only the last 8 bits are passed in (the rest are truncated). Hence, the webserver only checks if the file length is less than 100 for the last 8 bytes, not for the whole file length.

2. How did you exploit it?

  Simply inject a string with shellcode such that the length of the string's last 8 bits is less than 100.

3. How does the vulnerability work on the stack (specifically with respect to your webserver)?

  Overflows the stack such that the return address is overwritten to a location in a nop sled, which will eventually hit the shellcode and execute it. This will open a listening port on port 19442, which we can connect to with netcat and open a shell on the remote server.

4. How did you "tune" your attack (for the local and remote servers)?

  Tuning occurred with a string injected with the following format:
    -Return Address of Shellcode
    -Long No-Op Sled
    -Shellcode

  We decided to have a range of return addresses at the beginning so that our nop sled is in front and we jump forward into it. This allows us to make a bigger nop sled than if it had to be between the address we are overwriting and the start of the buffer's memory location.
  The return address is repeated many times in hopes of overflowing the stack and changing the return address to the middle of the long no-op sled, which leads directly into the shellcode. We increased our nop sled as much as we could when testing remotely to increase the chance 
  of our return address ending up in the nop sled. We didn't make the nop sled as big when testing locally because we had a better sense of where we need to return to thanks to gdb specifying the location where the return address is stored and the local webserver tells us where the
  beginning of the memory buffer is (i.e. the filename variable). 
  
  Remotely, we started from 0xbfffffff and worked our way down in increments of our nop sled divided by 2. We had success at 0xbfffff05 which is 250 down from 0xbfffffff. We didn't have to change how many times we were repeating our return address for remote testing as it was already 
  sizeable.
    

5. What does your final attack string look like? (not the exact string, just the format ex. GET / XXX YYYY ZZZ HTTP)

Looks like GET /\RA\RA\RA\RA\Nop\Nop\Nop\Nop\Nop\Nop\Nop\Nop\Shellcode HTTP

6. What command(s) did you use to carry out the attack?

  General: gdb for debugging and finding stack information for local server, gcc for compilation, netcat to connect to server and perform attacks by sending the attack string to the local/remote servers.
  
  For local testing:
  
  First, we run setarch `uname -m` -R /bin/bash to prevent randomized virtual addressing on our own machines.
  We used gcc -m32 -z execstack -fno-stack-protector webserver.c -o webserver to compile our local webserver and used the same command to compile our c program that writes the attack string
  to a txt file by replacing webserver with that c program name. Afterwards we run our c program to output the attack string into a text file.We then run the webserver in another terminal tab 
  using ./webserver 8000 and then run cat {our attack string txt file} | nc 127.0.0.1 8000 in the other terminal tab to connect to the local server and attack it. If successful, we should be 
  able to enter shell commands to manipulate the local server.
  
  For remote testing:
  
  This time we only had to compile our c program using the same command specified for local testing. We then run cat {our attack string txt file} | nc 310test.cs.duke.edu 9422 to connect to the remote
  server. If successful, then our terminal should hang. We then open a new terminal tab and type nc 310test.cs.duke.edu 19422 to listen on port 19422. If it hangs we will be able to use shell commands
  to manipulate the remote server. 

7. What file(s) did you modify on the remote server?

  The index.html file, as well as creating a few other files just for kicks, to demonstrate that the attack worked. Specifically, we created a PWNED.txt file in the www directory and echoed our netids
  into the index.html.
