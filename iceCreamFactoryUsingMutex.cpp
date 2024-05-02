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
#include <sys/mman.h>
#include <string>

using namespace std;
FILE *fptr;

double priceFlavours[3]  		  =  {70.1, 83.5, 14.51};
double priceCones[3]     		  =  {10.4, 11.5, 12.6};
double priceToppings[3]		  	  =  {5.9,  9.1,  11.2};

string flavours[3]		   		  =  {"ButterScotch", "Chocolate", "Vanilla"};
string cones[3]     	 	 	  =  {"Waffle",       "Sugar",     "Cake"};
string toppings[3]  			  =  {"Sprinkles",    "Fudge",     "WhippedCream"};

unsigned int amountFlavours[3]    =  {50, 50, 50};
unsigned int amountCones[3]	      =  {50, 50, 50};
unsigned int amountToppings[3]	  =  {70, 70, 70};

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

pthread_mutex_t m_flavours[3];
pthread_mutex_t m_toppings[3];
pthread_mutex_t m_cones[3];

int main(){
	fptr = freopen("data.txt","r", stdin);

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
	for(int i = 0; i<3; i++){
        pthread_mutex_init(&m_cones[i], NULL);
        pthread_mutex_init(&m_flavours[i], NULL);
        pthread_mutex_init(&m_toppings[i], NULL);
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
	
	free(tid);
	for(int i = 0; i<3; i++){
        pthread_mutex_destroy(&m_cones[i]);
        pthread_mutex_destroy(&m_toppings[i]);
        pthread_mutex_destroy(&m_flavours[i]);
	}
	fclose(fptr);
	fclose(fptr2);
	return 0;
}


void *worker(void *arg){
	double totalPrice = 0.0;
	struct buyer *customer = (struct buyer *)arg;	
	if(customer->coneFlavour == cones[0]){
		pthread_mutex_lock(&m_cones[0]);
		if(amountCones[0] == 0){
			customer->totalPrice = -1;
			pthread_mutex_unlock(&m_cones[0]);
			pthread_exit(NULL);
		}
		totalPrice += priceCones[0];
		--amountCones[0];
		pthread_mutex_unlock(&m_cones[0]);
	}
	else if(customer->coneFlavour == cones[1]){
		pthread_mutex_lock(&m_cones[1]);
		if(amountCones[1] == 0){
			customer->totalPrice = -1;
			pthread_mutex_unlock(&m_cones[1]);
			pthread_exit(NULL);
		}
		totalPrice += priceCones[1];
		--amountCones[1];
		pthread_mutex_unlock(&m_cones[1]);
	}
	else if(customer->coneFlavour == cones[2]){
		pthread_mutex_lock(&m_cones[2]);
		if(amountCones[2] == 0){
			customer->totalPrice = -1;
			pthread_mutex_unlock(&m_cones[2]);
			pthread_exit(NULL);
		}
		totalPrice += priceCones[2];
		--amountCones[2];
		pthread_mutex_unlock(&m_cones[2]);
	
	}
	for(int i = 0; i<customer->amountOfFlavours; i++){
		if(customer->flavours[i] == flavours[0]){
			pthread_mutex_lock(&m_flavours[0]);
			if(amountFlavours[0] == 0){
				customer->totalPrice = -1;
				pthread_mutex_unlock(&m_flavours[0]);
				pthread_exit(NULL);
			}
			totalPrice += priceFlavours[0];
			--amountFlavours[0];
			pthread_mutex_unlock(&m_flavours[0]);
		}
		else if(customer->flavours[i] == flavours[1]){
			pthread_mutex_lock(&m_flavours[1]);
			if(amountFlavours[1] == 0){
				customer->totalPrice = -1;
				pthread_mutex_unlock(&m_flavours[1]);
				pthread_exit(NULL);
			}
			totalPrice += priceFlavours[1];
			--amountFlavours[1];
			pthread_mutex_unlock(&m_flavours[1]);
		}
		else if(customer->flavours[i] == flavours[2]){
			pthread_mutex_lock(&m_flavours[2]);
			if(amountFlavours[2] == 0){
				customer->totalPrice = -1;
				pthread_mutex_unlock(&m_flavours[2]);
				pthread_exit(NULL);
			}
			totalPrice += priceFlavours[2];
			--amountFlavours[2];
			pthread_mutex_unlock(&m_flavours[2]);
		}
	}
	for(int i = 0; i<customer->amountOfToppings; i++){
		if(customer->toppings[i] == toppings[0]){
			pthread_mutex_lock(&m_toppings[0]);
			if(amountToppings[0] == 0){
				customer->totalPrice = -1;
				pthread_mutex_unlock(&m_toppings[0]);
				pthread_exit(NULL);
			}
			totalPrice += priceToppings[0];
			--amountToppings[0];
			pthread_mutex_unlock(&m_toppings[0]);
		}
		else if(customer->toppings[i] == toppings[1]){
			pthread_mutex_lock(&m_toppings[1]);
			if(amountToppings[1] == 0){
				customer->totalPrice = -1;
				pthread_mutex_unlock(&m_toppings[1]);
				pthread_exit(NULL);
			}
			totalPrice += priceToppings[1];
			--amountToppings[1];
			pthread_mutex_unlock(&m_toppings[1]);
		}
		else if(customer->toppings[i] == toppings[2]){
			pthread_mutex_lock(&m_toppings[2]);
			if(amountToppings[2] == 0){
				customer->totalPrice = -1;
				pthread_mutex_unlock(&m_toppings[2]);
				pthread_exit(NULL);
			}
			totalPrice += priceToppings[2];
			--amountToppings[2];
			pthread_mutex_unlock(&m_toppings[2]);
		}
	}
	customer->totalPrice = totalPrice;
	pthread_exit(NULL);
}