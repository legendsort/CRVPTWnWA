#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#define SZ 77
int n; // total number of nodes
int d[SZ][SZ]; // distance between two nodes
int save[SZ][SZ]; // saving value between two nodes
int route[SZ][SZ], len[SZ], nr; // path list in algorithm, length of each path, length of path list
int Aans[SZ], Bans[SZ], Cans[SZ]; // answer getting from this algorithm
int nA; // length of Aans
int temp[SZ]; //temp array for 2-opt
char str[111]; // temp char array for input
int belong[SZ]; // node is belong to path or not
int cnt[SZ]; // number of nodes belonging path
int dst[SZ]; // distance of path
int cannotReach[SZ];
int nc;


const int capacity[2][3] = {
	{500, 300, 200}, //for instance 9
	{1500, 1000, 500} //for instance 29
};
const int velocity[2] = {
	1400, //for instance 9, 1400m/hmin
	100 //for instance 29, 80km/hour
};

const int D = 50;
int V, Q;
// saving value structure
typedef struct pair {
	int u, v;
	int save;
} pair;


pair pairs[1111]; // for saving
int np = 0, nt = 0;
pair edges[SZ];


// get input from the file and parse n and distance value
int getInput(char* filename) {
    FILE* ptr;
    ptr = fopen(filename, "r");
 	
    if(NULL == ptr) {
        printf("file can't be opened \n");
        exit(0);
    }
	// get type of input
	n = strlen(filename) == 13 ? 9 : 29;
	// get input unnecessary input
 	int lineNumber = 0;
    while(fgets(str, 50, ptr) != NULL) {
        lineNumber++;
        if(lineNumber == 5) break;
    }
    int i = 0, j = 0;
    // input matrix
    for(i = 0; i < n; i ++) {
    	j = 0;
    	if(i == 0) fscanf(ptr, "d=[[%d", d[i] + j);
    	else {
    		fscanf(ptr, " [%d", d[i] + j);
    	}
    	for(j = 1; j < n; j++) {
    		fscanf(ptr, ",%d", d[i] + j);	
    	}
    	fscanf(ptr, "],\n");

    	lineNumber++;
    }
    fclose(ptr);
    return n == 29;
}

// calculate the distance from the route
int calc(int* route) {
	int pre = 0;
	int distance = 0;
	int i = 0;
	for(i=1; i<nA; i++) {
		distance += d[pre][route[i]];
		pre = route[i];
	}
	
	return distance;
}


// display the route
void display(int* route) {
	int n = nA;
	int i = 0;
	if(nc) {
		printf("The nodes we cannot reach to\n");
		for(i=0; i<nc; i++) {
			printf("%d ", cannotReach[i]);
		}
		puts("");
	} else {
		printf("Wow!!! We deliver all\n");
	}
	int vehicles = -1;
	int vehicleID = 1;
	int distance = 0, takeDays = 0;
	double takeHours = 0;
	int pre = 0;
	for(i=0; i<nA; i++) {
		if(i) printf("->");
		if(i < nA-1 && route[i] == 0) {
			if(i) {
				printf("0\n");
				printf("This vehicle moved %dkm and returned at %d day %.1lf hour.\n", distance, takeDays, takeHours);
				
			}
			printf("Vehicle %d: ", vehicleID++);
			distance = 0, takeDays = 0, takeHours = 0;
		} else {
			distance += d[pre][route[i]];
			double take = 1.0 * d[pre][route[i]] / V;
			takeHours += take;
			int days = ((int)takeHours) / 24;
			takeDays += days;
			takeHours -= 24 * days;
			if(route[i]) {
				takeHours = fmax(takeHours, 9.0);
				if(takeHours > 12) takeHours = fmax(takeHours, 14.0);
				if(takeHours > 17) {
					takeHours = 9;
					takeDays++;
				}
			} else {
				takeHours = fmax(takeHours, 7.0);
				if(takeHours > 19) {
					takeHours = 7;
					takeDays++;
				}
			}
			
		}
		printf("%d", route[i]);
		if(route[i] == 0) vehicles ++;
		pre = route[i];
	}
	printf("\n");
	printf("This vehicle moved %dkm and returned at %d days %.1lf hour.\n", distance, takeDays, takeHours);
	puts("");
	printf("Total distance: %d\n", calc(route));
	printf("Used vehicles: %d\n", vehicles);
}

/**
 * This is the beginning of the CW algorithm
 */

// compare function to sort pair array into non-increasing order
int cmp(const void *x, const void *y) {
	int l = ((pair *)x) -> save;
    int r = ((pair *)y) -> save;
    return l - r;
}

