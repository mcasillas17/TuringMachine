#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>
#include <time.h>
#include <math.h>
#include <queue>

#define TAPE_LENGTH 700 //LENGHT OF A TAPE
#define MAX_STATES 64   //MAX NUMBER OF STATES
#define TM_SIZE 1024    //BINARY TM SIZE
#define HALT 63         //63 IS HALT STATE
#define MAX_MOVES 1000  //MAX MOVES FOR A TM
#define MAX_ITS 100000  //MAX NUMBER OF MUTATIONS

using namespace std;

/*Structure that defines the states
  table of a TM:
  machine_cell has two rows (symbol 0 or 1),
  and N columns (total states, in this case, 64)*/
struct machine_cell{
    int write=0;
    char moveTo='X';
    int nState=0;
};

/* Global variables */
/*tape is the one we obtain after mutation,
  targetTape contains the binary of the text
  we want to replicate with the TM */
int tape[TAPE_LENGTH],targetTape[TAPE_LENGTH];
//For controlling the TM
int initHead,head,state,n;
//Counting the total moves of the TM
long long moves;
/*test is the TM we're testing,
  best is the best TM found,
  both in binary representation for mutation*/
int test[TM_SIZE],best[TM_SIZE];
//For reading the target text
string text,binary;
//The file in which we'll save the final machine
FILE * pFile;
/*Two structures: for test TM and best TM,
  these are the ones we simulate after mutation*/
machine_cell testMachine[2][MAX_STATES], bestMachine[2][MAX_STATES];
//For saving Hamming Distance
int ham;

//Initialize for the first time the tapes and variables
void init(){
    initHead = head = TAPE_LENGTH/2;
    for(int i=0;i<TAPE_LENGTH;i++)
        tape[i] = targetTape[i] = 0;
    for(int i=0;i<TM_SIZE;i++)
        test[i] = best[i] = 0;
}
//Preparing everything for the test, once we have the target text
void initMachine(){
    for(int i=0;i<TAPE_LENGTH;i++)
        tape[i]=0;
    head = TAPE_LENGTH/2;
    moves = 0;
    state = 0;
}
//Obtains Hamming distance for comparing tapes
int hamming(){
    int h=0;
    string strTape="";
    for(int i=0;i<(int)binary.size();i++){
        if(tape[initHead+i]!=(int)(binary[i]-'0')){ h++; }
    }
    return h;
}
//Runs the TM to check its result
void runTestMachine(){
    initMachine();
    int symR;
    while(state!=HALT&&head>=0&&head<TAPE_LENGTH&&moves<MAX_MOVES){
        symR = tape[head];
        tape[head]=testMachine[symR][state].write;
        testMachine[symR][state].moveTo=='R' ? head++:head--;
        state = testMachine[symR][state].nState;
        moves++;
    }
}
//Converts a text to binary
void textToBinary(){
    binary = "";
    for(int i=0;i<(int)text.size();i++){
        binary += bitset<7>(text[i]).to_string();
    }
    printf("Binary: %s\n",binary.c_str());
}
/*Reads the text introduced by the user and
  converts it to binary*/
void readText(){
    printf("Type the string to replicate:\n");
    getline(cin,text);
    init();
    textToBinary();
}
/*Produces a first mutation of N-random positions
  in the binary tapes*/
void firstMutation(){
    int m = rand()%(TM_SIZE - 750 + 1) + 750;
    //int m = rand()%(TM_SIZE - 250 + 1) + 250;
    printf("Bits to change in the first mutation: %d\n",m);
    int randNum;
    for(int i=0;i<m;i++){
        randNum = rand()%(TM_SIZE);
        //test[randNum] = 1;
        test[randNum]++; test[randNum]%=2;
        best[randNum] = test[randNum];
    }
}
/*Transforms the binary tape to a machine_cell structure
  for testing the TM.
  Same methodology, different tapes (test and best)*/
