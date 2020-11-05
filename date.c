#include "date.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct date{
	int day;
	int month;
	int year;
};

Date *date_create(char *datestr){
	struct date *d;
	if (datestr[2]=='/' && datestr[5]=='/'){
		//split the string
		char split[] = "/";
		char *token = strtok(datestr, split);
		int date[3];
		int i = 0;
		while(token != NULL){
			date[i] = atoi(token);
			token = strtok(NULL, split);
			i ++;
		}

		//might need to do a check here before directly mallocing
		if((d = (struct date *)malloc(sizeof(struct date)))!=NULL){
			//NEED TO FREE D AT SOME POINT
			d->day = date[0];
			d->month = date[1];
			d->year = date[2];
			
			/*
			printf("%d\n", date[0]);
			printf("%d\n", date[1]);
			printf("%d\nfinished", date[2]);
			*/
		}
		return d;
	}
	else{
		return NULL;
	}
}

Date *date_duplicate(Date *d){
	struct date *duplicate;
	if ((duplicate = (struct date *)malloc(sizeof(struct date)))!=NULL){
		duplicate->day = d->day;
		duplicate->month = d->month;
		duplicate->year = d->year;
	}
	return duplicate;
}

int date_compare(Date *date1, Date *date2){
	if (date1->year > date2->year){
		return 1;
	}
	else if (date1->year < date2->year){
		return -1;
	}
	else if (date1->month > date2->month){
		return 1;
	}
	else if (date1->month < date2->month){
		return -1;
	}
	else if (date1->day > date2->day){
		return 1;
	}
	else if (date1->day < date2->day){
		return -1;
	}
	else{
		return 0;
	}
}

void date_destroy(Date *d){
	free(d);
}
