#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

int n; // total number of nodes
int d[33][33]; // distance between two nodes
int save[33][33]; // saving value between two nodes
int route[33][33], len[33], nr; // path list in algorithm, length of each path, length of path list
int Aans[33], Bans[33], Cans[33]; // answer getting from this algorithm
int temp[33]; //temp array for 2-opt
char str[111]; // temp char array for input
int belong[33]; // node is belong to path or not

typedef struct pair {
	int u, v;
	int save;
} pair;


pair pairs[1111]; // for saving
int np = 0;
pair edges[33];

// get input from the file and parse n and distance value
void getInput(char* filename) {
    FILE* ptr;
    ptr = fopen(filename, "r");
 	
    if(NULL == ptr) {
        printf("file can't be opened \n");
        exit(0);
    }
	n = strlen(filename) == 13 ? 9 : 29;
	
 	int lineNumber = 0;
    while(fgets(str, 50, ptr) != NULL) {
        lineNumber++;
        if(lineNumber == 5) break;
    }
    int i = 0, j = 0;
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
}

// calculate the distance from the route
int calc(int* route) {
	int pre = 0;
	int cost = 0;
	for(int i=0; i<n-1; i++) {
		cost += d[pre][route[i]];
		pre = route[i];
	}
	cost += d[pre][0];
	return cost;
}
// display the route
void display(int* route) {
	printf("0");
	for(int i=0; i<n-1; i++) {
		printf("->%d", route[i]);
	}
	printf("->0\n");
	printf("Total distance: %d\n", calc(route));
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
	for(int i=1; i<n; i++) {
		for(int j=1; j<n; j++) {
			save[i][j] = d[0][i] + d[0][j] - d[i][j];
			pair now = {i, j, save[i][j]};
			if(i != j) pairs[np++] = now;
		}
	}

}

// apply CW algorithm
void executeCW() {
	//init
	for(int i=1; i<n; i++) belong[i] = false;
	// sort this array into non-increasing order
	qsort(pairs, np, sizeof(*pairs), cmp);
	for(int i=0; i<np; i++) {
		pair now = pairs[i];
//		printf("%d %d %d\n", now.u, now.v, now.save);
		int u = now.u, v = now.v;
		// check if can create new path
		if(belong[u] == false && belong[v] == false) {
			route[nr][len[nr] ++] = u;
			route[nr][len[nr] ++] = v;
			belong[u] = belong[v] = true;
			nr++;
			continue;
		}
		// concantenate with one path
		int headu=-1, tailu=-1, headv=-1, tailv=-1;
		for(int j=0; j<nr; j++) if(len[j]) {
			if(route[j][0] == u) headu = j;
			if(route[j][len[j] - 1] == u) tailu = j;
			if(route[j][0] == v) headv = j;
			if(route[j][len[j] - 1] == v) tailv = j;
		}
		// concantenate u->v to one path at the begin
		if(belong[u] == false && headv != -1) {
			len[headv] ++;
			for(int j=len[headv]-1; j>0; j--) route[headv][j] = route[headv][j-1];
			route[headv][0] = u;
			belong[u] = true;
			continue;
		}
		// concantenate u->v to one path at the end
		if(belong[v] == false && tailu != -1) {
			route[tailu][len[tailu]++] = v;
			belong[v] = true;
			continue;	
		}
		// check if can join two path
		if(tailu != -1 && headv != -1 && tailu != headv) {
			for(int j=0; j<len[headv]; j++) {
				route[tailu][len[tailu]++] = route[headv][j];
			}
			len[headv] = 0;
		}
	}
	// get the answer
	int k;
	for(int i=0; i<nr; i++) if(len[i]) {
		for(int j=0; j<n-1; j++) Aans[j] = route[i][j];
		break;
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
	for(int j=0; j<u; j++) B[j] = A[j];
	for(int j=v; j>=u; j--) B[u+(v-j)] = A[j];
	for(int j=v+1; j<n-1; j++) B[j] = A[j];	
}

// apply approach
void execute2Opt() {
	for(int i=0; i<n-1; i++) Bans[i] = Aans[i];
	int bestDis = calc(Bans);
	while(true) {
		int mn = bestDis; // infinitive
		for(int i=1; i<n-1; i++) {
			for(int j=i+1; j<n-1; j++) {
				// swap edge (i-1, i) and (j, j+1)
				swapEdge(i, j, Bans, temp);
				
				int tempDistance = calc(temp);
				if(tempDistance < mn) {
					mn = tempDistance;
					for(int k=0; k<n-1; k++) Bans[k] = temp[k];
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
	for(int j=0; j<n-1; j++) B[j] = A[j];
	int c = B[u];
	B[u] = B[u+1];
	B[u+1] = c;
	
}

int mem[33];
int best[33];
int tabu[1111][33];
int qb=0, qf=0;

// check if tmp is in tabu array
bool isInTabu(int *tmp) {
	for(int i=qf; i<qb; i++) {
		int same = 1;
		for(int j=0; j<n-1; j++) if(tmp[j] != tabu[i][j]) {
			same = 0;
			break;
		}
		if(same) return true;
	}
	return false;
}

// execute approach
void executeTabu() {
	for(int i=0; i<n-1; i++) Cans[i] = Aans[i];
	int bestDis = calc(Cans);
	int step = 1000;
	qb=qf=0;
	while(step--) {
		// swap ith node and (i+1)th node
		int mn = 0x3f3f3f3f;
		for(int i=0; i<n-2; i++) {
			swapNode(i, Bans, temp);
			if(!isInTabu(temp)) {
				int cost = calc(temp);
				if(mn > cost) {
					mn = cost;
					for(int j=0; j<n-1; j++) mem[j] = temp[j];
				}
			}
			
		}
		if(mn == 0x3f3f3f3f) break;
		if(mn < bestDis) {
			bestDis = mn;
			for(int i=0; i<n-1; i++) Cans[i] = mem[i];
		}
		memcpy(tabu[qb], mem, n);
		qb++;
		if(qb - qf > 100) qf++;
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

int main() {
	getInput("Instance29.txt");
//	getInput("Instance9.txt");
	ClarkeAndWright();
	ImproveWith2Opt();
	ImproveWithTabu();
}



