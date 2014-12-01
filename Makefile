VPATH = ./source
CFLAG = 

.PHONY : all
all : healthcenterserver patient1 patient2 doctor

healthcenterserver: healthcenterserver.c
	gcc $< -o $@ $(CFLAG) -lpthread

patient1 : patient1.c
	gcc $< -o $@ $(CFLAG)

patient2 : patient2.c
	gcc $< -o $@ $(CFLAG)

doctor : doctor.c
	gcc $< -o $@ $(CFLAG)

.PHONY : clean
clean:
	rm -rf healthcenterserver patient1 patient2 doctor 
