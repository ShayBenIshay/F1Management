/*
 * driver.c
 */

#include<stdio.h>
#include<stdlib.h>
#include"driver.h"
#include"team.h"
#include"season.h"
#include<assert.h>
#include<string.h>

struct driver {
    int driver_id;
    char* driver_name;
    Team driver_team;
    int driver_points;
    Season driver_season;
};

//DriverCreate builds new driver and returns driver. If failed, returns NULL
Driver DriverCreate(DriverStatus* status, char* driver_name, int driverId) {
    if (driver_name==NULL || driverId<1){
        if (status==NULL) return NULL;
        *status=INVALID_DRIVER;
        return NULL;
    } else {
        int length = strlen(driver_name);
        char *driver_temp = malloc(sizeof(char) * (length + 1));
        Driver driver =  malloc(sizeof(*driver));
        //test if malloc works or failed
        if (driver_temp == NULL || driver == NULL) {
            if (status == NULL) return NULL;
            *status = DRIVER_MEMORY_ERROR;
            return NULL;
        } else {
            strcpy(driver_temp, driver_name);
            driver->driver_name = driver_temp;
            driver->driver_id = driverId;
            driver->driver_team = NULL;
            driver->driver_points = 0;
            driver->driver_season = NULL;
            if (status == NULL) return driver;
            *status = DRIVER_STATUS_OK;
            return driver;
        }
    }
}

//DriverDestroy removes a driver from memory
void   DriverDestroy(Driver driver){
    if( driver == NULL ) return;
    free(driver->driver_name);
    free(driver);
    return;
}

const char* DriverGetName(Driver driver){
    if (driver==NULL)   return NULL;
    return driver->driver_name;
}

int DriverGetId(Driver driver){
    if (driver==NULL)   return 0;
    return driver->driver_id;
}

Team  DriverGetTeam(Driver driver){
    if (driver==NULL)   return NULL;
    return driver->driver_team;
}

void  DriverSetTeam(Driver driver, Team team){
    if ( driver == NULL ) return;
    driver->driver_team=team;
    return;
}

void  DriverSetSeason(Driver driver, Season season){
    if (driver == NULL || season == NULL ) return;
    driver->driver_season=season;
    driver->driver_points=0;
    return;
}

DriverStatus DriverAddRaceResult(Driver driver, int position){
    // test if position<1 then change status
    if (driver==NULL)   return INVALID_DRIVER;
    if (driver->driver_season == NULL) {
        return SEASON_NOT_ASSIGNED;
    }
    int drivers_number = SeasonGetNumberOfDrivers(driver->driver_season);
    if (position<1 || position > drivers_number)     return INVALID_POSITION;
    // calculates points based on number of participants and given position
    int NumberOfDrivers = SeasonGetNumberOfDrivers(driver->driver_season);
    driver->driver_points+=NumberOfDrivers-position;
    return DRIVER_STATUS_OK;
}

int DriverGetPoints(Driver driver, DriverStatus* status){
    if (driver==NULL){
        if (status == NULL ) return 0;
        *status=INVALID_DRIVER;
        return 0;
    } else {
        if (status == NULL ) return driver->driver_points;
        *status = DRIVER_STATUS_OK;
        return driver->driver_points;
    }
}