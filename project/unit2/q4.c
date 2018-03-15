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
#include <math.h>

struct waytag{
   char *key;
   char *value;

};
/*geographical distance function*/
/* We used the polar coordinate flat-Earth formula. The location of nodes got from a flat surface map of Alberta. The map shows neither spherical nor ellipsoidal earth. */
void geo_distance (sqlite3_context* ctx, int argc, sqlite3_value** values){
  double lat1 = sqlite3_value_double(values[0]);
  double lat2 = sqlite3_value_double(values[2]);
  double lon1 = sqlite3_value_double(values[1]);
  double lon2 = sqlite3_value_double(values[3]);

  double theta1=M_PI/180 * (90-lat1);
  double theta2=M_PI/180 * (90-lat2);
  double dis_lon=M_PI/180 * (lon1-lon2);
  double gdistance=6371.009*sqrt((theta1*theta1)+(theta2*theta2)-2*theta1*theta2*cos(dis_lon));
  sqlite3_result_double(ctx, gdistance);
}


/* This function takes input name : key=value and an empty struct nodetag pointer
it seprate the name and push these values into the empty struct */
void getPair(char *name, struct waytag *store){
  int i;
  int n;
  for (i = 0; name[i]!= '=';i++){
    (store->key)[i] = name[i];
  }
  (store->key)[i] =  '\0';
  for (n = 0; name[i+1]!= '\0';i++,n++){
    (store->value)[n] = name[i+1];
  }
  (store->value)[n] =  '\0';
}


/* Main Function*/
int main(int argc, char *argv[]){

  struct waytag *tag[argc-2];
  char *lists[argc];
  char * db_name = argv[1];
  char * string;
  char *searchPair = malloc(sizeof(char)*600);
  sqlite3 * db;
  sqlite3_stmt *stmt;

  //Open database file
  int rc = sqlite3_open(db_name, &db);
  if (rc){
    fprintf(stderr, "error: %s", sqlite3_errmsg(db));
    return 1;
  }


  // find input len
	for (int i =2; i<argc;i++){
    lists[i-2] = argv[i];
	}

  //store input key and value pairs into struct array pointers.
  for (int i =0; i<(argc-2);i++){
    tag[i] = malloc(sizeof(struct waytag));
    tag[i]->key = malloc(sizeof(char)*50+1);
    tag[i]->value = malloc(sizeof(char)*50+1);
    getPair(lists[i],tag[i]);
	}

   //convert the data to the sql query format
	strcat(searchPair,"(");
  for (int i =0; i<(argc-2);i++){
    strcat(searchPair,"(waytag.k='");
	  strcat(searchPair,tag[i]->key);
	  strcat(searchPair,"' AND");
	  strcat(searchPair," waytag.v='");
	  strcat(searchPair,tag[i]->value);
	  strcat(searchPair,"')");
	  if (i!=(argc-3))
       // OR statement connects different key/value pairs
      strcat(searchPair," OR ");
	}
  strcat(searchPair,")");

  // searchPair now contains the content with sql query format 
  /* create a new function to calculate distance with the arguement of latitude and longitude of two node */
  sqlite3_create_function(db, "distance", 4, SQLITE_UTF8, NULL, geo_distance, NULL, NULL);
  /* write a query to create a temperate table storing all wayid that match condition. Then select the number of ways and the largest path length. */
  char * sql = malloc(sizeof(char)*1000);
  strcat(sql,"with result(way_id) as (select way.id from waytag, way where waytag.id=way.id and ");
  strcat(sql,searchPair);
  strcat(sql," group by way.id) select count(distinct(result.way_id)), max(ls_dist) from result, (select r.way_id as num_path, sum(distance(n1.lat, n1.lon, n2.lat, n2.lon)) as ls_dist from result r, waypoint w1 INNER JOIN waypoint w2, node n1 INNER JOIN node n2 where w1.wayid=r.way_id and w2.wayid=r.way_id and w1.ordinal=w2.ordinal-1 and n1.id=w1.nodeid and n2.id=w2.nodeid group by r.way_id)");
  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
  if (rc != SQLITE_OK){
    fprintf(stderr, "error: %s", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 1;
  }

  /* execute the query and print the result */
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
    int col;
    for(col=0; col<sqlite3_column_count(stmt)-1; col++) {
      printf("%s|", sqlite3_column_text(stmt, col));
    }
    printf("%skm", sqlite3_column_text(stmt, col));
    printf("\n");
  }
  /* finalize statement and close database */
  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return 0;
}
