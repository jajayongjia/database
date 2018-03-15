/*
 * Copyright 2017 Yongjia Huang,Jie Wang CMPUT391, University of Alberta, All Rights Reserved.
 * You may use distribut, or modify this code under terms and conditions of the code of Student Behavior at University of Alberta
 * You may find a copy of the license in this project. Otherwise please contact jajayongjia@gmail.com
 *
 */
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define bufSize 1024

struct tag {
    char *k;
    char *v;
};

struct node {
    int id;
    double lat;
    double lon;
};


/* This function takes input name : key=value and an empty struct tag pointer
it seprate the name and push these values into the empty struct */
void getPair(char *name, struct tag *store){
    int i;
    int n;
    for (i = 0; name[i]!= '=';i++){
            (store->k)[i] = name[i];
    }
    (store->k)[i] =  '\0';
    for (n = 0; name[i+1]!= '\0';i++,n++){
            (store->v)[n] = name[i+1];
    }
    (store->v)[n] =  '\0';
}


/* Main Function*/
int main(int argc, char *argv[]){
    FILE *fp = fopen(argv[2], "r");
    char buf[bufSize];
    char *p;
    char * db_name = argv[1];
    sqlite3 * db;
    sqlite3_stmt *stmt;

    //Open databse file
    int rc = sqlite3_open(db_name, &db);
	if (rc){
		fprintf(stderr, "error: %s", sqlite3_errmsg(db));
		return 1;
	}
    if (fp == NULL){
        fprintf(stderr, "ERROR: input.txt does not exist.\n");
        exit(EXIT_FAILURE);
    }

    //the while loop reads a single line each time
    //the input data are seprated by tab
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
        p = strtok (buf, "\t");

        // two structs are used to store data;
        struct node *current_node;
        struct tag *current_tag;
        current_tag = malloc(sizeof (struct tag));
        current_tag->k = malloc((sizeof (char)*100));
        current_tag->v = malloc((sizeof (char)*100));
        current_node = malloc((sizeof (struct node)));

        // int i is used to count the number of column 
        int i = 0;
        while (p != NULL){
            switch (i){

                // if it is the first column, the data contains ID
                case 0:
                    current_node->id =atoi(p);
                    break;

                //if it is the second column, the data contains lat
                case 1:
                    current_node->lat =atof(p);
                    break;

                //if it is the third column, the data contains lon
                case 2:
                    current_node->lon =atof(p);
                    break;

                //otherwise, tha data contains nodetag
                default:
                    getPair(p,current_tag);
                    break;


            }


            if(i==2) {

                //if finished store node data
                //do node insertion here !
                /* insert the current_node->id,current_node->lat,current_node->lon into the node table */
                /****************************************************************************************************************/
                printf("id:%d \t lat:%f \t lon:%f \t\n",current_node->id,current_node->lat,current_node->lon);
                char * sql1 = "insert into node values (?,?,?)";
                rc = sqlite3_prepare_v2(db, sql1, -1, &stmt, 0);
                if (rc != SQLITE_OK){
                    fprintf(stderr, "error: %s", sqlite3_errmsg(db));
                    sqlite3_close(db);
                return 1;
                }
                sqlite3_bind_int(stmt, 1, current_node->id);
                sqlite3_bind_double(stmt, 2, current_node->lat);
                sqlite3_bind_double(stmt, 3, current_node->lon);
                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE) {
                    printf("ERROR inserting node: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                }
                /****************************************************************************************************************/
            }


            if (i>=3){

                //if finished store nodetag data
                //do nodetag insertion here !
                /****************************************************************************************************************/
                printf("id:%d \t k:%s \t v:%s \t \n",current_node->id,current_tag->k,current_tag->v);
                /* insert the current_node->id,current_tag->k,current_tag->v into nodetag table */
                char * sql2 = "insert into nodetag values (?,?,?)";
                rc = sqlite3_prepare_v2(db, sql2, -1, &stmt, 0);
                if (rc != SQLITE_OK){
                    fprintf(stderr, "error: %s", sqlite3_errmsg(db));
                    sqlite3_close(db);
                return 1;
                }
                sqlite3_bind_int(stmt, 1, current_node->id);
                sqlite3_bind_text(stmt, 2, current_tag->k, -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt, 3, current_tag->v, -1, SQLITE_STATIC);
                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE) {
                    printf("ERROR inserting nodetag: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                }
                /****************************************************************************************************************/
            }

            // change column
            i++;
            p = strtok (NULL, "\t");
        }
        
    }
    fclose(fp);

    /* finalize statement and close database */
    sqlite3_finalize(stmt);
  	sqlite3_close(db);




    return 0;
}
