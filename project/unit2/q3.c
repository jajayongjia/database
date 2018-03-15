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
   
};

/* This function takes input name : key=value and an empty struct nodetag pointer
it seprate the name and push these values into the empty struct */
char * getNode(char* nodetag){
    int i ;
    int n = 0;
    char * result = malloc(sizeof(char) * 50 +1);

    for (i = 0; nodetag[i]!= '=';i++){
    }
    for (i++;nodetag[i]!= '\0';i++){
        result[n] = nodetag[i];
        n++;
    }
    result[n] = '\0';
    return result;
}

/* Main Function */
int main(int argc, char *argv[]){
    char * db_name = argv[1];
    char * way;
    char * node1;
    char * node2;
    int w;
    int n1;
    int n2;

    sqlite3 * db;
    sqlite3_stmt *stmt;

    //open databse
    int rc = sqlite3_open(db_name, &db);
    if (rc){
        fprintf(stderr, "error: %s", sqlite3_errmsg(db));
        return 1;
    }


    way=getNode(argv[2]);
    sscanf(way, "%d", &w);
    /* create a new function to calculate distance with the arguement of latitude and longitude of two node */
    sqlite3_create_function(db, "distance", 4, SQLITE_UTF8, NULL, geo_distance, NULL, NULL);
    /* write a query to select the distance of path with given wayid. Sum the distance between two node in the path in sequence */
    char * sql = "select sum(distance(n1.lat, n1.lon, n2.lat, n2.lon)) from waypoint w1, waypoint w2, node n1, node n2 where w1.wayid=? and w2.wayid=? and w1.ordinal=w2.ordinal-1 and n1.id=w1.nodeid and n2.id=w2.nodeid";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    if (rc != SQLITE_OK){
        fprintf(stderr, "error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
    return 1;
    }

    sqlite3_bind_int(stmt, 1, w);
    sqlite3_bind_int(stmt, 2, w);

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