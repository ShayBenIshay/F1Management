/*
 * season.c
 */


#include<stdio.h>
#include<stdlib.h>
#include"driver.h"
#include"team.h"
#include"season.h"
#include<assert.h>
#include<string.h>

struct season {
    int season_year;
    int season_teams_number;
    Team* season_teams;
    int season_drivers_number;
    Driver* season_drivers;
};

int SeasonTokensCounter (const char* text);
int SeasonGetYear (const char* text);
int SeasonDriversCounter (const char* text);
Driver GetDriverById(Season season, int Id);
void ArrayCopy (int* source, int* destination, int length);

static int* last_race_results = NULL;

//SeasonCreate builds new season and returns season.
Season SeasonCreate(SeasonStatus* status,const char* season_info) {
    if ( season_info == NULL ){
        if ( status == NULL ) return NULL;
        *status = BAD_SEASON_INFO;
        return NULL;
    }
    Season season = malloc(sizeof(*season));
    if (season == NULL) {
        if (status == NULL) return NULL;
        *status = SEASON_MEMORY_ERROR;
        return NULL;
    } else {
        //lets parse season_info string
        season->season_teams_number = 0;
        season->season_drivers_number = 0;
        //loop to count the number of tokens
        int counter_tokens = SeasonTokensCounter(season_info);
        //first extract first row with year
        season->season_year = SeasonGetYear(season_info);
        //calculate how many drivers and teams and allocate memory
        season->season_drivers_number = SeasonDriversCounter(season_info);
        season->season_teams_number = (counter_tokens - 1) / 3;
        season->season_drivers = malloc(sizeof(season->season_drivers) * (season->season_drivers_number));
        if (season->season_drivers == NULL) {
            if (status == NULL) return NULL;
            *status = SEASON_MEMORY_ERROR;
            return NULL;
        }
        season->season_teams = malloc(sizeof(season->season_teams) * (season->season_teams_number));
        if (season->season_teams == NULL) {
            if (status == NULL) return NULL;
            *status = SEASON_MEMORY_ERROR;
            return NULL;
        }
        //then parse 3 rows each time: first is team, rest are drivers.
        int size_line = 0;
        int text_size = strlen(season_info);
        char *text_copy = malloc(sizeof(char) * (text_size + 1));
        strcpy(text_copy, season_info);
        char *current_line = text_copy;
        //skip first line with year
        current_line = strtok(current_line, "\n");
        size_line = strlen(current_line) + sizeof(char);
        current_line += size_line * sizeof(char);
        TeamStatus team_status;
        DriverStatus driver1_status;
        DriverStatus driver2_status;
        int i_teams = 0, i_drivers = 0;
        for (int i = 0; i < season->season_teams_number; i++) {
            //add team
            current_line = strtok(current_line, "\n");
            size_line = strlen(current_line) + sizeof(char);
            //size_total+=size_line;
            season->season_teams[i_teams] = TeamCreate(&team_status, current_line);
            current_line += size_line * sizeof(char);
            //add driver1
            current_line = strtok(current_line, "\n");
            size_line = strlen(current_line) + sizeof(char);
            season->season_drivers[i_drivers] = DriverCreate(&driver1_status, current_line, i_drivers + 1);
            DriverSetSeason(season->season_drivers[i_drivers], season);
            DriverSetTeam(season->season_drivers[i_drivers], season->season_teams[i_teams]);
            TeamAddDriver(season->season_teams[i_teams], season->season_drivers[i_drivers]);
            i_drivers++;
            current_line += size_line * sizeof(char);
            //add driver2
            current_line = strtok(current_line, "\n");
            size_line = strlen(current_line) + sizeof(char);
            if (strcmp(current_line, "None")) {
                season->season_drivers[i_drivers] = DriverCreate(&driver2_status, current_line, i_drivers + 1);
                DriverSetSeason(season->season_drivers[i_drivers], season);
                DriverSetTeam(season->season_drivers[i_drivers], season->season_teams[i_teams]);
                TeamAddDriver(season->season_teams[i_teams], season->season_drivers[i_drivers]);
                i_drivers++;
            }
            current_line += size_line * sizeof(char);
            i_teams++;
        }
        free(text_copy);
        //allocate memory for a copy of most recent race results
        last_race_results = malloc(sizeof(int) * season->season_drivers_number);
        if (last_race_results == NULL) {
            if (status == NULL) return NULL;
            *status = SEASON_MEMORY_ERROR;
            return NULL;
        }
        if (status == NULL) return season;
        *status = SEASON_OK;
        return season;
    }
}

