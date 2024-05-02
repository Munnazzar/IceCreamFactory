#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/shm.h>
#include <fcntl.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <sys/mman.h>
#include <string>
#include <time.h>
using namespace std;
struct sysinfo memInfo;
FILE* fptr;
struct buyer{
        unsigned int ID;
        unsigned int amountOfFlavours;
        unsigned int amountOfToppings;
        string coneFlavour;
        string flavours[3];
        string toppings[3];
        double totalPrice;
};
void *worker(void *arg);
typedef class monitor{
private:
    friend void* worker(void *arg);
    double priceFlavours[3]  		  =  {70.1, 83.5, 14.51};
    double priceCones[3]     		  =  {10.4, 11.5, 12.6};
    double priceToppings[3]		  	  =  {5.9,  9.1,  11.2};

    string flavours[3]		   		  =  {"ButterScotch", "Chocolate", "Vanilla"};
    string cones[3]     	 	 	  =  {"Waffle",       "Sugar",     "Cake"};
    string toppings[3]  			  =  {"Sprinkles",    "Fudge",     "WhippedCream"};

    unsigned int amountFlavours[3]    =  {50, 50, 50};
    unsigned int amountCones[3]	      =  {50, 50, 50};
    unsigned int amountToppings[3]	  =  {70, 70, 70}; 

    pthread_mutex_t cond_flavours[3];
    pthread_mutex_t cond_cones[3];
    pthread_mutex_t cond_toppings[3];

public:
    monitor(){
        for(int i = 0; i < 3; i++){
            pthread_mutex_init(&cond_flavours[i], NULL);
            pthread_mutex_init(&cond_cones[i], NULL);
            pthread_mutex_init(&cond_toppings[i], NULL);
        }
    }
    double getPrice(string type, int index){
        if(type == "flavour") return priceFlavours[index];
        else if(type == "cone") return priceCones[index];
        else if(type == "topping") return priceToppings[index];
    }
    double flavourWoker(void* arg){
        string flavour = *((string*)arg); 
        for(int i = 0; i<3; i++){
            if(flavour == this->flavours[i]){
                pthread_mutex_lock(&cond_flavours[i]);
                if(this->amountFlavours[i] <= 0){
                    pthread_mutex_unlock(&cond_flavours[i]);
                    return -1.0;
                }
                this->amountFlavours[i]--;
                pthread_mutex_unlock(&cond_flavours[i]);
                return this->priceFlavours[i];
            }
        }    
        return -1.0;    
    }
    double coneWorker(void* arg){
        string cone = *((string*)arg);
        for(int i = 0; i<3; i++){
            if(cone == this->cones[i]){
                pthread_mutex_lock(&cond_cones[i]);
                if(this->amountCones[i] <= 0){
                    pthread_mutex_unlock(&cond_cones[i]);
                    return -1.0;
                } 
                this->amountCones[i]--;
                pthread_mutex_unlock(&cond_cones[i]);
                
                return this->priceCones[i];
            }
        }    
        return -1.0;    
    }
    double toppingWorker(void* arg){
        string topping = *((string*)arg);
        for(int i = 0; i<3; i++){
            if(topping == this->toppings[i]){
                pthread_mutex_lock(&cond_toppings[i]);
                if(this->amountToppings[i] <= 0){
                    pthread_mutex_unlock(&cond_toppings[i]);
                    return -1.0;
                }
                this->amountToppings[i]--;
                pthread_mutex_unlock(&cond_toppings[i]);
                return this->priceToppings[i];
            }
            continue;
        }   
        return -1.0;    
    }
}monitor;

monitor M;

