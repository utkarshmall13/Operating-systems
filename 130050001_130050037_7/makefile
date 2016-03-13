all: races-partA races-partB races-partC

races-partA: races-partA.c	
	@g++ -pthread races-partA.c -o races-partA

races-partB: races-partB.c	
	@g++ -pthread races-partB.c -o races-partB

races-partC: races-partC.c	
	@g++ -pthread races-partC.c -o races-partC
	
clean:
	@rm -rf races-partA races-partB races-partC