void   SeasonDestroy(Season season){
    //for loop to release all teams array with TeamDestroy
    if ( season == NULL ) return;
    if ( season->season_drivers != NULL ) {
        for (int i=0;i<season->season_drivers_number;i++) {
            DriverDestroy(season->season_drivers[i]);
        }
        free(season->season_drivers);
    }
    if ( season->season_teams != NULL ) {
        for (int i=0;i<season->season_teams_number;i++) {
            TeamDestroy(season->season_teams[i]);
        }
        free(season->season_teams);
    }
    //free last_race_results array and the season itself
    free(last_race_results);
    free(season);
    return;
}

Driver SeasonGetDriverByPosition(Season season, int position, SeasonStatus* status) {
    if (season == NULL) {
        if (status == NULL) return NULL;
        *status = SEASON_NULL_PTR;
        return NULL;
    } else if (position < 1 || position > season->season_drivers_number) {
        if (status == NULL) return NULL;
        *status = SEASON_NULL_PTR;
        return NULL;
    } else {
        Driver *season_drivers_sorted = SeasonGetDriversStandings(season);
        if (season_drivers_sorted == NULL) return NULL;
        if (status == NULL) return season_drivers_sorted[position - 1];
        *status = SEASON_OK;
        return season_drivers_sorted[position - 1];
    }
}

//SeasonGetDriversStandings sorting the season->season_drivers array and returns a copy.
Driver* SeasonGetDriversStandings(Season season){
    if (season==NULL) return NULL;
    int drivers_number = season->season_drivers_number;
    DriverStatus driver1_status, driver2_status;
    //allocating memory for the copy of season->season_drivers
    Driver* season_drivers_copy = malloc(sizeof(Driver)*drivers_number);
    if (season_drivers_copy == NULL) return NULL;
    for (int j=0;j<drivers_number-1;j++) {
        for (int i = 0; i < drivers_number-1-j; i++) {
            Driver* driver1 = &(season->season_drivers[i]);
            Driver* driver2 = &(season->season_drivers[i + 1]);
            //WE DEFINE STATUS PARAMETERS AND DON'T CHECK THEM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            int driver1_points = DriverGetPoints(*driver1, &driver1_status);
            int driver2_points = DriverGetPoints(*driver2, &driver2_status);
            if (driver1_points < driver2_points) {
                Driver driver_temp = season->season_drivers[i];
                season->season_drivers[i] = season->season_drivers[i+1];
                season->season_drivers[i+1] = driver_temp;
            } else if (driver1_points == driver2_points) {
                int driver1_id = DriverGetId(*driver1);
                int driver2_id = DriverGetId(*driver2);
                for (int k = 0; k < drivers_number; k++) {
                    if (last_race_results[k] == driver2_id) {
                        Driver driver_temp = season->season_drivers[i];
                        season->season_drivers[i] = season->season_drivers[i+1];
                        season->season_drivers[i+1] = driver_temp;
                    } else if (last_race_results[k] == driver1_id) break;
                }
            }
        }
    }
    //the loop duplicate the sorted array and returns the copy
    for(int i=0;i<drivers_number;i++){
        season_drivers_copy[i] = season->season_drivers[i];
    }
    return season_drivers_copy;
}

Team SeasonGetTeamByPosition(Season season, int position, SeasonStatus* status){
    if (season == NULL) {
        if (status == NULL) return NULL;
        *status = SEASON_NULL_PTR;
        return NULL;
    } else if ( position < 1 || position > season->season_teams_number ) {
        if ( status == NULL ) return NULL;
        *status = SEASON_NULL_PTR;
        return NULL;
    } else {
        Team* season_teams_sorted = SeasonGetTeamsStandings(season);
        if ( season_teams_sorted == NULL ) return NULL;
        if ( status == NULL ) return season_teams_sorted[position-1];
        *status = SEASON_OK;
        return season_teams_sorted[position-1];
    }
}

Team* SeasonGetTeamsStandings(Season season){
    if (season==NULL) return NULL;
    int teams_number = season->season_teams_number;
    //allocating memory for the copy of season->season_drivers
    Team* season_teams_copy = malloc(sizeof(Team)*teams_number);
    if (season_teams_copy == NULL) return NULL;
    for (int j=0;j<teams_number-1;j++) {
        for (int i = 0; i < teams_number-1-j; i++) {
            Team* team1 = &(season->season_teams[i]);
            Team* team2 = &(season->season_teams[i + 1]);
            int team1_points = TeamGetPoints(*team1, NULL);
            int team2_points = TeamGetPoints(*team2, NULL);
            if (team1_points < team2_points) {
                Team team_temp = season->season_teams[i];
                season->season_teams[i] = season->season_teams[i+1];
                season->season_teams[i+1] = team_temp;
            } else if (team1_points == team2_points) {
                Driver team1_driver1 = TeamGetDriver(*team1, FIRST_DRIVER);
                Driver team1_driver2 = TeamGetDriver(*team1, SECOND_DRIVER);
                Driver team2_driver1 = TeamGetDriver(*team2, FIRST_DRIVER);
                Driver team2_driver2 = TeamGetDriver(*team2, SECOND_DRIVER);
                int team1_driver1_id = DriverGetId(team1_driver1);
                int team1_driver2_id = DriverGetId(team1_driver2);
                int team2_driver1_id = DriverGetId(team2_driver1);
                int team2_driver2_id = DriverGetId(team2_driver2);
                for (int k = 0; k < season->season_drivers_number; k++) {
                    if (last_race_results[k] == team2_driver1_id ||\
                     last_race_results[k] == team2_driver2_id) {
                        Team team_temp = season->season_teams[i];
                        season->season_teams[i] = season->season_teams[i + 1];
                        season->season_teams[i + 1] = team_temp;
                        break;
                    } else if (last_race_results[k] == team1_driver1_id ||\
                     last_race_results[k] == team1_driver2_id) break;
                }
            }
        }
    }
    //the loop duplicate the sorted array and returns the copy
    for(int i=0;i<teams_number;i++){
        season_teams_copy[i] = season->season_teams[i];
    }
    return season_teams_copy;
}

