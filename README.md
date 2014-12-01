=========================================================================================

NAME:	Huang Zhixian
USC ID:	5294095647

=========================================================================================

PROJECT DESCRIPTION

In this project, I've designed four applications: healthcenterserver, doctor, patient1 and patient2. These four applications work together as a simple online medical appointment system.
The patient1/patient2 uses TCP socket connection to communicate with the Health Center Server, and uses UDP socket connection to communicate with the doctor.
The project is based on UNIX socket programming and C programing language.
I implemented multithread programing, using pthread_create() function, in the Health Center Server code. So the Health Center Server can deal with multiple concurrent patients.
I also implemented multiprocess programing, using fork() funciton, in the Doctor code. So doctor1 and doctor2 will be concurrently created in the system using one executable.
I provided a Makefile to compile the source code.

=========================================================================================

FILE LIST

./README
./Makefile							
./source/healthcenterserver.c		the TCP Server, authenticates and makes appointment with the patient.
./source/patient1.c					the TCP & UDP Client, communicates with the Health Center Server and the Doctor.
./source/patient2.c					the TCP & UDP Client, is almost identical as the Patient 1.
./source/doctor.c					the UDP Server, response for the price inquery from the patient.
./input/patient1.txt				the input file contains username and password of the first patient.
./input/patient2.txt				the input file contains username and password of the second patient.
./input/users.txt					the input file contains username and password of the registered users.
./input/availabilities.txt			the input file contains the available appointments of the doctors.
./input/doc1.txt					the input file contains the detail of the insurance plans of Doctor 1.
./input/doc2.txt					the input file contains the detail of the insurance plans of Doctor 2.
./input/patient1insurance.txt		the input file contains the insurance plan of patient1.
./input/patient2insurance.txt		the input file contains the insurance plan of patient2.

=========================================================================================

RUN INSTRUCTION

1. Update input file in the directory "./input/", if needed.
2. Make files using command "make".
3. Run the Health Center Server using command "./healthcenterserver".
4. Run the Doctor using command "./doctor".
5. Run the Patient 1 or the Patient 2 using command "./patient1" or "./patient2".
6. Clean output files using "make clean", if needed.

=========================================================================================

FORMAT OF MESSAGE EXCHANGED

All formats of message exchanged are strings.

=========================================================================================

IDIOSYNCRASY

No.
=========================================================================================

REUSED CODE

No.
=========================================================================================


