
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <cassert>
#include "ts.h"

using namespace std;

struct state {
	TS& ts; 
	int  pl; // player (1 or 2)
	int* c; 
	int* t; 
	bool failure; 
	state(TS& ts_) : ts(ts_) {
		c = new int [ts.n]; 
		t = new int [ts.n]; 
		for(int i=0; i<ts.n; i++) c[i] = t[i] = 0; 
		pl = 1; 
		failure = false; 
	}
	state(const state& s) : ts(s.ts), pl(s.pl), failure(s.failure) {
		c = new int [ts.n]; 
		t = new int [ts.n]; 
		for(int i=0; i<ts.n; i++) {
			c[i] = s.c[i]; 
			t[i] = s.t[i]; 
		}
	}
	~state() {
		delete [] c; 
		delete [] t;
	}
	bool operator==(const state& s) const { return (pl==s.pl) && equals(c,s.c,ts.n) && equals(t,s.t,ts.n) && (failure==s.failure); }
	bool operator<(const state& s) const { 
		if(pl<s.pl) return true; 
		if(s.pl<pl) return false; 
		for(int i=0; i<ts.n; i++) {
			if(c[i]<s.c[i]) return true; 
			if(s.c[i]<c[i]) return false; 
			if(t[i]<s.t[i]) return true; 
			if(s.t[i]<t[i]) return false; 
		}
		if(failure && !s.failure) return true; 
		if(!failure && s.failure) return false; 
		return false; 
	}
	int d(int i) const { return max(t[i] - (ts.T[i] - ts.D[i]), 0); }
	bool equals(const int* a, const int* b, int n) const {
		for(int i=0; i<n; i++)
			if(a[i]!=b[i]) return false; 
		return true;
	}
	void print() const {
		// PRINTT 
		cout << "P" << pl << " ";  
		for(int i=0; i<ts.n; i++) {
			cout << "c["<< i << "]: " << c[i] << " "; 
			cout << "d[" << i << "]: " << d(i) << " "; 
			cout << "t["<< i << "]: " << t[i] << " "; 
		}
		if(failure) cout << "*FAIL*" << " "; 
		cout << endl; 
	}
	state& operator=(const state& s) {
		if(this != &s) {
			ts = s.ts; 
			pl = s.pl; 
			failure = s.failure; 
			for(int i=0; i<ts.n; i++) {
				c[i] = s.c[i]; 
				t[i] = s.t[i]; 
			}
		}
		return *this; 
	}
}; 

bool quiet = false; 
bool verbose = false; 
multimap<state, state> g;
typedef multimap<state,state>::const_iterator NI; // neighborhood iterator
//map<state, bool> failures; 
map<state, bool> fixpoint; 
map<state, bool> generated; 
typedef map<state,bool>::const_iterator SBI; // state (bool map) iterator

// Main data structures
TS ts; // the task system being analyzed
int m; // n. of processors

int cardinality(int S, int n) {
	int c = 0; 
	for(int i=0; i<n; i++)
		if(S & (1<<i)) c++; 
	return c; 
}

void init() {
	g = multimap<state,state>(); 
	//ts = TS(1); 
	//ts.setTask(0, 2, 3, 4); 
	//ts.setTask(1, 2, 4, 5); 
	m = 1; 
}

void populate() {
	//generated = map<state,bool>(); 
	state start(ts); 
	generated[start] = true; 
	queue<state> q; 
	q.push(start); 
	int count = 1; 
	while(!q.empty()) {
		state s = q.front(); 
		q.pop();
		if(s.pl==1) {
			// V1 -> V2
			for(int K=0; K<(1<<ts.n); K++) {
				//DEBUGE(K); 
				state v2 = s; 
				v2.pl = 2; 
				for(int i=0; i<ts.n; i++) {
					if((K & (1<<i)) && s.t[i]==0) {
						v2.c[i] = ts.C[i]; 
						v2.t[i] = ts.T[i]; 
					}
				}
				//v2.print();
				//DEBUGE(generated[v2]); 
				if(!generated[v2]) {
					generated[v2] = true; 
					count++; 
					q.push(v2); 
				}
				g.insert(make_pair(s, v2)); 
			}
		}
		else if(s.pl==2) {
			// V2 -> V1
			bool some_ok = false; 
			for(int S=0; S<(1<<ts.n); S++) {
				if(cardinality(S,ts.n) > m) continue; 
				state v1 = s; 
				v1.pl = 1; 
				v1.failure = false; 
				bool no_deadmiss = true; 
				for(int i=0; i<ts.n; i++) {
					v1.t[i] = max(v1.t[i]-1, 0); 
					if((S & (1<<i))) {
						v1.c[i] = max(v1.c[i]-1, 0); 
					}
					if(v1.d(i)==0 && v1.c[i]>0) {
						no_deadmiss = false; 
						//cout << "MISS!" << endl; 
						//DEBUGE(some_ok); 
					}
				}
				if(no_deadmiss) { 
					some_ok = true; 
					//v1.print(); 
					//DEBUGE(generated[v1]); 
					if(!generated[v1]) {
						generated[v1] = true; 
						count++; 
						q.push(v1); 
					}
					g.insert(make_pair(s, v1)); 
				}
			}
			if(!some_ok) {
				s.failure = true; 
				//failures[s] = true; 
			}
		}
		if(verbose) s.print(); 
	} 
	cerr << count << " states generated. " << endl; 
}

bool findFixpoint() {
	//fixpoint = failures; // deep-copy ?
	//map<state, bool> next = fixpoint; 
	bool newStuff = true; 
	int count = 0; 
	while(newStuff) {
		newStuff = false; 
		for(SBI i=generated.begin(); i!=generated.end(); i++) {
			// state == i->first
			// bool == i->second
			const state& s = i->first; 
			if(fixpoint[s]) continue; // s already in fixpoint
			pair<NI,NI> b = g.equal_range(s); // get neighbors
			if(s.pl==1) {
				// add if SOME neighbor in fixpoint
				bool someIn = false; 
				for(NI j=b.first; j!=b.second; ++j) {
					const state& v = j->second; 
					if(fixpoint[v]) someIn = true; 
				}
				if(someIn) {
					fixpoint[s] = true; 
					newStuff = true; 
					++count; 
					if(verbose) s.print(); 
				}
			}
			else if(s.pl==2) {
				// add if ALL neighbors in fixpoint
				bool allIn = true; 
				for(NI j=b.first; j!=b.second; ++j) {
					const state& v = j->second; 
					if(!fixpoint[v]) allIn = false; 
				}
				if(allIn) {
					fixpoint[s] = true; 
					newStuff = true; 
					++count; 
					if(verbose) s.print(); 
				}
			}
		}
	}
	state start(ts); 
	bool feasible = !fixpoint[start]; 
	if(verbose) cerr << count << " states in fixpoint." << endl; 
	cerr << "The system is " << (feasible ? "ONLINE FEASIBLE" : "ONLINE INFEASIBLE") << "." << endl; 
	return feasible; 
}


int main(int argc, char* argv[]) {
	if(argc>1 && string(argv[1])=="-v") verbose = true;  
	if(argc>1 && string(argv[1])=="-q") quiet = true;  
	init(); 
	if(!quiet) cerr << "Number of processors? " << endl; 
	cin >> m; 
	ts.read(quiet); 
	populate(); 
	return ((int)findFixpoint()); 
}