int SeasonGetNumberOfDrivers(Season season){
		if (season==NULL)	return 0;
		return season->season_drivers_number;
}

int SeasonGetNumberOfTeams(Season season){
	if (season==NULL)	return 0;
		return season->season_teams_number;
}

SeasonStatus SeasonAddRaceResult(Season season, int* results){
    //check if season is NULL
    if(season == NULL || results ==NULL) return SEASON_NULL_PTR;
    //save a copy of most recent race results
    ArrayCopy(results,last_race_results, season->season_drivers_number);
    int drivers_number = season->season_drivers_number;
    //add results
    for(int i=0 ; i<drivers_number ; i++){
        DriverAddRaceResult(GetDriverById(season, results[i]),i+1);
    }
    return SEASON_OK;
}

//internal functions

//ArrayCopy copies array from type int from destination to source
void ArrayCopy (int* source, int* destination, int length){
    for (int i=0; i<length;i++){
        destination[i]=source[i];
    }
    return;
}

//GetDriverById gets position and returns driver accordingly
Driver GetDriverById(Season season, int Id){
    int drivers_number = season->season_drivers_number;
    for (int i_drivers=0;i_drivers<drivers_number;i_drivers++){
        if (Id==DriverGetId(season->season_drivers[i_drivers])) return season->season_drivers[i_drivers];
    }
    //no driver was found with a given Id
    return NULL;
}

//TokensCounter counts and returns the number of tokens in a given text.
int SeasonTokensCounter (const char* text){
	int count=0, size_total=0,size_line=0, text_size=0;
	text_size = strlen(text);

    char* line = malloc(sizeof(char)*(text_size+1));
    strcpy(line,text);
    char* end_of_line=line;
	while (size_total<text_size) {
        end_of_line = strtok(end_of_line,"\n");
        size_line=strlen(end_of_line)+sizeof(char);
        size_total+=size_line;
        end_of_line+=size_line*sizeof(char);
		count++;
	}
    free (line);
    return count;
}

//SeasonGetYear gets text, extracts the first row and returns year.
int SeasonGetYear (const char* text){
    char* text_copy = malloc(sizeof(char)*(strlen(text)+1));
    strcpy(text_copy,text);
    text_copy = strtok(text_copy,"\n");
	int year =  atoi(text_copy);//convert string to int
    free (text_copy);
	return year;
}

//SeasonDriversCounter counts "None" in a given text and returns the number of drivers.
int SeasonDriversCounter (const char* text){
	int count=0, size_total=0,size_line=0, text_size=0, none_counter=0;
	text_size = strlen(text);
    char* line = malloc(sizeof(char)*(text_size+1));
    strcpy(line,text);
    char* end_of_line=line;
    //skip year and continue to parsing
    end_of_line = strtok(end_of_line,"\n");
    size_line=strlen(end_of_line)+sizeof(char);
    size_total+=size_line;
    end_of_line+=size_line*sizeof(char);
    count++;
	while (size_total<text_size) {
        end_of_line = strtok(end_of_line,"\n");
        //skip team and continue to count drivers
        size_line=strlen(end_of_line)+sizeof(char);
        size_total+=size_line;
        end_of_line+=size_line*sizeof(char);
        count++;
        //count first driver
        end_of_line = strtok(end_of_line,"\n");
        if (!strcmp(end_of_line,"None")) none_counter++;
        size_line=strlen(end_of_line)+sizeof(char);
        size_total+=size_line;
        end_of_line+=size_line*sizeof(char);
        count++;
        //count second driver
        end_of_line = strtok(end_of_line,"\n");
        if (!strcmp(end_of_line,"None")) none_counter++;
        size_line=strlen(end_of_line)+sizeof(char);
        size_total+=size_line;
        end_of_line+=size_line*sizeof(char);
		count++;
	}
    free (line);
    return (((count-1)*2)/3)-none_counter;
}

//end of internal functions