void *worker(void *arg){
    double totalPrice = 0.0;
    buyer *customer = (buyer *)arg;
    double temp = M.coneWorker(&customer->coneFlavour);
    if(temp == -1.0){
        customer->totalPrice = -1.0;
        pthread_exit(NULL);
    }
    else{
        totalPrice += temp;
    }
    for(int i = 0; i<customer->amountOfFlavours; i++){
        temp = M.flavourWoker(&customer->flavours[i]);
        if(temp == -1.0){
            customer->totalPrice = -1.0;
            pthread_exit(NULL);
        }
        else{
            totalPrice += temp;
            break;
        }
    }
    for(int i = 0; i<customer->amountOfToppings; i++){
        temp = M.toppingWorker(&customer->toppings[i]);
        if(temp == -1.0){
            customer->totalPrice = -1.0;
            pthread_exit(NULL);
        }
        else{
            totalPrice += temp;
            break;
        }
    }
    customer->totalPrice = totalPrice;
    pthread_exit(NULL);
}

int main(){
    clock_t start = clock();
    fptr = freopen("data.txt","r", stdin);
    sysinfo (&memInfo);
    int numThreads;
	int numBuyers;
	cin>>numThreads;
	getchar();
	cin>>numBuyers;	
	getchar();
	pthread_t *pid = new pthread_t[numThreads];
	struct buyer *allBuyers = new struct buyer[numBuyers];
	for(int i = 0; i<numBuyers; i++){
		allBuyers[i].totalPrice = 0;	
		cin>>allBuyers[i].ID;
		getchar();
		cin>>allBuyers[i].amountOfFlavours;
		getchar();
		cin>>allBuyers[i].amountOfToppings;
		getchar();
		cin>>allBuyers[i].coneFlavour;
		allBuyers[i].coneFlavour.erase(0, allBuyers[i].coneFlavour.find_first_not_of(" \n\r\t"));
		for(int j = 0; j<allBuyers[i].amountOfFlavours; j++){
			cin>>allBuyers[i].flavours[j];
			allBuyers[i].flavours[j].erase(0, allBuyers[i].flavours[j].find_first_not_of(" \n\r\t"));
		}
		for(int j = 0; j<allBuyers[i].amountOfToppings; j++){
			cin>>allBuyers[i].toppings[j];
			allBuyers[i].toppings[j].erase(0, allBuyers[i].toppings[j].find_first_not_of(" \n\r\t"));	
		}
	}
	pthread_t *tid = new pthread_t[numThreads];
	int check = 0;
	int checkAgain = 0;
	for(int i = 0; i<numBuyers; 1){
		for(int j = 0; j<numThreads; j++){
			if(check >= numBuyers)	break;
			pthread_create(&tid[j], NULL, worker, (void*) &allBuyers[i]);
			++i;
			++check;
		}
		for(int j = 0; j<numThreads; j++){
            if(checkAgain >= numBuyers)	break;
			pthread_join(tid[j], NULL);
			++checkAgain;
		}
	}

	FILE* fptr2 = freopen("output.txt", "a+", stdout);	
	for(int i = 0; i<numBuyers; i++){
		cout<<"Customer ID: "<<allBuyers[i].ID<<endl;
		cout<<"Cones: "<<allBuyers[i].coneFlavour<<endl;
		cout<<"Flavours: ";
		for(int j = 0; j<allBuyers[i].amountOfFlavours; j++){
			cout<<allBuyers[i].flavours[j]<<" ";
		}
		cout<<"\nToppings: ";
		for(int j = 0; j<allBuyers[i].amountOfToppings; j++){
			cout<<allBuyers[i].toppings[j]<<" ";
		}
		if(allBuyers[i].totalPrice == -1)
			cout<<"\nTotal Price: Not enough ingredients"<<endl;
		else
			cout<<"\nTotal Price: "<<allBuyers[i].totalPrice<<endl;
		cout<<endl<<endl;
	}
	long long totalPhysMem = memInfo.totalram;
	totalPhysMem  = totalPhysMem * memInfo.mem_unit;
	cout<<"Total Physical Memory: "<<totalPhysMem<<endl;
	free(tid);
	
    clock_t end = clock();
    cout<<"Time taken: "<<(double)(end-start)/CLOCKS_PER_SEC<<endl;
    fclose(fptr);
	fclose(fptr2);
	return 0;   
}