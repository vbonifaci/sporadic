#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <set>
#include <cassert>
#include "ts.h"

using namespace std;

bool quiet = false; 
bool verbose = false; 

bool equals(const int* a, const int* b, int n) {
	for(int i=0; i<n; i++)
		if(a[i]!=b[i]) return false; 
	return true;
}

int cardinality(int S, int n) {
	int c = 0; 
	for(int i=0; i<n; i++)
		if(S & (1<<i)) c++; 
	return c; 
}

struct c_state {
	TS& ts; 
	int* c; 
	c_state(TS& ts_) : ts(ts_) {
		c = new int [ts.n]; 
		for(int i=0; i<ts.n; i++) c[i] = 0; 
	}
	c_state(const c_state& cs) : ts(cs.ts) {
		c = new int [ts.n]; 
		for(int i=0; i<ts.n; i++) c[i] = cs.c[i]; 
	}
	~c_state() {
		delete [] c; 
	}
	bool operator==(const c_state& cs) const {
		return equals(c, cs.c, ts.n); 
	}
	bool operator<(const c_state& cs) const {
		for(int i=0; i<ts.n; i++) {
			if(c[i] < cs.c[i]) return true; 
			if(cs.c[i] < c[i]) return false; 
		}
		return false; 
	} 
}; 

struct state {
	TS& ts; 
	int  pl; // player (1 or 2)
	int* t; 
	set<c_state> Q;  
	typedef set<c_state>::const_iterator CI;
	state(TS& ts_) : ts(ts_) {
		t = new int [ts.n]; 
		for(int i=0; i<ts.n; i++) t[i] = 0; 
		c_state c_zero(ts); 
		Q.insert(c_zero); 
		pl = 1; 
	}
	state(const state& s) : ts(s.ts), pl(s.pl), Q(s.Q) {
		t = new int [ts.n]; 
		for(int i=0; i<ts.n; i++) {
			t[i] = s.t[i]; 
		}
	}
	~state() {
		delete [] t;
	}
	bool operator==(const state& s) const { return (pl==s.pl) && equals(t,s.t,ts.n) && Q==s.Q; }
	bool operator<(const state& s) const { 
		if(pl<s.pl) return true; 
		if(s.pl<pl) return false; 
		for(int i=0; i<ts.n; i++) {
			if(t[i]<s.t[i]) return true; 
			if(s.t[i]<t[i]) return false; 
		}
		if(Q < s.Q) return true; 
		if(s.Q < Q) return false; 
		return false; 
	}
	int d(int i) const { return max(t[i] - (ts.T[i] - ts.D[i]), 0); }
	void print() const {
		cout << "P" << pl << " ";  
		for(int i=0; i<ts.n; i++) {
			cout << "d[" << i << "]: " << d(i) << " "; 
			cout << "t["<< i << "]: " << t[i] << " "; 
		}
		cout << endl << "{" << endl; 
		for(CI csi=Q.begin(); csi!=Q.end(); csi++) {
			for(int i=0; i<ts.n; i++) {
				cout << "\tc[" << i << "]: " << csi->c[i] << " "; 
			}
			cout << endl; 
		}
		if(Q.empty()) cout << "\t*FAIL*" << " " << endl; 
		cout << "}" << endl; 
	}
}; 

// Main data structures
TS ts; // the task system being analyzed
int m; // n. of processors

bool feasible(TS& ts) {
	map<state, bool> generated; 
	
	bool failureGenerated = false; 
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
						v2.t[i] = ts.T[i]; 
						for(state::CI csi=v2.Q.begin(); csi!=v2.Q.end(); csi++) {
							csi->c[i] = ts.C[i]; 
						}
					}
				}
				//v2.print();
				//DEBUGE(generated[v2]); 
				if(!generated[v2]) {
					generated[v2] = true; 
					count++; 
					q.push(v2); 
				}
			}
		}
		else if(s.pl==2) {
			// V2 -> V1
			state v1 = s; 
			v1.pl = 1; 
			for(int i=0; i<ts.n; i++) {
				v1.t[i] = max(v1.t[i]-1, 0); 
			}
			v1.Q.clear(); 
			for(state::CI csi=s.Q.begin(); csi!=s.Q.end(); csi++) {
				for(int S=0; S<(1<<ts.n); S++) {
					if(cardinality(S, ts.n) > m) continue; 
					c_state cs(*csi);
					bool deadmiss = false;  
					for(int i=0; i<ts.n; i++) {
						if((S & (1<<i))) {
							cs.c[i] = max(csi->c[i]-1, 0); 
						}
						if(v1.d(i)==0 && cs.c[i]>0) {
							deadmiss = true; 
							break; 
						}
					}
					if(!deadmiss) { 
						v1.Q.insert(cs); 
					}
				}
			}
			if(!generated[v1]) {
				if(v1.Q.empty()) failureGenerated = true; 
				generated[v1] = true; 
				count++; 
				q.push(v1); 
			}
		}
		if(verbose) s.print(); 
	} 
	cerr << count << " states generated. " << endl; 
	bool feasibleTS = !failureGenerated; 
	cerr << "The task system is " << (feasibleTS ? "FEASIBLE" : "INFEASIBLE") << "." << endl; 
	return feasibleTS; 
}

int main(int argc, char* argv[]) {
	if(argc>1 && string(argv[1])=="-v") verbose = true;  
	if(argc>1 && string(argv[1])=="-q") quiet = true; 
	if(!quiet) cerr << "Number of processors? " << endl; 
	cin >> m; 
	ts.read(quiet); 
	return ((int) feasible(ts)); 
}