// calculate saving values for each pair of node and save it in array
void getSavingValues() {
	// make pair with save value
	int i, j;
	for(i=1; i<n; i++) {
		for(j=1; j<n; j++) {
			save[i][j] = d[0][i] + d[0][j] - d[i][j];
			pair now = {i, j, save[i][j]};
			if(i != j) pairs[np++] = now;
		}
	}
}

// check whether we can deliver to clients with path in time
// suitable time [9am: 12am] [2pm: 5pm]
// work time[7am: 7pm]
// if we can deliver next day and so on, always return true
int validPath(int *route, int nt) {
	return 1;
	double t = 7;
	int pre = 0;
	int i;
	for(i=0; i<nt; i++) {
		int p = route[i];
		double take = 1.0 * d[pre][p] / V;
		t += take;
		t = fmax(t, 9.0);
		if(t>12) t = fmax(t, 14.0);
		if(t > 17) return false;
		pre = p;
	}
	double take = 1.0 * d[pre][0] / V;
	t += take;
	if(t > 19) return 0;
	return true;
}


// check whether we can deliver to clients with path in time, format of path is different
// suitable time [9am: 12am] [2pm: 5pm]
// work time[7am: 7pm]
int tt[SZ]; // test array for this function
bool validAns(int *route, int nr) {
	int i;
	nt = 0;
	for(i=1; i<nr; i++) {
		if(route[i] == 0) {
			if(validPath(tt, nt) == 0) return false;
			nt = 0;
			continue;	
		}
		tt[nt++] = route[i];
	}
	return true;
}


// apply CW algorithm
void executeCW() {
	//init
	int i, j;
	for(i=1; i<n; i++) belong[i] = false;
	// sort this array into non-increasing order
	qsort(pairs, np, sizeof(*pairs), cmp);
	for(i=0; i<np; i++) {
		pair now = pairs[i];
		int u = now.u, v = now.v;
		// concantenate with one path
		int headu=-1, tailu=-1, headv=-1, tailv=-1;
		for(j=0; j<nr; j++) if(len[j]) {
			if(route[j][0] == u) headu = j;
			if(route[j][len[j] - 1] == u) tailu = j;
			if(route[j][0] == v) headv = j;
			if(route[j][len[j] - 1] == v) tailv = j;
		}
		// check if can create new path
		if(belong[u] == false && belong[v] == false && Q >= 2) {
			nt = 0;
			temp[nt++] = u;
			temp[nt++] = v;
			if(validPath(temp, nt)) {
				memcpy(route[nr], temp, (len[nr] = 2) * 4);
				belong[u] = belong[v] = true;
				nr++;
			}
			continue;
		}
		// concantenate u->v to one path at the begin
		if(belong[u] == false && headv != -1 && Q > len[headv]) {
			nt = 0;
			temp[nt++] = u;
			for(j=0; j<len[headv]; j++) temp[nt++] = route[headv][j];
			if(validPath(temp, nt)) {
				memcpy(route[headv], temp, (++len[headv]) * 4);	
				belong[u] = true;
			}
			continue;
		}
		// concantenate u->v to one path at the end
		if(belong[v] == false && tailu != -1 && Q > len[tailu]) {
			nt = 0;
			memcpy(temp, route[tailu], len[tailu] * 4);
			nt = len[tailu];
			temp[nt++] = v;
			if(validPath(temp, nt)) {
				memcpy(route[tailu], temp, (++len[tailu]) * 4);	
				belong[v] = true;
			}
			continue;	
		}
		// check if can join two path
		if(tailu != -1 && headv != -1 && tailu != headv && Q >= len[headv] + len[tailu]) {
			nt = 0;
			memcpy(temp, route[tailu], len[tailu] * 4);
			memcpy(temp + len[tailu], route[headv], len[headv] * 4);
			nt = len[tailu] + len[headv];
			if(validPath(temp, nt)) {
				memcpy(route[tailu], temp, (len[tailu] = nt) * 4);
				len[headv] = 0;	
			}
		}
	}
	// get the answer
	int k;
	nA = 0;
	Aans[nA++] = 0;
	for(i=0; i<nr; i++) if(len[i]) {
		
		for(j=0; j<len[i]; j++) Aans[nA++] = route[i][j];
		Aans[nA++] = 0;
	}
	for(i=1; i<n; i++) {
		if(belong[i] == false) {
			nt = 0;
			temp[nt++] = i;
			if(validPath(temp, nt)) {
				belong[i] = true;
				Aans[nA++] = i;
				Aans[nA++] = 0;
			} else {
				cannotReach[nc++] = i;
				
			}
		}
	}
}

