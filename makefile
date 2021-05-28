
OPTIONS = -ansi -Wall -Wno-sign-compare
AUX = ts.cpp

all: feasible onlfeas gedf gfp llf

feasible: feasible.cpp $(AUX)
	g++ $(OPTIONS) -o feasible feasible.cpp $(AUX)

onlfeas: onlfeas.cpp $(AUX)
	g++ $(OPTIONS) -o onlfeas onlfeas.cpp $(AUX)
	
gedf: schedtst.cpp $(AUX)
	g++ $(OPTIONS) -D GEDF -o gedf schedtst.cpp $(AUX)

gfp: schedtst.cpp $(AUX)
	g++ $(OPTIONS) -D GFP -o gfp schedtst.cpp $(AUX)

llf: schedtst.cpp $(AUX)
	g++ $(OPTIONS) -D LLF -o llf schedtst.cpp $(AUX)

clean: 
	rm feasible onlfeas gedf gfp llf





