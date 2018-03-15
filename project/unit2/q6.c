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
	int id;
	char *k;
	char *v;
};

struct waypoint {
	int wayid;
	int ordinal;
	int nodeid;
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


/* Main Function */
int main(int argc, char *argv[]){
	FILE *fp = fopen(argv[2], "r");
	char buf[bufSize];
	char *p;
	char * db_name = argv[1];
	sqlite3 * db;
	sqlite3_stmt *stmt;

	//Open database file
	int rc = sqlite3_open(db_name, &db);
	if (rc){
		fprintf(stderr, "error: %s", sqlite3_errmsg(db));
		return 1;
	}

	if (fp == NULL){
		fprintf(stderr, "ERROR: input.txt does not exist.\n");
		exit(EXIT_FAILURE);
	}

	// line=1 is the first line of the tuple in tsv file
	// line=2 is the second line of the tuple in tsv file
	// line=3 is the emplty line of the tuple in tsv file
	int line=1;

	//the while loop reads a single line each time
    //the input data are seprated by tab
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		buf[strlen(buf) - 1] = '\0'; // eat the newline fgets() stores
		p = strtok (buf, "\t");

		// two structs are used to store data;
		struct waypoint *current_waypoint;
		struct tag *current_tag;
		current_tag = malloc(sizeof (struct tag));
		current_tag->k = malloc((sizeof (char)*100));
		current_tag->v = malloc((sizeof (char)*100));
		current_waypoint = malloc((sizeof (struct waypoint)));

        // int i is used to count the number of column 
		int i = 0;
		//wayId are used to store wayID
		int wayID ;



        // Two conditional val are controlling this while loop.
        // All insertion are happened inside this while loop.
        // conditional val line is controling the switch between (way waytag)line , (waypoint) line and (empty)line
        // conditional val i is controling the number of column for each line
		while (p != NULL){


			if (line == 1){

				// fgets is currently reading the wayID 
				if (i == 0 ){
					wayID =atoi(p);
					current_tag->id = atoi(p);
					printf("wayid:%d\t close:whatever you want\n",wayID);

				// do insert way into way table (wayid , close_or_not)
                /*******************************************************************************/
					char * sql3 = "insert into way values (?,0)";
					rc = sqlite3_prepare_v2(db, sql3, -1, &stmt, 0);
					if (rc != SQLITE_OK){
						fprintf(stderr, "error: %s", sqlite3_errmsg(db));
						sqlite3_close(db);
						return 1;
					}
					sqlite3_bind_int(stmt, 1, wayID);
					rc = sqlite3_step(stmt);
					if (rc != SQLITE_DONE) {
						printf("ERROR inserting way: %s\n", sqlite3_errmsg(db));
						sqlite3_close(db);
					}
				}
				/*******************************************************************************/

               
                // fgets is currently reading the waytags
				if (i > 0 ) {
					getPair(p,current_tag);
					printf("waytagID:%d\t k:%s\t v:%s \n",current_tag->id,current_tag->k,current_tag->v);

				// do insert waytag into waytagtable (tagID, k, v)
				/*******************************************************************************/
					char * sql5 = "insert into waytag values (?,?,?)";
					rc = sqlite3_prepare_v2(db, sql5, -1, &stmt, 0);
					if (rc != SQLITE_OK){
						fprintf(stderr, "error: %s", sqlite3_errmsg(db));
						sqlite3_close(db);
					return 1;
					}
					sqlite3_bind_int(stmt, 1, current_tag->id);
					sqlite3_bind_text(stmt, 2, current_tag->k, -1, SQLITE_STATIC);
					sqlite3_bind_text(stmt, 3, current_tag->v, -1, SQLITE_STATIC);
					rc = sqlite3_step(stmt);
					if (rc != SQLITE_DONE) {
						printf("ERROR inserting waytag: %s\n", sqlite3_errmsg(db));
						sqlite3_close(db);
					}
				}
				/*******************************************************************************/

			}

			/*if fgets is currently reading the waypoint line */
			else if (line == 2){
				current_waypoint->wayid = wayID;
				current_waypoint->ordinal = i;
				current_waypoint->nodeid = atoi(p);

                // do insert waypoint into waypoint table (wayid , ordinal, nodeID)
				/*******************************************************************************/
				printf("wayPointID:%d\t ordinal:%d\t nodeID:%d\n",current_waypoint->wayid,current_waypoint->ordinal,current_waypoint->nodeid);
				char * sql4 = "insert into waypoint values (?,?,?)";
				rc = sqlite3_prepare_v2(db, sql4, -1, &stmt, 0);
				if (rc != SQLITE_OK){
					fprintf(stderr, "error: %s", sqlite3_errmsg(db));
					sqlite3_close(db);
				return 1;
				}
				sqlite3_bind_int(stmt, 1, current_waypoint->wayid);
				sqlite3_bind_int(stmt, 2, current_waypoint->ordinal);
				sqlite3_bind_int(stmt, 3, current_waypoint->nodeid);
				rc = sqlite3_step(stmt);
				if (rc != SQLITE_DONE) {
					printf("ERROR inserting nodetag: %s\n", sqlite3_errmsg(db));
					sqlite3_close(db);
				}
			}
			/*******************************************************************************/
			i++;
			p = strtok (NULL, "\t");

		}


		// do update closed in way table after insert all node in waypoint
		/*******************************************************************************/
		char * sql6 = "update way set closed = case when (select nodeid from waypoint where wayid=? and ordinal= (select max(ordinal) from waypoint where wayid=?))=(select nodeid from waypoint where wayid=? and ordinal= (select min(ordinal) from waypoint where wayid=?)) then 1 else 0 end where id=?";
		rc = sqlite3_prepare_v2(db, sql6, -1, &stmt, 0);
		if (rc != SQLITE_OK){
			fprintf(stderr, "error: %s", sqlite3_errmsg(db));
			sqlite3_close(db);
			return 1;
		}
		printf("wayID:%d\n",wayID);
		sqlite3_bind_int(stmt, 1, wayID);
		sqlite3_bind_int(stmt, 2, wayID);
		sqlite3_bind_int(stmt, 3, wayID);
		sqlite3_bind_int(stmt, 4, wayID);
		sqlite3_bind_int(stmt, 5, wayID);
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE) {
			printf("ERROR inserting nodetag: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
		}
		/*******************************************************************************/
		line = (line==3) ? 1 : line+1;
	}
	fclose(fp);



 	/* finalize statement and close database */
	sqlite3_finalize(stmt);
	sqlite3_close(db);




	return 0;
}
