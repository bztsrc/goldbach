/***********************************************************
 * Program to check the Goldbach's conjecture solution     *
 * written by Zoltan Baldaszti Tue Jan 8 22:03:06 CET 2008 *
 * the mathematical background and all coding was done by  *
 * the author, all rights reserved to him.                 *
 * Contact: zoltan DOT baldaszti AT gmail.com              *
 *                                                         *
 * Goldbach's conjecture: all even numbers (>2) can be     *
 * represented by an addititon of two primes.              *
 * This program is reponsible for checking if the given    *
 * prime indexing method fulfill this or not.              *
 *                                                         *
 * Compile: gcc goldbach.c -o goldbach                     *
 * Usage: ./goldbach (num of primes)                       *
 ***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//global variables
long argument;		//required number of primes
long *primes=NULL;	//array to hold the primes
long num_primes=0;	//number of elements in primes array

//this function checks if n is a prime, if so, then stores it in primes array
//prime check is done by dividing n with all number in primes array
void is_prime(long n)
{
	long prime=0,i,third;
	//even number can't be a prime (we exclude 2 also, but this is what we want!)
	if(n%2==0) return;
	//shortcut for the first 4 primes
	if(n==1 || n==3 || n==5 || n==7) prime=n;
	else {
		//we do the fastest method possible: we use the primes already found
		//to determine wether the given n is a prime or not. Obviously, since
		//it's an odd number, it's enough to check primes not larger than
		//the number divided by 3.
		prime=n; third=n/3;
		for(i=1;i<num_primes && primes[i]<=third;i++) if(n%(primes[i])==0) { prime=0; break; }
	}
	//if it's a prime, append it to our list
	if(prime){
		primes=realloc(primes,(num_primes+1)*sizeof(long));
		primes[num_primes]=prime;
		num_primes++;
	}
}

//function to generate primes. It also uses a cache in file "primes.txt"
//if less prime required than loaded from cache, does nothing.
void generateprimes()
{
	long i;
	FILE *f;
	f=fopen("primes.txt","r");
	if(f){
		while(!feof(f)) {
			fscanf(f,"%ld\n",&i);
			if(i>0){
				primes=realloc(primes,(num_primes+1)*sizeof(long));
				primes[num_primes++]=i;
				if(num_primes==argument) break;
			}
		}
		fclose(f);
	}
	printf("Loaded %ld primes.\n",num_primes);
	if(num_primes<argument){
		time_t s=0,e=0;
		printf("Generating %ld more primes... ",(argument-num_primes)); fflush(stdout);
		time(&s);
		for(i=(num_primes?primes[num_primes-1]+2:1);num_primes<argument;i+=2) is_prime(i);
		time(&e);
		f=fopen("primes.txt","w+");
		for(i=0;i<num_primes;i++) fprintf(f,"%ld\n",primes[i]);
		fclose(f);
		printf("OK (%d secs)\n",(int)(e-s));
	}
	//sanity check (not necessary, but be paranoid)
	if(argument<num_primes) num_primes=argument;
}

//the main function
int main(int argc,char **argv)
{
	//---variables
	//i=universal index
	//j,k=indexes to primes array
	//even=the calculated even number
	//max=the largest even number calculated
	long i,j=0,k=1,max=2,even=0;
	//---variables to check the result
	//seg=the 8 bit block where the boolean flags reside
	//offs=offset in the seg-th block
	//lastseg=the last segment (required to allocate appropirate amount of memory)
	//check=array where the boolean values are stored
	//mask=2^1,2^2,...,2^7 stored in an array for speed up
	long seg=0,offs=0,lastseg=-1;
	unsigned char *check=NULL,mask[]={1,2,4,8,16,32,64,128};

	//if we have an argument then use it, if not, ask for it
	if(argv[1]!=NULL) argument=strtol(argv[1],NULL,10);
	else {
		printf("Number of primes to use? ");
		scanf("%ld",&argument);
	}

	//generate the primes we'll use
	generateprimes();

	//print status header
	printf("%11s%9s%9s%11s%11s%11s\n","i","j","k","p(j)","p(k)","p(j)+p(k)");

	//main loop. Note that the exit condition depends on k, not i
	//here we calculate even number by adding two primes and set
	//a flag to indicate that the even number is in Rf.
	for(i=0;k<num_primes;i++){
		//get the even number
		even=primes[j]+primes[k];
		//print status
		printf("%11ld%9ld%9ld%11ld%11ld%11ld%5ld%%\r",i,j,k,primes[j],primes[k],even,(k+1)*100/num_primes);
		//calculate the next indexes (the point of all) - it's beautiful, how simple it is
		//Note, that if (2), max will contain the last value of
		//a B(k) set, therefore as larger argument you gave, the larger number of
		//even numbers will be missing. But, max(B(k))>min(B(k+1)) and we can garantee
		//that only even numbers less than min(B(k+1)) will be generated
		j++; if(j==k) {
			j=0; k++;
			//(1)
			if(max<primes[j]+primes[k]) max=primes[j]+primes[k];
		}
		//(2)
		/*if(max<primes[j]+primes[k]) max=primes[j]+primes[k];*/

		//finally set the number-has-been-calculated flag to true.
		seg=(even-2)/16; offs=((even-2)/2)%8;
		if(seg>lastseg) { check=realloc(check,seg+1); check[seg]=0; lastseg=seg; }
		check[seg]|=mask[offs];
	}
	printf("\nLast prime:%ld largest even number:%ld\nChecking. Missing:",primes[k-1],max);
	//do a check: have we generated all even numbers in interval [4,max] or not?
	//if so, it means that we can represent every even number by adding two primes
	//therefore Goldbach's conjecture is no longer an unsolved problem.
	j=0;
	for(i=4;i<=max;i+=2){
		seg=(i-2)/16; offs=((i-2)/2)%8;
		if(seg>lastseg || !(check[seg]&mask[offs])) { j++; printf(" %ld",i); }
	}
	//print the result of the check
	if(j==0) printf(" none.\nCorrect, all even numbers in [4,%ld] have been generated.\n",max);
	else printf("\nNot correct, %ld even number(s) missing.\n",j);
	exit(j);
}