// get solution of TSP using Clarke and Wright Algorithm
void ClarkeAndWright() {
	printf("Start ClarkeAndWright Algorithm for TSP problem\n");
	clock_t st = clock(); // for measure execution time
	getSavingValues();
	executeCW();
	clock_t ed = clock();
	display(Aans);
	printf("Execution time: %dms\n\n", (ed-st));
}

/**
 * This is the end of the CW algorithm
 */ 
 
/**
 * This is the beginning of the 2-Opt approach
 */
 
void swapEdge(int u, int v, int *A, int *B) {
	int j;
	for(j=0; j<u; j++) B[j] = A[j];
	for(j=v; j>=u; j--) B[u+(v-j)] = A[j];
	for(j=v+1; j<nA; j++) B[j] = A[j];	
}

// apply approach
void execute2Opt() {
	int i, j, k;
	for(i=0; i<nA; i++) Bans[i] = Aans[i];
	int bestDis = calc(Bans);
	while(true) {
		int mn = bestDis; // infinitive
		for(i=1; i<nA; i++) if(Bans[i] != 0) {
			for(j=i+1; j<nA; j++) if(Bans[j] != 0) {
				// swap edge (i-1, i) and (j, j+1)
				swapEdge(i, j, Bans, temp);
				if(!validAns(temp, nA)) continue; 
				int tempDistance = calc(temp);
				if(tempDistance < mn) {
					mn = tempDistance;
					for(k=0; k<nA; k++) Bans[k] = temp[k];
				}
			}
		}
		if(mn == bestDis) break;
		bestDis = mn;
	}
}

// apply approach
// improve TSP with 2 Opt approach
void ImproveWith2Opt() {
	printf("Improve algorithm using 2-Opt\n");
	clock_t st = clock();
	execute2Opt();
	clock_t ed = clock();
	display(Bans);
	printf("Execution time: %dms\n\n",(ed-st));
}

/**
 * This is the end of the 2-Opt approach
 */

/**
 * This is the beginning of the Tabu Search approach
 */

// swap uth node and (u+1)th node
void swapNode(int u, int *A, int *B) {
	int j;
	for(j=0; j<nA; j++) B[j] = A[j];
	int c = B[u];
	B[u] = B[u+1];
	B[u+1] = c;
	
}

int mem[SZ];
int best[SZ];
int tabu[5555][SZ];
int qb=0, qf=0;

// check if tmp is in tabu array
bool isInTabu(int *tmp) {
	int i, j;
	for(i=qf; i<qb; i++) {
		int same = 1;
		for(j=0; j<nA; j++) if(tmp[j] != tabu[i][j]) {
			same = 0;
			break;
		}
		if(same) return true;
	}
	return false;
}

// execute approach
void executeTabu() {
	int i, j;
	for(i=0; i<nA; i++) Cans[i] = Bans[i];
	int bestDis = calc(Cans);
	int step = 1000;
	qb=qf=0;
	while(step--) {
		// swap ith node and (i+1)th node
		int mn = 0x3f3f3f3f;
		for(i=0; i<nA-1; i++) if(Cans[i] != 0 && Cans[i+1] != 0) {
			swapNode(i, Cans, temp);
			if(!validAns(temp, nA)) continue;
			if(!isInTabu(temp)) {
				int cost = calc(temp);
				if(mn > cost) {
			
					mn = cost;
					for(j=0; j<nA; j++) mem[j] = temp[j];
				}
			}	
		}
		if(mn == 0x3f3f3f3f) break;
		if(mn < bestDis) {
			bestDis = mn;
			for(i=0; i<nA; i++) Cans[i] = mem[i];
		}
		memcpy(tabu[qb], mem, nA * 4);
		qb++;
		if(qb - qf > 500) qf++;
	}
}

// improve TSP with Tabu search
void ImproveWithTabu() {
	printf("Improve algorithm using Tabu Search\n");
	clock_t st = clock();
	executeTabu();
	clock_t ed = clock();
	display(Cans);
	printf("Execution time: %dms\n\n",(ed-st));
	
}

// init for every capacity
void init() {
	np = nt = nr = nA = 0;	
	qf = qb = 0;
	nc = 0;
	
}

int main() {
	
	int fileId = getInput("Instance29.txt");
//	int fileId = getInput("Instance9.txt");
	int i;
	for(i=0; i<3; i++) {
		init();
		Q = capacity[fileId][i] / D;
		V = velocity[fileId];
		ClarkeAndWright();
		ImproveWith2Opt();
		ImproveWithTabu();
	}
	sleep(10); 
	return 0;
}



