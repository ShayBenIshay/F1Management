/*
 * team.c
 */

#include<stdio.h>
#include<stdlib.h>
#include"driver.h"
#include"team.h"
#include"season.h"
#include<assert.h>
#include<string.h>

struct team {
    char* team_name;
    Driver team_driver1;
    Driver team_driver2;
};

//TeamCreate builds new team and returns team. If failed, returns NULL
Team TeamCreate(TeamStatus* status, char* name){
    if ( name == NULL ){
        if ( status == NULL ) return NULL;
        *status = TEAM_NULL_PTR;
        return NULL;
    } else {
        int length = strlen(name);
        Team team = malloc(sizeof(*team));
        char *temp_team_name = malloc(sizeof(char) * (length + 1));
        //test if malloc works or failed
        if (temp_team_name == NULL || team == NULL) {
            if (status == NULL) return NULL;
            *status = TEAM_MEMORY_ERROR;
            return NULL;
        } else {
            strcpy(temp_team_name, name);
            team->team_name = temp_team_name;
            team->team_driver1 = NULL;
            team->team_driver2 = NULL;
            if (status == NULL) return team;
            *status = TEAM_STATUS_OK;
            return team;
        }
    }
}

void TeamDestroy(Team team){
    if ( team == NULL ) return;
    free(team->team_name);
    free(team);
    return;
}

TeamStatus TeamAddDriver(Team team, Driver driver){
    if (driver==NULL || team==NULL){
        return TEAM_NULL_PTR;
    } else if (team->team_driver1!=NULL && team->team_driver2!=NULL) {
        return TEAM_FULL;
    } else if (team->team_driver1==NULL) {
        team->team_driver1 = driver;
        return TEAM_STATUS_OK;
    } else {
        team->team_driver2 = driver;
        return TEAM_STATUS_OK;
    }
}

const char * TeamGetName(Team  team){
    if (team ==NULL)    return NULL;
    return team->team_name;
}
Driver TeamGetDriver(Team  team, DriverNumber driver_number){
    if (team==NULL || (driver_number!=0 && driver_number!=1)) {
        return NULL;
    }
    if (driver_number==0)	return team->team_driver1;
    if (driver_number==1)	return team->team_driver2;
    return NULL;
}

int TeamGetPoints(Team  team, TeamStatus *status){
    // in case team is NULL, return NULL
    if ( team == NULL ) {
        if ( status == NULL ) return 0;
        *status = TEAM_NULL_PTR;
        return 0;
    } else {
        DriverStatus driver1_status, driver2_status;
        // in case team is not NULL, test how many drivers in team
        int driver1_points = DriverGetPoints(team->team_driver1, &driver1_status);
        int driver2_points = DriverGetPoints(team->team_driver2, &driver2_status);
        //test if status invalid
        if (driver1_status == INVALID_DRIVER) {
            if (status == NULL) return 0;
            *status = TEAM_STATUS_OK;
            return 0;
        } else if (driver2_status == INVALID_DRIVER) {
            // in case no drivers in team, return 0
            if (status == NULL) return driver1_points;
            *status = TEAM_STATUS_OK;
            return driver1_points; // in case 1  driver, returns his points.
        } else {
            if ( status == NULL ) return driver1_points + driver2_points;
            *status = TEAM_STATUS_OK;
            return driver1_points + driver2_points; // in case 2 drivers exist, return the sum of two drivers.
        }
    }
}
