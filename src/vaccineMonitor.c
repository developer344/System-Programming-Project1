#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "linkedList.h"
#include "BST.h"

#define NUMOFHASHFUNCTIONS 16

char *curr_date()
{
    //------------------------------------Current date------------------------------------//
    //This function calculates the current date every time its called and returns
    //a string of the date
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char year[5];
    char month[4];
    char day[4];
    char *currDate = malloc(sizeof(char) * 12);
    sprintf(year, "%d", tm.tm_year + 1900);
    sprintf(month, "%d", tm.tm_mon + 1);
    sprintf(day, "%d", tm.tm_mday);
    strcpy(currDate, day);
    strcat(currDate, "-");
    strcat(currDate, month);
    strcat(currDate, "-");
    strcat(currDate, year);
    return currDate;
}

int main(int argc, char **argv)
{
    //Checking the number of arguments
    if (argc != 5)
    {
        perror("ERROR: Wrong number of arguments!");
        exit(1);
    }
    char *citizenFile;
    int bloomsize;
    //Reading the name of the citizen file that the application is going to read the records from
    for (int i = 0; i < argc; i++)
        if (!strcmp(argv[i], "-c"))
        {
            citizenFile = malloc(sizeof(char) * strlen(argv[i + 1]));
            strcpy(citizenFile, argv[i + 1]);
            break;
        }
    //Reading the number of bytes that the bloomfilter bit array will have
    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-b"))
        {
            //Multipluing by 8 to convert from bytes to bits
            bloomsize = 8 * (atoi(argv[i + 1]));
            break;
        }
    }
    //-------------------------------------------------------------------------------------//
    FILE *inputFile = fopen(citizenFile, "r");
    //This will hold the data from its line that is read
    char line[100];
    //This will hold the data from its line that is read separetly
    char data[10][30];
    //To be used on the separation of the line inyo its compoonents
    char *key;

    CTPtr countryTree = malloc(sizeof(CT));
    countryTree_init(countryTree);

    BSTPtr citizenTree = malloc(sizeof(BST));
    BST_init(citizenTree);

    linkedListPtr virusList = malloc(sizeof(linkedList));
    list_init(virusList);

    virusPtr currVirus;
    citizenRecordPtr record;
    citizenRecordPtr duplicate;
    cTreeNodePtr countryNode;
    //------------------------------------Reading File------------------------------------//
    while (fgets(line, sizeof(line), inputFile))
    {
        //Reading line by line from the file given
        line[strcspn(line, "\n")] = 0;
        //Separating the lines into the usefull info
        key = strtok(line, " ");
        for (int i = 0; key != NULL; i++)
        {
            strcpy(data[i], key);
            key = strtok(NULL, " ");
        }
        //Checking if citizen id is within the limits
        if (atoi(data[0]) > 0 && atoi(data[0]) < 10000)
        {
            //Checking if age is valid
            if (atoi(data[4]) > 0)
            {
                //Checking  if date is valid
                char dateStr[10];
                strcpy(dateStr, data[7]);
                DatePtr date = date_init(data[7]);
                if ((date->day > 0 && date->day <= 30 && date->month > 0 && date->month <= 12 && date->year > 1900 && date->year < 3000 && !strcmp(data[6], "YES")) || !strcmp(data[6], "NO"))
                    //Checking if a citizen is already in the citizen tree
                    if ((duplicate = findcitizenRecord(citizenTree, atoi(data[0]))) == NULL)
                    {
                        //Here I check if the country of the current citizen has been added to the country
                        //tree if not I create a new node in the country tree for the new country
                        if ((countryNode = countryTree_findCountry(countryTree, data[3], atoi(data[4]))) == NULL)
                            countryNode = countryTree_insertCountry(countryTree, data[3], atoi(data[4]));
                        //Creating new citizen and inserting the unique elements only so
                        //that country, virus, date and yes/no will not be duplicated
                        record = malloc(sizeof(citizenRecord));
                        record->citizenId = atoi(data[0]);
                        record->name = malloc(sizeof(char) * strlen(data[1]));
                        strcpy(record->name, data[1]);
                        record->surname = malloc(sizeof(char) * strlen(data[2]));
                        strcpy(record->surname, data[2]);
                        //For the country each citizen has a pointer to their country
                        //Citizens from the same country point to the same country node
                        record->country = countryNode;
                        record->age = atoi(data[4]);
                        //Inserting the new citizen to the tree that contains all the citizens of every country
                        if (insertcitizenRecord(citizenTree, record) < 0)
                        {
                            perror("Error: couldn't insert record in citizen tree\n");
                        }
                        //I search in the list of viruses if the virus insetred already exists or not
                        if ((currVirus = list_searchElement(virusList, data[5])) != NULL)
                        {
                            //Then I insert the citizen to the virus
                            virus_insert(currVirus, record, data[0], data[6], dateStr);
                        }
                        else
                        {
                            //If the inserted virus doesn't exist in the virus list I create it and I insert it in the virus list
                            currVirus = malloc(sizeof(virus));

                            virus_init(currVirus, data[5], NUMOFHASHFUNCTIONS, bloomsize);
                            //I insert the new citizen to the newly created virus
                            virus_insert(currVirus, record, data[0], data[6], dateStr);
                            //I insert the new virus to the virus list
                            list_insertItem(virusList, currVirus);
                        }
                    }
                    else
                    {
                        //If the inserted citizen already exists in the citizen tree then I take the citizen
                        //and insert them in the virus that was given
                        if ((currVirus = list_searchElement(virusList, data[5])) != NULL)
                        {
                            //If the virus given already exists then I check if the citizen has already has been
                            //inserted in thatvirus
                            if (find_inVirus(currVirus, duplicate->citizenId) > 0)
                                //If the citizen is not in that virus I insert them there
                                virus_insert(currVirus, duplicate, data[0], data[6], dateStr);
                            else
                                //If the citizen is already in the virus I print error
                                printf("ERROR IN RECORD %s\n", line);
                        }
                        else
                        {
                            //If the virus given doesn't exist I create it
                            currVirus = malloc(sizeof(virus));
                            virus_init(currVirus, data[5], NUMOFHASHFUNCTIONS, bloomsize);
                            //I insert the citizen to the newly created virus
                            virus_insert(currVirus, duplicate, data[0], data[6], dateStr);
                            //I insert the new virus to the virus list
                            list_insertItem(virusList, currVirus);
                        }
                    }
                else
                    printf("ERROR! Date is invalid please enter a valid date in record %s\n", line);
                dateDestructor(date);
            }
            else
                printf("ERROR! Age is a negatice number in record %s\n", line);
        }
        else
            printf("ERROR! Citizen id is out of limits in record %s\n", line);
    }
    fclose(inputFile);
    printf("Citizens inserted correctly from file %s\n", citizenFile);
    //------------------------------------Options------------------------------------//
    int numOfArguments;
    slNodePtr Node;
    do
    {
        //I print the options available
        printf("========\n");
        printf("OPTIONS:\n");
        printf("=========================================================================================================================\n");
        printf("/vaccineStatusBloom citizenID virusName\n");
        printf("/vaccineStatus citizenID virusName\n");
        printf("/vaccineStatus citizenID\n");
        printf("/populationStatus [country] virusName date1 date2 (country is optional)\n");
        printf("/popStatusByAge [country] virusName date1 date2 (country is optional)\n");
        printf("/insertCitizenRecord citizenID firstName lastName country age virusName YES/NO [date] (date applies only for YES option)\n");
        printf("/vaccinateNow citizenID firstName lastName country age virusName\n");
        printf("/list-nonVaccinated-Persons virusName\n");
        printf("/exit\n");
        printf("=========================================================================================================================\n");

        //I read the users input

        fgets(line, sizeof(line), stdin);

        line[strcspn(line, "\n")] = 0;

        key = strtok(line, " ");

        for (numOfArguments = 0; key != NULL; numOfArguments++)
        {

            strcpy(data[numOfArguments], key);
            key = strtok(NULL, " ");
        }

        if (!strcmp(data[0], "/vaccineStatusBloom"))
        {
            //If the command is "/vaccineStatusBloom" the I check if the number of arguments given are correct
            if (numOfArguments == 3)
                //I check if the citizen id is valid
                if (atoi(data[1]) > 0 && atoi(data[1]) < 10000)
                    //I check if the virus given exists in the virus list
                    if ((currVirus = list_searchElement(virusList, data[2])) != NULL)
                        //I check if the citizen given is vaccinated through the bloomfilter
                        if (virus_checkIfVaccinatedBloom(currVirus, data[1]))
                            printf("MAYBE VACCINATED\n");
                        else
                            printf("NOT VACCINATED\n");
                    else
                        printf("ERROR! Virus %s does not exist in the database\n", data[2]);
                else
                    printf("ERROR! Wrong format: Second argument must be integer from 1 to 9999\n");
            else
                printf("ERROR! Wrong number of arguments the correct format is: \"/vaccineStatusBloom citizenID virusName\"\n");
        }
        else if (!strcmp(data[0], "/vaccineStatus"))
        {
            //If the command is "/vaccineStatus" the I check if the number of arguments given are correct
            if (numOfArguments == 3)
            {
                //If the number of arguments are 3 that means that I have to check if the citizen given is
                //vaccinated to the virus that is also given by the user

                //I check if the citizen id is valid
                int citizenID = atoi(data[1]);
                if (citizenID > 0 && citizenID < 10000)
                    //I check if the virus given exists in the virus list
                    if ((currVirus = list_searchElement(virusList, data[2])) != NULL)
                        //If the citizen is in the
                        if ((Node = skipList_findRecord(currVirus->vaccinatedVirusSL, citizenID)) != NULL)
                            printf("VACINATED ON %d-%d-%d\n", Node->date->day, Node->date->month, Node->date->year);
                        else
                            printf("NOT VACCINATED\n");
                    else
                        printf("ERROR! Virus %s does not exist in the database\n", data[2]);
                else
                    printf("ERROR! Wrong format: Second argument must be integer from 1 to 9999\n");
            }
            else if (numOfArguments == 2)
            {
                //If the number of arguments are 2 that means that I have to check if the citizen given is vaccinated or not
                //to all the viruses in the virus list

                //I check if the citizen id is valid
                int citizenID = atoi(data[1]);
                if (citizenID > 0 && citizenID < 10000)
                {
                    //I check for every virus in the virus list if the citizen given is vaccinated to that virus or not
                    listNodePtr node = virusList->Begining;
                    while (node != NULL)
                    {
                        currVirus = node->virus;
                        if ((Node = skipList_findRecord(currVirus->vaccinatedVirusSL, citizenID)) != NULL)
                        {
                            printf("%s YES %d-%d-%d\n", currVirus->name, Node->date->day, Node->date->month, Node->date->year);
                        }
                        else if (skipList_findRecord(currVirus->notVaccinatedVirusSL, citizenID) != NULL)
                            printf("%s NO\n", currVirus->name);
                        node = node->nextNode;
                    }
                }
                else
                    printf("ERROR! Wrong format: Second argument must be integer from 1 to 9999\n");
            }
            else
                printf("ERROR! Wrong number of arguments the correct format is: \"/vaccineStatus citizenID virusName\" or \"/vaccineStatus citizenID\"\n");
        }
        else if (!strcmp(data[0], "/populationStatus"))
        {
            //If the command is "/populationStatus" the I check if the number of arguments given are correct
            if (numOfArguments == 4)
            {
                //If the number of arguments is 4 it means that I have to print the number of not vaccinated people
                //of each country to the given virus

                //I check if the virus given exists in the virus list
                virusPtr currVirus;
                if ((currVirus = list_searchElement(virusList, data[1])) != NULL)
                {
                    //Checking if the dates are valid
                    DatePtr date1 = date_init(data[2]);
                    DatePtr date2 = date_init(data[3]);
                    if (date1->day > 0 && date1->day <= 30 && date1->month > 0 && date1->month <= 12 && date1->year > 1900 && date1->year < 3000)
                        if (date2->day > 0 && date2->day <= 30 && date2->month > 0 && date2->month <= 12 && date2->year > 1900 && date2->year < 3000)
                            countryTree_popStatus(countryTree, NULL, currVirus, date1, date2);
                        else
                            printf("ERROR! First date is invalid please enter a valid date!\n");
                    else
                        printf("ERROR! Second date is invalid please enter a valid date!\n");
                    dateDestructor(date1);
                    dateDestructor(date2);
                }
                else
                    printf("ERROR! Virus %s does not exist in the database\n", data[2]);
            }
            else if (numOfArguments == 5)
            {
                //If the number of arguments is 5 it means that I have to print the number of not vaccinated people of
                //the given country country to the given virus

                //I check if the virus given exists in the virus list
                virusPtr currVirus;
                if ((currVirus = list_searchElement(virusList, data[2])) != NULL)
                {
                    //Checking if the dates are valid
                    DatePtr date1 = date_init(data[3]);
                    DatePtr date2 = date_init(data[4]);
                    if (date1->day > 0 && date1->day <= 30 && date1->month > 0 && date1->month <= 12 && date1->year > 1900 && date1->year < 3000)
                        if (date2->day > 0 && date2->day <= 30 && date2->month > 0 && date2->month <= 12 && date2->year > 1900 && date2->year < 3000)
                            countryTree_popStatus(countryTree, data[1], currVirus, date1, date2);
                        else
                            printf("ERROR! First date is invalid please enter a valid date!\n");
                    else
                        printf("ERROR! Second date is invalid please enter a valid date!\n");
                    dateDestructor(date1);
                    dateDestructor(date2);
                }
                else
                    printf("ERROR! Virus %s does not exist in the database\n", data[2]);
            }
            else
            {
                printf("ERROR! Wrong number of arguments the correct format is: \"/populationStatus [country] virusName date1 date2\" (country is optional)\n");
            }
        }
        else if (!strcmp(data[0], "/popStatusByAge"))
        {
            //If the command is "/popStatusByAge" the I check if the number of arguments given are correct
            if (numOfArguments == 4)
            {
                //If the number of arguments is 4 it means that I have to print the number of not vaccinated
                //people of each country categorized by age to the given virus

                //I check if the virus given exists in the virus list
                virusPtr currVirus;
                if ((currVirus = list_searchElement(virusList, data[1])) != NULL)
                {
                    //Checking if the dates are valid
                    DatePtr date1 = date_init(data[2]);
                    DatePtr date2 = date_init(data[3]);
                    if (date1->day > 0 && date1->day <= 30 && date1->month > 0 && date1->month <= 12 && date1->year > 1900 && date1->year < 3000)
                        if (date2->day > 0 && date2->day <= 30 && date2->month > 0 && date2->month <= 12 && date2->year > 1900 && date2->year < 3000)
                            countryTree_popStatusAge(countryTree, NULL, currVirus, date1, date2);
                        else
                            printf("ERROR! First date is invalid please enter a valid date!\n");
                    else
                        printf("ERROR! Second date is invalid please enter a valid date!\n");
                    dateDestructor(date1);
                    dateDestructor(date2);
                }
                else
                    printf("ERROR! Virus %s does not exist in the database\n", data[2]);
            }
            else if (numOfArguments == 5)
            {
                //If the number of arguments is 5 it means that I have to print the number of not vaccinated
                //people of the given country categorized by age to the given virus

                //I check if the virus given exists in the virus list
                virusPtr currVirus;
                if ((currVirus = list_searchElement(virusList, data[2])) != NULL)
                {
                    //Checking if the dates are valid
                    DatePtr date1 = date_init(data[3]);
                    DatePtr date2 = date_init(data[4]);
                    if (date1->day > 0 && date1->day <= 30 && date1->month > 0 && date1->month <= 12 && date1->year > 1900 && date1->year < 3000)
                        if (date2->day > 0 && date2->day <= 30 && date2->month > 0 && date2->month <= 12 && date2->year > 1900 && date2->year < 3000)
                            countryTree_popStatusAge(countryTree, data[1], currVirus, date1, date2);
                        else
                            printf("ERROR! First date is invalid please enter a valid date!\n");
                    else
                        printf("ERROR! Second date is invalid please enter a valid date!\n");
                    dateDestructor(date1);
                    dateDestructor(date2);
                }
                else
                    printf("ERROR! Virus %s does not exist in the database\n", data[2]);
            }
            else
            {
                printf("ERROR! Wrong number of arguments the correct format is: \"popStatusByAge [country] virusName datpopStatusByAge [country] virusName date1 date2 (country is optional)e1 date2\" (country is optional)\n");
            }
        }
        else if (!strcmp(data[0], "/insertCitizenRecord"))
        {
            //If the command is "/insertCitizenRecord" the I check if the number of arguments given are correct accorging to the available options
            if ((numOfArguments == 9 && !strcmp("YES", data[7])) || (numOfArguments == 8 && !strcmp("NO", data[7])))
            {
                //Checking if citizen id is within the limits
                if (atoi(data[1]) > 0 && atoi(data[1]) < 10000)
                {
                    //Checking if age is valid
                    if (atoi(data[5]) > 0)
                    {
                        //Checking  if date is valid
                        char dateStr[10];
                        strcpy(dateStr, data[8]);
                        DatePtr date = date_init(data[8]);
                        if ((date->day > 0 && date->day <= 30 && date->month > 0 && date->month <= 12 && date->year > 1900 && date->year < 3000 && !strcmp(data[7], "YES")) || !strcmp(data[7], "NO"))
                            //Checking if a citizen is already in the citizen tree
                            if ((duplicate = findcitizenRecord(citizenTree, atoi(data[1]))) == NULL)
                            {
                                //Here I check if the country of the current citizen has been added to the country
                                //tree if not I create a new node in the country tree for the new country
                                if ((countryNode = countryTree_findCountry(countryTree, data[4], atoi(data[5]))) == NULL)
                                    countryNode = countryTree_insertCountry(countryTree, data[4], atoi(data[5]));
                                //Creating new citizen and inserting the unique elements only so
                                //that country, virus, date and yes/no will not be duplicated
                                record = malloc(sizeof(citizenRecord));
                                record->citizenId = atoi(data[1]);
                                record->name = malloc(sizeof(char) * strlen(data[2]));
                                strcpy(record->name, data[2]);
                                record->surname = malloc(sizeof(char) * strlen(data[3]));
                                strcpy(record->surname, data[3]);
                                //For the country each citizen has a pointer to their country
                                //Citizens from the same country point to the same country node
                                record->country = countryNode;
                                record->age = atoi(data[5]);
                                //Inserting the new citizen to the tree that contains all the citizens of every country
                                if (insertcitizenRecord(citizenTree, record) < 0)
                                {
                                    perror("Error: couldn't insert record in citizen tree\n");
                                }
                                //I search in the list of viruses if the virus insetred already exists or not
                                if ((currVirus = list_searchElement(virusList, data[6])) != NULL)
                                {
                                    //Then I insert the citizen to the virus
                                    virus_insert(currVirus, record, data[1], data[7], dateStr);
                                }
                                else
                                {
                                    //If the inserted virus doesn't exist in the virus list I create it and I insert it in the virus list
                                    currVirus = malloc(sizeof(virus));
                                    virus_init(currVirus, data[6], NUMOFHASHFUNCTIONS, bloomsize);
                                    //I insert the new citizen to the newly created virus
                                    virus_insert(currVirus, record, data[1], data[7], dateStr);
                                    //I insert the new virus to the virus list
                                    list_insertItem(virusList, currVirus);
                                }
                            }
                            else
                            {
                                //If the inserted citizen already exists in the citizen tree then I take the citizen
                                //and insert them in the virus that was given
                                if ((currVirus = list_searchElement(virusList, data[6])) != NULL)
                                {
                                    if (find_inVirus(currVirus, duplicate->citizenId) > 0)
                                        //If the citizen is not in that virus I insert them there
                                        virus_insert(currVirus, duplicate, data[1], data[7], dateStr);
                                    else
                                    {
                                        //If the citizen is already in the virus I check whether the citizen is in the
                                        //vaccinated or non-vsccinsted skip list
                                        if ((Node = skipList_findRecord(currVirus->vaccinatedVirusSL, duplicate->citizenId)) != NULL)
                                            //If the citizen is already vaccinated I print an error accordingly
                                            printf("ERROR: CITIZEN %d ALREADY VACCINATED ON %d-%d-%d\n", Node->citizenId, Node->date->day, Node->date->month, Node->date->year);
                                        else if (skipList_findRecord(currVirus->notVaccinatedVirusSL, duplicate->citizenId) != NULL)
                                            //If the citizen is not vaccinated I check if the user inserted the option YES or NO
                                            if (!strcmp(data[7], "YES"))
                                            {
                                                //If the user inserted the option yes then I remove the citizen from the non-vaccinated
                                                //skip list and I insert them in the vaccinated skiplist
                                                skipList_deleteCitizen(currVirus->notVaccinatedVirusSL, duplicate->citizenId);
                                                virus_insert(currVirus, duplicate, data[1], data[7], dateStr);
                                            }
                                    }
                                }
                                else
                                {
                                    //If the virus given doesn't exist I create it
                                    currVirus = malloc(sizeof(virus));
                                    virus_init(currVirus, data[6], NUMOFHASHFUNCTIONS, bloomsize);
                                    //I insert the citizen to the newly created virus
                                    virus_insert(currVirus, duplicate, data[1], data[7], dateStr);
                                    //I insert the new virus to the virus list
                                    list_insertItem(virusList, currVirus);
                                }
                            }
                        else
                            printf("ERROR! Date is invalid please enter a valid date!\n");
                        dateDestructor(date);
                    }
                    else
                        printf("ERROR! Age is a negatice number\n");
                }
                else
                    printf("ERROR! Citizen id is out of limits\n");
            }
            else
                printf("Wrong number of arguments! Correct format is \"/insertCitizenRecord citizenID firstName lastName country age virusName YES/NO [date] (date applies only for YES option)\"\n");
        }
        else if (!strcmp(data[0], "/vaccinateNow"))
        {
            //Checking if citizen id is within the limits
            if (atoi(data[1]) > 0 && atoi(data[1]) < 10000)
            {
                //Checking if age is valid
                if (atoi(data[5]) > 0)
                {
                    char *currDate = curr_date();
                    //Checking if a citizen is already in the citizen tree
                    if ((duplicate = findcitizenRecord(citizenTree, atoi(data[1]))) == NULL)
                    {
                        //Here I check if the country of the current citizen has been added to the country
                        //tree if not I create a new node in the country tree for the new country
                        if ((countryNode = countryTree_findCountry(countryTree, data[4], atoi(data[5]))) == NULL)
                            countryNode = countryTree_insertCountry(countryTree, data[4], atoi(data[5]));
                        //Creating new citizen and inserting the unique elements only so
                        //that country, virus, date and yes/no will not be duplicated
                        record = malloc(sizeof(citizenRecord));
                        record->citizenId = atoi(data[1]);
                        record->name = malloc(sizeof(char) * strlen(data[2]));
                        strcpy(record->name, data[2]);
                        record->surname = malloc(sizeof(char) * strlen(data[3]));
                        strcpy(record->surname, data[3]);
                        //For the country each citizen has a pointer to their country
                        //Citizens from the same country point to the same country node
                        record->country = countryNode;
                        record->age = atoi(data[5]);
                        //Inserting the new citizen to the tree that contains all the citizens of every country
                        if (insertcitizenRecord(citizenTree, record) < 0)
                        {
                            perror("Error: couldn't insert record in citizen tree\n");
                        }
                        //I search in the list of viruses if the virus insetred already exists or not
                        if ((currVirus = list_searchElement(virusList, data[6])) != NULL)
                        {
                            //Then I insert the citizen to the virus with the option YES
                            virus_insert(currVirus, record, data[1], "YES", currDate);
                        }
                        else
                        {
                            //If the inserted virus doesn't exist in the virus list I create it and I insert it in the virus list
                            currVirus = malloc(sizeof(virus));
                            virus_init(currVirus, data[6], NUMOFHASHFUNCTIONS, bloomsize);
                            //I insert the new citizen to the newly created virus
                            virus_insert(currVirus, record, data[1], "YES", currDate);
                            //I insert the new virus to the virus list
                            list_insertItem(virusList, currVirus);
                        }
                    }
                    else
                    {
                        //If the inserted citizen already exists in the citizen tree then I take the citizen
                        //and insert them in the virus that was given
                        if ((currVirus = list_searchElement(virusList, data[6])) != NULL)
                        {
                            //If the inserted citizen already exists in the citizen tree then I take the citizen
                            //and insert them in the virus that was given
                            if (find_inVirus(currVirus, duplicate->citizenId) > 0)
                                //If the citizen is not in that virus I insert them there
                                virus_insert(currVirus, duplicate, data[1], "YES", currDate);
                            else
                            {
                                //If the citizen is already in the virus I check whether the citizen is in the
                                //vaccinated or non-vsccinsted skip list
                                if ((Node = skipList_findRecord(currVirus->vaccinatedVirusSL, duplicate->citizenId)) != NULL)
                                    //If the citizen is already vaccinated I print an error accordingly
                                    printf("ERROR: CITIZEN %d ALREADY VACCINATED ON %d-%d-%d\n", Node->citizenId, Node->date->day, Node->date->month, Node->date->year);
                                else if (skipList_findRecord(currVirus->notVaccinatedVirusSL, duplicate->citizenId) != NULL)
                                {
                                    //If the citizen is not vaccinated I remove the citizen from the non-vaccinated
                                    //skip list and I insert them in the vaccinated skiplist
                                    skipList_deleteCitizen(currVirus->notVaccinatedVirusSL, duplicate->citizenId);
                                    virus_insert(currVirus, duplicate, data[1], "YES", currDate);
                                }
                            }
                        }
                        else
                        {
                            //If the virus given doesn't exist I create it
                            currVirus = malloc(sizeof(virus));
                            virus_init(currVirus, data[6], NUMOFHASHFUNCTIONS, bloomsize);
                            //I insert the citizen to the newly created virus
                            virus_insert(currVirus, duplicate, data[1], "YES", currDate);
                            //I insert the new virus to the virus list
                            list_insertItem(virusList, currVirus);
                        }
                    }
                    free(currDate);
                }
                else
                    printf("ERROR! Age is a negatice number in record %s\n", line);
            }
            else
                printf("ERROR! Citizen id is out of limits in record %s\n", line);
        }
        else if (!strcmp(data[0], "/list-nonVaccinated-Persons"))
        {
            //If the command is "/list-nonVaccinated-Persons" the I check if the number of arguments given are correct
            if (numOfArguments == 2)
            {
                //I check if the virus given exists in the virus list
                if ((currVirus = list_searchElement(virusList, data[1])) != NULL)
                {
                    printf("========================================\n");
                    printf("Non vaccinated persons for the virus %s:\n", currVirus->name);
                    printf("========================================\n");
                    //Printing the contents of the skip list that contains the non vaccinated citizens for the given virus
                    skipList_print(currVirus->notVaccinatedVirusSL);
                }
                else
                    printf("ERROR! Virus %s does not exist in the database\n", data[1]);
            }
            else
                printf("Wrong number of arguments! Correct format is: \"/list-nonVaccinated-Persons virusName\"\n");
        }
        else if (strcmp(data[0], "/exit"))
        {
            //If the command that the user iserted doesn't exist I print an error
            printf("There is no command \"%s\" please choose again from the options below\n", data[0]);
        }
    } while (strcmp(data[0], "/exit"));
    //destruction of all previously created data structures
    countryTree_destructor(countryTree);
    list_deleteList(virusList);
    BST_destructor(citizenTree);
    free(citizenFile);
    free(key);
    return 0;
}