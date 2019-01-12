In one vulcan console, run this command to start the server:

srun -N 1 -n 2 -A sspwork -p pdebug server.elf --file `pwd`/connect.txt --port 5605

In another vulcan console, run this command to start the client:

./client.elf --file connect.txt