void testToMachine(){
    int currentState=0, currentSymbol=0;
    int write=0, index=0, nextState=0;
    char moveTo;
    while(index<TM_SIZE){
        write = test[index++];
        test[index++]==1?moveTo='R':moveTo='L';
        nextState = 0;
        for(int j=5;j>=0;j--){
            if(test[index]==1) nextState |= 1<<j;
            index++;
        }
        testMachine[currentSymbol][currentState].write = write;
        testMachine[currentSymbol][currentState].moveTo = moveTo;
        testMachine[currentSymbol][currentState].nState = nextState;
        currentSymbol++;
        if(currentSymbol==2){
            currentSymbol=0;
            currentState++;
        }
    }
}
void bestToMachine(){
    int currentState=0, currentSymbol=0;
    int write=0, index=0, nextState=0;
    char moveTo;
    while(index<TM_SIZE){
        write = best[index++];
        best[index++]==1?moveTo='R':moveTo='L';
        nextState = 0;
        for(int j=5;j>=0;j--){
            if(best[index]==1) nextState |= 1<<j;
            index++;
        }
        bestMachine[currentSymbol][currentState].write = write;
        bestMachine[currentSymbol][currentState].moveTo = moveTo;
        bestMachine[currentSymbol][currentState].nState = nextState;
        currentSymbol++;
        if(currentSymbol==2){
            currentSymbol=0;
            currentState++;
        }
    }
}
/*Reduces the best TM, deleting the states
  not visited in the process*/
void reduceBestMachine(){
    queue<int> q;
    int graph[64][2];
    int newIndexes[64];
    for(int i=0;i<64;i++)
        newIndexes[i]=-1;
    int f,currentState=1,v1,v2,write;
    char moveTo;
    newIndexes[0]=0;
    q.push(0);
    while(!q.empty()){
        f = q.front();
        v1 = bestMachine[0][f].nState;
        v2 = bestMachine[1][f].nState;
        if(newIndexes[v1]==-1){
            q.push(v1);
            newIndexes[v1]=currentState++;
        }
        if(newIndexes[v2]==-1){
            q.push(v2);
            newIndexes[v2]=currentState++;
        }
        q.pop();
    }
    int visited[64];
    for(int i=0;i<64;i++)
        visited[i]=0;
    string finalMachine = text+"Machine.txt";
    pFile = fopen(finalMachine.c_str(),"w");
    fprintf(pFile,"%d\n",currentState);
    q.push(0);
    while(!q.empty()){
        f = q.front();
        if(visited[f]==0){
            write = bestMachine[0][f].write;
            moveTo = bestMachine[0][f].moveTo;
            v1 = bestMachine[0][f].nState;
            fprintf(pFile,"%d 0 %d %c %d\n",newIndexes[f],write,moveTo,newIndexes[v1]);
            q.push(v1);
            write = bestMachine[1][f].write;
            moveTo = bestMachine[1][f].moveTo;
            v2 = bestMachine[1][f].nState;
            fprintf(pFile,"%d 0 %d %c %d\n",newIndexes[f],write,moveTo,newIndexes[v2]);
            q.push(v2);
            visited[f]=1;
        }
        q.pop();
    }
    fclose(pFile);
    printf("\nFinal machine description on %s file.\n\n",finalMachine.c_str());
}
/*Compares both tapes (test and best) for checking
  if an improvement has been found*/
void evaluate(){
    int tHam = hamming();
    if(tHam<ham){
        printf("\nMachine changed! New Distance: %d\n",tHam);
        for(int i=0;i<TM_SIZE;i++){
            best[i]=test[i];
        }
        ham = tHam;
        printf("T text: %s\n",binary.c_str());
        string r = "";
        for(int i=0;i<binary.size();i++)
            r += tape[initHead+i]+'0';
        printf("B text: %s\n",r.c_str());
    }
}
//Mutates N-random positions in the test tape
void mutate(){
    int randNum = rand()%TM_SIZE;
    test[randNum]++; test[randNum]%=2;
}
//Running the program
int main(){
    srand(time(0));
    string cont="Y";
    int cI;
    while(cont=="Y"){
        init(); //Initializing
        cI = 0; //For counting mutations
        readText(); //User introduces text
        //Full process
        firstMutation();
        testToMachine();
        runTestMachine();
        ham = hamming();
        printf("Initial distance: %d\n",ham);
        do{
            printf("Iteration: %d\r",cI+1);
            mutate();
            testToMachine();
            runTestMachine();
            evaluate();
            cI++;
        }while(ham>0&&cI<MAX_ITS);
        bestToMachine();
        reduceBestMachine();
        printf("\n");
        printf("Replicate another string? [Y|N]:\n");
        getline(cin,cont);
    }
    return 0;
}
