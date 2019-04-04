/**
 * Author: Linda Ngo
 * StudentID: 1004683
 */

#include "CalendarParser.h"
#include "LinkedListAPI.h"
#include <ctype.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

//NOTE: NEED TO VALIDATE DATETIME OBJ AND RETURN INV_EVENT - check if less than the required length
// strlen(date) must be equal to 8
// strlen(time) must be 6
//alarm trigger value must not be null

/********************** helper function prototypes ****************************/

/**
return type | function name | paramaters */

//custom string functions
char*         myStrCat       ( char** str1, char* str2 );
int           myStrCmp       ( char*  str1, char* str2 );

//used to create objects
ICalErrorCode createEvent    ( FILE** fp,    Event*    objEvent );
ICalErrorCode createDate     ( char*  value, DateTime* objDate  );
ICalErrorCode createAlarm    ( FILE** fp,    Alarm*    objAlarm );

//used for parsing a content line
char*         getProperty    ( char*  contentLine );
char*         getValue       ( char*  contentLine );
char*         getContentLine ( FILE** fp );

//required to unfold
ICalErrorCode unfold         ( char** cl1,   FILE **fp );

char* writeProperty(void* toBePrinted);
char* writeAlarm(void* toBePrinted);
char* writeDate(void* toBePrinted);
char* writeEvent(void* toBePrinted);
char* writeList(List * list, char* (*writeFunction)(void* toBePrinted));
//ICalErrorCode writeCalendar(char* fileName, const Calendar* obj);

void validateCalProperties(List* properties, ICalErrorCode* err);
int  isValidCalProp(Property* prop);

void validateEvent(const Event* obj, ICalErrorCode* err);
void validateEvtProperties(List* properties, ICalErrorCode* err);
int  isValidEvtProp(Property* prop);
int  isValidEvtPropPlus(Property* prop);

void validateAlarm(Alarm* obj, ICalErrorCode* err);
void validateAlarmProperties(List* properties, ICalErrorCode* err);

int  numOccurencesValid(List* list, char* property, int numOccurenceAllowed);

/************************ helper function definitions **************************/

//strcats str2 onto str1, reallocs more memory
char* myStrCat( char** str1, char* str2 ){

  //if any of the values are invalid, return NULL
  if ( *str1 == NULL || str2 == NULL ){
    return NULL;
  }

  //reallocate memory for str1 to account for added characters
  *str1 = realloc( *str1, strlen( *str1 ) + strlen( str2 ) + 3);
  if ( *str1 == NULL ){
    return NULL;
  }

  return strcat( *str1, str2 );
}

//function for compare (case insensitive)
int myStrCmp(char* str1, char* str2){

  if ( str1 == NULL || str2 == NULL ){
    return 0;
  }

  int  c = 0;

  char temp1[strlen(str1)+500];
  char temp2[strlen(str2)+500];


  strcpy( temp1, str1 );
  strcpy( temp2, str2 );

  //convert temp1 to upper
  while ( temp1[c] != '\0' ) {
     if ( temp1[c] >= 'a' && temp1[c] <= 'z' ) {
        temp1[c] = temp1[c] - 32;
     }
     c++;
  }

  //conver temp2 to upper
  c = 0;
  while ( temp2[c] != '\0' ) {
     if ( temp2[c] >= 'a' && temp2[c] <= 'z' ) {
        temp2[c] = temp2[c] - 32;
     }
     c++;
  }

  return strcmp( temp1, temp2 );
}

//create alarm will do while the content line is not end valarm (DONE)
ICalErrorCode createAlarm(FILE **fp, Alarm* objAlarm){

  if( fp == NULL || *fp == NULL || objAlarm == NULL ){ //invalid params
    return OTHER_ERROR;
  }
  //init variables
  char* contentLine = NULL;
  char* property    = NULL;
  char* value       = NULL;

  int   exit        = FALSE;

  int   numAction   = 0;
  int   numTrigger  = 0;

  //attempt to initialize the list
  objAlarm->properties = initializeList( &printProperty, &deleteProperty, &compareProperties );

  objAlarm->trigger = NULL;

  //failure to initialize list
  if( objAlarm->properties == NULL ){
    return OTHER_ERROR;
  }

  do{

    contentLine = getContentLine( fp );

    if( contentLine == NULL ){ //invalid line endings
      free( contentLine );
      return INV_FILE;
    }

    property    = getProperty( contentLine );
    if(strcmp(property, "") == 0){
      free(contentLine);
      free(property);
      return INV_ALARM;
    }

    value       = getValue   ( contentLine );

    if( myStrCmp( property, "END" ) == 0 ){

      if( myStrCmp( value, "VALARM" ) == 0 ){
        exit = TRUE;
      } else { //missing end tag
        free( contentLine );
        free( value       );
        free( property    );
        return INV_ALARM; //missing closing tag/incorrect closing tag
      }

    } else if ( myStrCmp( property,"ACTION" ) == 0 ){

      //too many actions or malformed values
      if( ( ++numAction ) != 1 || value == NULL || strcmp( value, "" ) == 0 ){
        free( contentLine );
        free( value       );
        free( property    );
        return INV_ALARM; //issue with action
      }

      strcpy(objAlarm->action, value);

    } else if ( myStrCmp( property, "TRIGGER" ) == 0 ){

      //too many triggers or malformed value
      if( ( ++numTrigger ) != 1 || value == NULL || strcmp( value,"" ) == 0 ){
        free( contentLine );
        free( value       );
        free( property    );
        return INV_ALARM; //issue with trigger
      }

      objAlarm->trigger = malloc(strlen(value)+1);

      if(objAlarm->trigger == NULL){
        free( contentLine );
        free( property    );
        free( value       );
        return OTHER_ERROR; //issue with allocation
      }

      strcpy(objAlarm->trigger, value);

    } else { //if not one of the required properties or if not a closing tag, general property
      /*Property* newProp = calloc(1, sizeof(Property)+ sizeof(char[999]));

      //failure to allocate memory
      if(newProp == NULL){
        free( contentLine );
        free( property    );
        free( value       );
        return OTHER_ERROR; //issue with allocation
      }

      //copy the values of the property
      strcpy( newProp->propName,  property );
      strcpy( newProp->propDescr, value );

      //add to the list of alarm properties
      insertBack( objAlarm->properties, newProp );*/
      if(strcmp(value,"")!=0){
        //create a new property
        Property* newProp = calloc(1, sizeof(Property)+ sizeof(char[999]));

        //failure to allocate memory
        if(newProp == NULL){
          free( contentLine );
          free( property    );
          free( value       );
          return OTHER_ERROR; //issue with allocation
        }

        //copy the values of the property
        strcpy( newProp->propName,  property );
        strcpy( newProp->propDescr, value );

        //add to the list of alarm properties
        insertBack( objAlarm->properties, newProp );
      } else {
        free( contentLine );
        free( value       );
        free( property    );
        return INV_ALARM; //issue with trigger
      }
    }

    //outside of if-elseif-else statement, free temp memory
    free( contentLine );
    free( property    );
    free( value       );

  } while ( exit == FALSE && !feof( *fp ) ); //loop again if not reached exit tag or not eof

  if( numTrigger!= 1){
    objAlarm->trigger = NULL;
    return INV_ALARM;
  }

  if(numAction != 1){
    return INV_ALARM;
  }

  //no closing tag
  if(exit==FALSE){
    return INV_ALARM; //missing closing tag
  }

  //printf("numAction = %d\n", numAction);
  //printf("numTrigger = %d\n", numTrigger);
  return OK;

}

//get dateTime information from the content line
ICalErrorCode createDate(char* value, DateTime* objDate){

  //return INV_DT due to date-time property present contents are malformed
  if( value == NULL || strcmp( value, "" ) == 0 ){
    return INV_DT; //contents malformed
  }

  //validate the value

  //not the expected length
  //16 if YYYYMMDDTHHMMSSZ (4Y, 2M, 2D, 1T, 2H, 2M, 2S, 1Z)
  //15 if YYYYMMDDTHHMMSS
  if( strlen( value ) != 16 && strlen( value ) != 15 ){
    return INV_DT;
  }

  //invalid string contents
  for( int i = 0 ; i < strlen(value) ; i++ ){
    if( i == 8 ){
      if( value[8] != 'T' ){ //in the specs, it states that YYYMMDD must be prior to capital T
        return INV_DT; //missing the T
      }
    } else if ( strlen(value) == 16 ){ //if this is the UTC one
      if( value[15] != 'z' && value[15] != 'Z' ){ //if its not the letter Z
        return INV_DT; //not the Z in UTC value
      }
    } else{
      if( !isdigit( value[i] ) ){ //if the rest isnt a digit, malformed
        return INV_DT; //isnt a number
      }
    }
  }

  //parse date information from content line
  char date[9];
  date[0] = '\0';
  for( int i = 0 ; i <= 7 ; i++ ){
    date[i] = value[i];
  }
  date[8] = '\0';
  strcpy(objDate->date, date);

  //get the time
  char time[7];
  time[0] = '\0';
  for( int i = 0 ; i <= 5 ; i++ ){
    time[i] = value[9+i];
  }
  time[6] = '\0';
  strcpy( objDate->time, time );

  //Z indicates UTC
  if( value[15] == 'Z' || value[15] == 'z' ){
    objDate->UTC = true;
  } else { //value [15] would be a null terminator
    objDate->UTC = false;
  }

  return OK;
}

//function reads file until reaching the word end
ICalErrorCode createEvent(FILE** fp, Event* objEvent){

  //initialize event lists
  objEvent->properties = initializeList( &printProperty, &deleteProperty, &compareProperties );
  objEvent->alarms     = initializeList( &printAlarm,    &deleteAlarm,    &compareAlarms );

  //failure to initialize list
  if( objEvent->properties == NULL || objEvent->alarms == NULL ){
    return OTHER_ERROR;
  }

  //get content line and break them into property and value
  char* contentLine = NULL;
  char* property    = NULL;
  char* value       = NULL;

  int   endEvent    = FALSE; //failure to find the ending tag is inv_event

  //if required properties exist more than once, this is an error
  int   numUID      = 0;
  int   numDTSTAMP  = 0;
  int   numDTSTART  = 0;

  //while we havent reached end vevent and it isnt the end of the file,
  //continue parsing the event
  //if we reach the end of the file without encountering end vevent first, error
  do {

    contentLine = getContentLine(fp);

    //invalid line endings
    if(contentLine == NULL){
      free(contentLine);
      return INV_FILE; //invalid line ending
    }

    property = getProperty( contentLine );
    if(strcmp(property,"")==0){
      free(contentLine);
      free(property);
      //return INV_CAL;
      return INV_EVENT;
    }
    value    = getValue   ( contentLine );

    //if the properties are specific key words add them in as those
    //else they just fall into the list of properties
    if (myStrCmp(property, "END") == 0){ //if ending tag

      if(myStrCmp(value, "VEVENT") == 0){
        endEvent = TRUE;
      } else {                //if there is an end tag but it isnt for ending event
        free(contentLine);    //the end valarm is handled in create alarm
        free(value);
        free(property);
        return INV_EVENT;  //missing end tag or invalid end tag
      }

    } else if(myStrCmp(property, "DTSTAMP") == 0){

      if((++numDTSTAMP)!=1){ //dupicate dtstamp, only allowed to occur once
        free(contentLine);
        free(value);
        free(property);
        return INV_EVENT;
      }
      //create date from the contentline value
      DateTime* newDate = malloc(sizeof(DateTime));

      if(newDate == NULL){
        return OTHER_ERROR; //failure to allocate memory
      }

      ICalErrorCode error = createDate(value, newDate);

      //error in creating date
      if(error!= OK){
        free(contentLine);
        free(property);
        free(value);
        deleteDate(newDate);
        return error;
      }

      objEvent->creationDateTime = *newDate;
      deleteDate(newDate);

    } else if (myStrCmp(property, "DTSTART") == 0){

      //only allowed to occur once
      if((++numDTSTART)!=1){
        free(contentLine);
        free(property);
        free(value);
        return INV_EVENT;
      }

      DateTime* newDate = malloc(sizeof(DateTime));

      if(newDate == NULL){
        return OTHER_ERROR;
      }

      ICalErrorCode error = createDate(value, newDate);

      //check if date parsing was ok
      if(error!=OK){
        free(contentLine);
        free(value);
        free(property);
        deleteDate(newDate);
        return error;
      }

      objEvent->startDateTime = *newDate;
      deleteDate( newDate );

    } else if ( myStrCmp( property, "UID" ) == 0 ){

      //uid should only appear once
      if( ( ++numUID ) != 1 ){
        free(contentLine);
        free(property);
        free(value);
        return INV_EVENT;
      }

      //uid is present but malformed (i.e. missing value)
      if( value == NULL || strcmp( value, "" ) == 0 ){
        free(contentLine);
        free(property);
        free(value);
        return INV_EVENT;
      }

      strcpy( objEvent->UID, value );
    } else if ( myStrCmp( contentLine, "BEGIN:VALARM" ) == 0 ){
      //printf("create alarm\n");
      //create a new alarm
      Alarm* newAlarm = malloc(sizeof(Alarm));
      ICalErrorCode error = createAlarm(fp, newAlarm);
      if(error!=OK){
        free(contentLine);
        free(property);
        free(value);
        deleteAlarm(newAlarm);
        return error;
      }
      //and add it to the back of the alarms list
      insertBack(objEvent->alarms, newAlarm);
    } else {
      //printf("add to property\n");
      /*Property* newProp = calloc(1, sizeof(Property)+ sizeof(char[999])+strlen(value));
      strcpy(newProp->propName, property);
      strcpy(newProp->propDescr, value);
      insertBack(objEvent->properties, newProp);*/
      //add it into the properties list

      if(strcmp(value, "") != 0){
        Property* newProp = calloc(1, sizeof(Property)+ sizeof(char[999])+strlen(value));
        strcpy(newProp->propName, property);
        strcpy(newProp->propDescr, value);
        insertBack(objEvent->properties, newProp);
      } else {
        free(contentLine);
        free(value);
        free(property);
        return INV_EVENT;
      }


    }
    //for some reason, there is an issue in freeing these???
    //free the things
    free(contentLine);
    free(value);
    free(property);

  } while(endEvent == FALSE && !feof(*fp));

  if(numUID != 1 || numDTSTAMP != 1 || numDTSTART != 1){
    //deleteEvent(objEvent);
    return INV_EVENT;
  }

  //missing closing tag
  if(endEvent == FALSE){
    return INV_EVENT;
  }

  return OK;
}

char* getProperty(char* contentLine){
  if(contentLine == NULL){
    return NULL;
  }
  //printf("Enter getProperty\n");
  int i = 0;
  int c = 0;

  //printf("%s\n", contentLine);

  //char keys[] = ":;";
  char* property = malloc((strlen(contentLine)+500)*sizeof(char));

  //i = strcspn(contentLine, keys);
  while(contentLine[i]!=';' && contentLine[i]!=':' && i < strlen(contentLine)){
    //printf("%c\n", contentLine[i]);
    i++;
  }

  //property = realloc(property, i+1);
  property[0] = '\0';

  while(c < i){
    property[c] = contentLine[c];
    property[++c] = '\0';
  }

  //printf("getProperty:\t%s\n", property);
  return property;
}

char* getValue(char* str){
  //printf("Enter getValue\n");

  if (str == NULL){
    return NULL;
  }

  int i = 0;
  int c = 0;

  char contentLine[strlen(str)+100];
  strcpy(contentLine, str);

  //char keys[] = ":";
  char* value = malloc(sizeof(char)*(strlen(contentLine) + 100));
  //printf("contentline = %s\n", contentLine);
  //i = strcspn(contentLine, keys);

  int found = 0;
  while(contentLine[i]!=';' && contentLine[i]!=':' && i < strlen(str)){
    //printf("%c\n", value[i]);
    i++;

  }

  //printf("%c\n", contentLine[i]);
  if(contentLine[i] == ':' || contentLine[i] == ';') found = 1;
  if(found == 0) {
    strcpy(value,"");
    return value;
  }

  //printf("%c\n", value[i]);
  while(contentLine[i]!='\0'){
    value[c] = contentLine[++i];
    value[++c] = '\0';
  }



  //printf("getValue:\t%s\n", value);
  return value;
}

//handling of comments should be done in the calls
char* getContentLine(FILE **fp){
  //printf("Enter getContentLine\n");

  int index = 0;
  char c;
  char* contentLine = malloc(sizeof(char)*300);

  contentLine[0] = '\0';
  strcpy(contentLine,"");

  c=fgetc(*fp);

  while(c !='\r' && c != '\n' && !feof(*fp)){
    contentLine[index] = c;
    contentLine[++index] = '\0';
    c = fgetc(*fp);
  }

  int crlf = 0;
  //when we exit the while loop, either we have reached \r or \n
  if(c == '\r'){ //if it is /r check if it is follwed by a \n
    if((c=fgetc(*fp))!= '\n'){
      free(contentLine);
      return NULL;
    } else {
      crlf = 1;
    }
  } else if (c == '\n'){ //if it is \n, it is immediately an error
    free(contentLine);
    return NULL;
  }/* else if (feof(*fp)){
    free(contentLine);
    return NULL;
  }*/
  else if (crlf == 0){
    free(contentLine);
    return NULL;
  }

  if (strcmp(contentLine, "") == 0 || contentLine == NULL){
    //free(contentLine);
    //return NULL;
    return contentLine;
  }

  //while the next line is a comment, just move the fp to the next
  while((c = fgetc(*fp)) == ';'){
    while((c = fgetc(*fp))!='\n'){
      //just do nothing
    }
  }
  ungetc(c, *fp);

  c = fgetc(*fp);
  while (c=='\t' || c==' '){
    ICalErrorCode error = unfold(&contentLine, fp);

    //if there was an issue in the unfolding due to improper line ending
    //NEED TO DOUBLE CHECK THIS
    if (error != OK){
      free(contentLine);
      return NULL;
    }
    //printf("UNFOLDED\n");
    //printf("%s\n", contentLine);

    /*if((c=fgetc(*fp))!= '\n'){
      free(contentLine);
      return NULL;
    }*/
    c = fgetc(*fp);
  }
  ungetc(c, *fp);//can be either space or not, i will need to move the
                   //file pointer back one to fix the logic
  return contentLine;
}

/*
  cl1: the content line that the second part will be appended to
*/
ICalErrorCode unfold( char** cl1, FILE **fp ){

    int newLen;
    char* cl2;
    cl2 = getContentLine(fp);

    //next line has error in it
    if(cl2 == NULL){
      return INV_FILE;
    }

    newLen = strlen(*cl1) + strlen(cl2) + 500;
    *cl1 = realloc(*cl1, newLen);
    strcat(*cl1,cl2);

    free(cl2);
    return OK;
}

char* writeProperty(void* toBePrinted){
  char* tmpStr;
  Property* tmpProperty;
  int len;

  if(toBePrinted == NULL){
    return NULL;
  }

  tmpProperty = (Property*)toBePrinted;

  //print: <PropertyName>:<PropertyDescription>
  /*
  Length of the string is:
  Length of the propertyName + PropertyDescription + 2 for '\0' and ':'
  */
  len = strlen(tmpProperty->propName) + strlen(tmpProperty->propDescr) + 10;
  tmpStr = malloc(sizeof(char)*len);

  sprintf(tmpStr, "%s:%s\r\n", tmpProperty->propName, tmpProperty->propDescr);

  return tmpStr;
}

char* writeAlarm(void* toBePrinted){
  if (toBePrinted == NULL){
    return NULL;
  }

  Alarm* tmpAlarm = (Alarm*) toBePrinted;

  int currentLength = 15;
  char* str = malloc(currentLength);

  char action[210];

  int triggerLength = strlen(tmpAlarm->trigger) + 10;
  char trigger[triggerLength];

  char* properties = writeList(tmpAlarm->properties, &writeProperty); //NEED TO CONVER THIS TO WRITE
  properties = realloc(properties, strlen(properties)+2);
  properties[strlen(properties)] = '\0';

  sprintf(action, "ACTION:%s\r\n", tmpAlarm->action);
  sprintf(trigger, "TRIGGER:%s\r\n", tmpAlarm->trigger);

  str[0] = '\0';
  strcpy(str, "BEGIN:VALARM\r\n");
  str[strlen(str)] = '\0';

  myStrCat(&str, action);
  str[strlen(str)] = '\0';

  myStrCat(&str, trigger);
  str[strlen(str)] = '\0';

  myStrCat(&str, properties);
  str[strlen(str)] = '\0';

  myStrCat(&str, "END:VALARM\r\n");
  str[strlen(str)] = '\0';

  //printf("%s", str);

  free(properties);

  return str;
}

char* writeDate(void* toBePrinted){
  if(toBePrinted==NULL){
    return NULL;
  }

  DateTime* tmpDate = (DateTime*) toBePrinted;
  char* str = malloc(20);

  str[0] = '\0';

  sprintf(str, "%sT%s", tmpDate->date, tmpDate->time);
  if(tmpDate->UTC == true){
    strcat(str, "Z");
  }

  myStrCat(&str, "\r\n");

  str[strlen(str)] = '\0';

  return str;
}

char* writeEvent(void* toBePrinted){
  Event* tmpEvent;

  if (toBePrinted == NULL){
    return NULL;
  }

  tmpEvent = (Event*)toBePrinted;

  int len = strlen("BEGIN:VEVENT\r\n") + 10;
  char* str = malloc(sizeof(char)* len);

  char* strDate1 = writeDate(&(tmpEvent->creationDateTime));
  char* strDate2 = writeDate(&(tmpEvent->startDateTime));

  char* properties = writeList(tmpEvent->properties, &writeProperty);
  char* alarms = writeList(tmpEvent->alarms, &writeAlarm);

  char uid[999];

  sprintf(uid, "UID:%s\r\n", tmpEvent->UID);

  str[0] = '\0';

  strcpy(str,"BEGIN:VEVENT\r\n");
  myStrCat(&str, uid);
  myStrCat(&str, "DTSTAMP:");
  myStrCat(&str, strDate1); //write data appends /r/n to end
  myStrCat(&str, "DTSTART:");
  myStrCat(&str, strDate2);
  myStrCat(&str, properties);
  myStrCat(&str, alarms);
  myStrCat(&str, "END:VEVENT\r\n");
  str[strlen(str)] = '\0';


  free(strDate1);
  free(strDate2);
  free(properties);
  free(alarms);

  //printf("printEvent returns:\n%s", str);
  return str;
  //need to malloc for uid, createDateTime, startDateTime, properties and alarms

  //properties and alarms are strings so we can just use toString from the LinkedListAPI
}

char* writeList(List * list, char* (*writeFunction)(void* toBePrinted)){
	ListIterator iter = createIterator(list);
	char* str;

	str = (char*)malloc(sizeof(char));
	strcpy(str, "");

	void* elem;
	while((elem = nextElement(&iter)) != NULL){
		char* currDescr = writeFunction(elem);
		int newLen = strlen(str)+50+strlen(currDescr);
		str = (char*)realloc(str, newLen);
		//strcat(str, "\n");
		strcat(str, currDescr);

		free(currDescr);
	}

	return str;
}

/** Function to writing a Calendar object into a file in iCalendar format.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a file representing the
        Calendar contents in iCalendar format has been created
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param obj - a pointer to a Calendar struct
 **/
ICalErrorCode writeCalendar(char* fileName, const Calendar* obj){
  if (obj == NULL){
    return WRITE_ERROR;
  }

  FILE* fp = fopen(fileName, "w");
  if (fp == NULL){
    return WRITE_ERROR;
  }

  char* str = calloc(10000, sizeof(char));
  char version[15]; //8 char for "VERSION:" and 3 for value and 2 for \r\n
  char prodID[1000];
  char* properties = writeList(obj->properties, &writeProperty);
  char* events = writeList(obj->events, &writeEvent);

  myStrCat(&str, "BEGIN:VCALENDAR\r\n");

  //printf("obj->version = %.1f\n", obj->version);
  sprintf(version, "VERSION:%.1f\r\n", obj->version);
  myStrCat(&str, version);
  sprintf(prodID, "PRODID:%s\r\n", obj->prodID);
  myStrCat(&str, prodID);

  properties = realloc(properties, strlen(properties)+1);
  properties[strlen(properties)] = '\0';

  myStrCat(&str, properties);
  str[strlen(str)] = '\0';

  events = realloc(events, strlen(events)+1);
  events[strlen(events)] = '\0';

  myStrCat(&str, events);
  myStrCat(&str, "END:VCALENDAR\r\n");
  str[strlen(str)] = '\0';

  free(properties);
  free(events);

  fprintf(fp, "%s", str);
  free(str);
  fclose(fp);
  return OK;
}

int numOccurencesValid(List* list, char* property, int numOccurenceAllowed){
  ListIterator iter = createIterator(list);

  int numOccurence = 0;

  Property* elem;
  while((elem = nextElement(&iter))!=NULL){
    if (myStrCmp(elem->propName, property) == 0){
      numOccurence++;
    }
  }

  //numOccurenceAllowed == 0 indicates that the property is allowed to occur more than a specified number
  if (numOccurenceAllowed == 0){
    if (numOccurence > numOccurenceAllowed){ //ie numOccurence > 0
      return TRUE;
    }
  }

  if (numOccurenceAllowed == numOccurence){
    return TRUE;
  }

  return FALSE;
}

int isValidCalProp(Property* prop){
  if (myStrCmp(prop->propName, "CALSCALE") == 0 || myStrCmp(prop->propName, "METHOD") == 0){
    //printf("sizeof = %lu\n", strlen(prop->propDescr));
    if(strcmp(prop->propDescr, "") == 0 || strlen(prop->propDescr) == 0){
      return FALSE; //property with an empty value
    }
    return TRUE;
  }
  return FALSE; //property that should not occur in calendar
}

void validateCalProperties(List* properties, ICalErrorCode* err){
  //empty list, dont need to do any verifications
  if (getLength(properties) == 0){
    *err = OK;
    return;
  }

  ListIterator iter = createIterator(properties);

  Property* prop;
  while ((prop = nextElement(&iter))!=NULL){

    //check if it is a valid calendar property
    if (isValidCalProp(prop)==FALSE){
      *err = INV_CAL;
      return;
    }

    //check if the number of occurences in the property follow the rules
    //the calendar properties must only occur once
    if(numOccurencesValid(properties, prop->propName, 1) == FALSE){
      *err = INV_CAL;
      return;
    }
  }

  *err = OK;
  return;
}

/** Function to validating an existing a Calendar object
 *@pre Calendar object exists and is not NULL
 *@post Calendar has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the calendar
 *@param obj - a pointer to a Calendar struct
 **/
ICalErrorCode validateCalendar(const Calendar* obj){
  int testing = FALSE;

  if (testing) printf("Function ValidateCalendar: Validating Calendar\n");

  if (obj == NULL){
    if (testing) printf("Function ValidateCalendar: obj param is NULL\n");
    return INV_CAL; //if the argument is null
  }

  //check if any of the lists are NULL
  if(obj->properties == NULL){
    if (testing) printf("Function ValidateCalendar: calendar properties list is NULL\n");
    return INV_CAL;
  }

  if(obj->events == NULL){
    if (testing) printf("Function ValidateCalendar: calendar events list is NULL\n");
    return INV_CAL;
  }

  //validate required
  if(getLength(obj->events) < 1){
    if (testing) printf("Function ValidateCalendar: empty events list\n");
    return INV_CAL; //empty events list
  }

  if(obj->version <= 0){
    if (testing) printf("Function ValidateCalendar: missing version\n");
    return INV_CAL; //missing version
  }

  if(strcmp(obj->prodID, "") == 0){
    if (testing) printf("Function ValidateCalendar: missing prodID\n");
    return INV_CAL; //missing prodID
  }


  //validate optional propeerties
  ICalErrorCode err = OK;
  validateCalProperties(obj->properties, &err);
  if (err != OK){
    if (testing) printf("Function ValidateCalendar: Invalid optional calendar properties\n");
    return err;
  }

  //validate events
  ListIterator events = createIterator(obj->events);

  Event* evt;
  while((evt = nextElement(&events))!=NULL){
    validateEvent(evt, &err);
    if (err!=OK){
      if (testing) printf("Function ValidateCalendar: Invalid event\n");
      return err;
    }
  }

  return OK;
}

//TO DO: VERIFY THAT THE VALID ARE WHAT THEY ARE SUPPOSED TO BE
int isValidEvtProp(Property* prop){
  char* valid[] = {"CLASS", "CREATED", "DESCRIPTION", "GEO", "LAST-MODIFIED", "LOCATION",
                   "ORGANIZER", "PRIORITY", "SEQUENCE", "STATUS", "SUMMARY", "TRANSP",
                   "URL", "RECURRENCE-ID"};
  int NUM_VALID = 14;

  //if property with empty value
  if (strcmp(prop->propDescr, "") == 0 || strlen(prop->propDescr) == 0){
    return INV_EVENT;
  }

  //search through the valid properties
  for (int i = 0; i < NUM_VALID; i++){
    if (myStrCmp(valid[i], prop->propName) == 0){
      return TRUE;
    }
  }

  return FALSE;
}

//TO DO: VERIFY THAT THE VALID ARE WHAT THEY ARE SUPPOSED TO BE
int isValidEvtPropPlus(Property* prop){
  char* valid[] = {"ATTACH", "ATTENDEE", "CATEGORIES", "COMMENT", "CONTACT",
                   "EXDATE", "RELATED-TO", "RESOURCES", "RDATE"
                   ,"RRULE"};
  int NUM_VALID = 10;

  //check if property is null
  if (strcmp(prop->propDescr, "") == 0 || strlen(prop->propDescr) == 0){
    return INV_EVENT;
  }

  for (int i = 0; i < NUM_VALID; i++){
    if (myStrCmp(valid[i], prop->propName) == 0){
      return TRUE;
    }
  }

  return FALSE;
}

void validateEvtProperties(List* properties, ICalErrorCode* err){
  int testing = FALSE;
  //printf("%d\n", getLength(properties));
  if (getLength(properties) == 0) {
    if (testing) printf("Function validateEvtProperties: event properties is empty\n");
    *err = OK;
    return;
  }

  //either may appear
  //dtend and duration must not occur in the same eventprop
  //search through the list and see if both appear
  int numDTEND = 0;
  int numDUR   = 0;
  ListIterator search = createIterator(properties);

  Property* prop;
  while ((prop = nextElement(&search))!=NULL){
    if (strcmp(prop->propDescr, "") == 0){
      if (testing) printf("Function validateEvtProperties: empty prop description\n");
      *err = INV_EVENT;
      return;
    }
    if(myStrCmp(prop->propName, "DTEND") == 0){
      numDTEND++;
      if(numDTEND > 1){
        if (testing) printf("Function validateEvtProperties: DTEND occurs more than once (%d)\n", numDTEND);
        *err = INV_EVENT;
        return;
      }
    }

    if(myStrCmp(prop->propName, "DURATION") == 0){
      numDUR++;
      if(numDUR > 1){
        if (testing) printf("Function validateEvtProperties: DURATION occurs more than once (%d)\n", numDUR);
        *err = INV_EVENT;
        return;
      }
    }

  }

  if (numDTEND >= 1 && numDUR >= 1) {
    if (testing) printf("Function validateEvtProperties: DTEND and DURATION occur together in the same event\n");
    *err = INV_EVENT;
    return;
  }

  ListIterator iter = createIterator(properties);

  while ((prop = nextElement(&iter))!=NULL){
    //skip if it is dtend or duration because we already checked that
    if (myStrCmp(prop->propName, "DTEND")!=0 && myStrCmp(prop->propName, "DURATION")!=0){
      //missing optional that can only appear once || missing optional that can appear more than once
      if (isValidEvtProp(prop)==FALSE && isValidEvtPropPlus(prop)==FALSE){ //if neither
        if (testing) printf("Function validateEvtProperties: Invalid property (%s)\n", prop->propName);
        *err = INV_EVENT;
        return;
      }
      //check if the number of occurences in the property follow the rules
      if(isValidEvtProp(prop) == TRUE && numOccurencesValid(properties, prop->propName, 1) == FALSE){
        if (testing) printf("Function validateEvtProperties: Invalid property (%s)\n", prop->propName);
        *err = INV_EVENT;
        return;
      }
      //check if the number of occurences in the property follow the rules
      if(isValidEvtPropPlus(prop)==TRUE && numOccurencesValid(properties, prop->propName, 0) == FALSE){
        if (testing) printf("Function validateEvtProperties: Invalid number of occurence (%s)\n", prop->propName);
        *err = INV_EVENT;
        return;
      }
    }
  }

  *err = OK;
  return;
}

void validateDate(DateTime* dt, ICalErrorCode* err){
  //printf("%s %s\n",dt->date, dt->time);
  int testing = FALSE;
  if (testing) printf("strlen(%s) = %lu\n", dt->date, strlen(dt->date));
  if (testing) printf("strlen(%s) = %lu\n", dt->time, strlen(dt->time));
  if (strlen(dt->date)!=8) {
    if (testing) printf("validateDate: invalid dt->date\n");
    *err = INV_EVENT;
    return;
  }

  if (strlen(dt->time)!=6){
    if (testing) printf("validateDate: invald dt->time\n");
    *err = INV_EVENT;
    return;
  }

  if (testing) printf("validateDate: valid\n");
  *err = OK;
  return;
}

void validateEvent(const Event* obj, ICalErrorCode* err){
  int testing = FALSE;

  if (testing) printf("Function validateEvent: validating event\n");

  //check if any of the lists are null
  if(obj->properties == NULL){
    if (testing) printf("Function validateEvent: event properties list NULL\n");
    *err = INV_EVENT;
    return;
  }
  if(obj->alarms == NULL){
    if (testing) printf("Function validateEvent: event alarms list NULL\n");
    *err = INV_EVENT;
    return;
  }
  //missing required
  if(&(obj->UID) == NULL || strlen(obj->UID) == 0 || strcmp(obj->UID,"")==0){
    if (testing) printf("Function validateEvent: missing UID\n");
    *err = INV_EVENT;
    return;
  }
  if(&(obj->creationDateTime) == NULL || sizeof(obj->creationDateTime) == 0) {
    if (testing) printf("Function validateEvent: missing creationDateTime\n");
    *err = INV_EVENT;
    return;
  }
  if(&(obj->startDateTime) == NULL || sizeof(obj->startDateTime) == 0){
    if (testing) printf("Function validateEvent: missing startDateTime\n");
    *err = INV_EVENT;
    return;
  }

  //check the date time objects and make sure they are ok
  DateTime* temp = (DateTime*)(&(obj->creationDateTime));

  //validateDate(obj->creationDateTime, err);
  validateDate(temp, err);
  if (*err != OK){
    *err = INV_EVENT;
    if (testing) printf("Function Validate Event: invalid date\n");
    return;
  }

  temp = (DateTime*)(&(obj->startDateTime));
  validateDate(temp, err);
  if (*err != OK){
    if (testing) printf("Function Validate Event: invalid date\n");
    *err = INV_EVENT;
    return;
  }

  //validate optional
  validateEvtProperties(obj->properties, err);
  if (*err != OK){
    if (testing) printf("Function validateEvent: invalid optional event property\n");
    *err = INV_EVENT;
    return;
  }

  //validate the alarms
  ListIterator alarms = createIterator(obj->alarms);

  Alarm* alrm;
  while((alrm = nextElement(&alarms))!=NULL){
    //char* alrmStr = printAlarm(alrm);
    //printf("%s\n", alrmStr);
    validateAlarm(alrm, err);
    //printf("ICalErrorCode err = ");
    //printError(*err);
    //printf("\n");
    if(*err!=OK) {
      if (testing) printf("Invalid alarm\n");
      return;
    }
  }

  if (testing) printf("Its good\n");
  *err = OK;
  return;
}

void validateAlarmProperties(List* properties, ICalErrorCode* err){
  int testing = FALSE;

  //if (testing) printf("Function validateAlarmProperties\n");

  if (getLength(properties) == 0){
    if (testing) printf("Function validateAlarmProperties: list of alarm properties is empty\n");
    *err = OK;
    return;
  }

  /* duration and repeat are both optional and must no occur more than once
     but if one occurs, so does the other*/
  int numREPEAT = 0;
  int numDUR = 0;
  int numATTACH = 0;

  ListIterator search = createIterator(properties);

  Property* prop;

  //Validate alarm properties
  //iterate and make sure they are all attach, repeat, or duration
  while ((prop = nextElement(&search))!=NULL){
    if (testing) printf("Function validateAlarmProperties: check property %s\n", prop->propName);
    //empty propDescr
    if (strcmp(prop->propDescr, "") == 0) {
      if (testing) printf("Function validateAlarmProperties, property is empty\n");
      *err = INV_ALARM;
      return;
    }

    if(myStrCmp(prop->propName, "REPEAT") == 0){
      numREPEAT++;
      if(numREPEAT != 1){
        if (testing) printf("Function validateAlarmProperties: REPEAT occurs more than once (%d)\n", numREPEAT);
        *err = INV_ALARM;
        return;
      }
    } else if(myStrCmp(prop->propName, "DURATION") == 0){
      numDUR++;
      if(numDUR != 1){
        if (testing) printf("Function validateAlarmProperties: DUR occurs more than once (%d)\n", numDUR);
        *err = INV_ALARM;
        return;
      }
    } else /*if(myStrCmp(prop->propName, "REPEAT") != 0 && myStrCmp(prop->propName, "DURATION") != 0)*/{
      if (testing) printf("Function validateAlarmProperties: the remaining properties must be attach\n");
      if(myStrCmp(prop->propName, "ATTACH") ==0){
        numATTACH++;
        if(numATTACH > 1){
          if (testing) printf("Function validateAlarmProperties: empty property (%s)\n", prop->propName);
          *err = INV_ALARM;
          return;
        }
      } else {
        if (testing) printf("Function validateAlarmProperties: invalid alarm property (%s)\n", prop->propName);
        *err = INV_ALARM;
        return;
      }
    }
  }


  if ((numREPEAT == 1 && numDUR != 1) || (numREPEAT != 1 && numDUR == 1) ){
    if (testing) printf("Function validateAlarmProperties: REPEAT and DURATION don't occur simultaneously\n");
    *err = INV_ALARM;
    return;
  }

  *err = OK;
  return;
}

void validateAlarm(Alarm* obj, ICalErrorCode* err){
  int testing = FALSE;

  if (testing) printf("Function validateAlarm: Validating alarm\n");
  //missing action
  if (strcmp(obj->action, "") == 0 || strlen(obj->action) == 0){
    if (testing) printf("Function validateAlarm: Missing action\n");
    *err = INV_ALARM;
    return;
  }

  //missing trigger
  if (obj->trigger == NULL || strcmp(obj->trigger, "") == 0 || strlen(obj->trigger) == 0){
    if (testing) printf("Function validateAlarm: Missing trigger\n");
    *err = INV_ALARM;
    return;
  }

  if (testing) printf("Function validateAlarm: validate alarm properties\n");
  //validate optional properties
  validateAlarmProperties(obj->properties, err);

  if(*err!=OK) return;

  *err = OK;
  return;
}

/***************************** REQUIRED FUNCTIONS *****************************/

/** Function to create a Calendar object based on the contents of an iCalendar file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid calendar has been created, its address was stored in the variable obj, and OK was returned
		or
		An error occurred, the calendar was not created, all temporary memory was freed, obj was set to NULL, and the
		appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param fileName - a string containing the name of the iCalendar file
 *@param a double pointer to a Calendar struct that needs to be allocated
**/
ICalErrorCode createCalendar(char* fileName, Calendar** obj){
  int testing = FALSE;

  /***********the following tests to see if the fileName is valid**************/

  //if the fileName is NULL
  if(fileName == NULL){
    if(testing) printf("fileName is null\n");
    (*obj) = NULL;
    return INV_FILE;
  }

  //if an empty string
  if(strcmp(fileName,"") == 0){
    if(testing) printf("fileName is empty\n");
    (*obj) = NULL;
    return INV_FILE;
  }

  //check if extension line is valid (should have the .ics extension)
  char* extension = strrchr(fileName, '.');

  //if the file has no dot in it or is a .ics
  if (extension == NULL || strcmp(extension+1,"ics") != 0){
    if(testing) printf("fileName doesn't have the .ics extension\n");
    (*obj) = NULL;
    //free(extension); //not necessary
    return INV_FILE;
  }

  //if the file doesnt exist or cannot be opened
  FILE *fp = fopen(fileName, "r");
  if(fp == NULL){
    //fclose(fp); //not necessary
    if (testing) printf("file doesn't exist or cannot be opened\n");
    *obj = NULL;
    return INV_FILE;
  }

  /*************************** initialize variables ***************************/

  /* these variables are used for reading of the content lines in the file **/
  char* contentLine = NULL; //the whole line until the next property
                              //will remain NULL if invalid file
  char* property    = NULL; //everything before the colon or semi-colon
  char* value       = NULL; //everthing after the semi colon or colon
                              //including linefolded content

  /* used to determine if there are duplicates, or missing required tags ****/

  int numVersions   = 0; //the calendar object must contain properties version
  int numProdID     = 0;    //and numProdID, but they must not occur
                            //more than once, if the calendar is valid,
                            //these will be equal to one
  int numEvents     = 0; //there must exist at least one event
  int numEnd        = 0; //there must be a closing calendar tag (only one)

  /* used to determine when to exit the loops *******************************/

  int toExit        = FALSE; //indicates that we have found the end tag
  int beginCalFound = FALSE; //indicates if we have found the begin tag

  /********************** initialize the calendar object **********************/

  //allocate memory for the Calendar object
  //if((*obj)!=NULL)(*obj) = realloc((*obj),sizeof(Calendar));
  (*obj) = calloc(1, sizeof(Calendar));

  //check if malloc of calendar was successful
  if((*obj) == NULL){
    if(testing) printf("malloc for cal obj failed\n");
    fclose(fp);
    return OTHER_ERROR;
  }

  //initialize the lists for properties and events
  (*obj)->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
  (*obj)->events     = initializeList(&printEvent,    &deleteEvent,    &compareEvents);

  //check if initialization of lists was ok
  if((*obj)->properties == NULL || (*obj)->events == NULL){
    if(testing) printf("failure to initialize list\n");
    return OTHER_ERROR;
  }

  /** we have gotten past the initialization of everything, onto parsing now **/

  //search through the file until we reach content line "BEGIN:VCALENDAR"
  //or we have reached end of file
  do {
    contentLine = getContentLine(&fp); //get the content line
    if(contentLine == NULL){
      //free(contentLine);
      deleteCalendar((*obj));
      fclose(fp);
      return INV_FILE;
    }
    //if the content line is not begin cal, free the content line and keep searhcing
    if(myStrCmp(contentLine, "BEGIN:VCALENDAR")!=0){
      free(contentLine);
    } else { //if we have found it, we can exit the loop
      beginCalFound = TRUE;
      free(contentLine);
    }
  } while (beginCalFound == FALSE && !feof(fp));

  //if we have searched through the entire file and there is no begin vcalendar,
  //return error
  //ie. invald opening calendar tags
  if (feof(fp)){
    if (testing) printf("missing opening tag\n");
    deleteCalendar((*obj));
    fclose(fp);
    return INV_CAL;
  }

  //after finding the opening tag, parse the lines until we reach the closing tag
  //or it is the end of the file
  do {
    contentLine = getContentLine(&fp);

    //invalid line endings
    if(contentLine == NULL){
      free(contentLine);
      deleteCalendar((*obj));
      //fclose(fp);
      fseek(fp, -2, SEEK_CUR);

      char c = fgetc(fp);
      if (c == '\r'){
        c = fgetc(fp);
        if (c == '\n'){
          c = fgetc(fp);
          if (c == EOF && toExit == FALSE) {
            fclose(fp);
            return INV_CAL;
          }
        }
      }

      fclose(fp);
      return INV_FILE;
    }
    if (strcmp(contentLine, "") != 0){

      value       = getValue(contentLine);
      property    = getProperty(contentLine);

      if (myStrCmp(property, "END") == 0){
        //printf("case: end\n");
        if(myStrCmp(value,"VCALENDAR")!=0){
          if(testing) printf("Invalid closing tags\n");
          free(contentLine);
          free(property);
          free(value);
          deleteCalendar((*obj));
          //freeList((*obj)->events);
          fclose(fp);
          return INV_CAL;
        } else {
          toExit = TRUE;
          if((++numEnd) != 1){
            if(testing)printf("Invalid closing tag: appears more than once\n");
            free(contentLine);
            free(property);
            free(value);
            deleteCalendar((*obj));
            fclose(fp);
            return INV_CAL;
          }
        }
      } else if(myStrCmp(property, "BEGIN") == 0){
        //printf("case: begin\n");
        if(myStrCmp(value, "VEVENT") == 0){
          numEvents++;
          //printf("create an event\n");
          Event* newEvent = malloc(sizeof(Event));

          if(newEvent == NULL){
            if(testing) printf("malloc returns NULL\n");
            free(contentLine);
            free(property);
            free(value);
            deleteCalendar((*obj));
            fclose(fp);
            return OTHER_ERROR;
          }

          ICalErrorCode error = createEvent(&fp, newEvent);
          if (error != OK){
            //free(newEvent);
            free(contentLine);
            free(property);
            free(value);
            deleteEvent(newEvent);
            deleteCalendar((*obj));
            fclose(fp);
            return error;
          }

          //create event and append it to the end of the list
          insertBack((*obj)->events, newEvent);
        }else{
          //the file should not begin anything else but begin event and begin alarm
          //begin alarm is dealt with in create event
          if(testing) printf("invalid opening tag\n");
          free(contentLine);
          free(property);
          free(value);
          deleteCalendar((*obj));
          fclose(fp);
          return INV_CAL;
        }
      } else if(myStrCmp(property, "VERSION") == 0){ //if VERSION, set the calendar version to the float
          //printf("case: version\n");
          if((++numVersions)!=1){
            if(testing) printf("calendar version property is present, appears more than once\n");
            free(contentLine);
            free(property);
            free(value);
            deleteCalendar((*obj));
            fclose(fp);
            return DUP_VER;
          }

          if(value == NULL || strcmp(value,"")==0){
            if(testing) printf("version value is missing\n");
            free(contentLine);
            free(property);
            free(value);
            deleteCalendar((*obj));
            fclose(fp);
            return INV_VER;
          }

          /********* used to check if the value for version is malformed ********/
          char d = '\0';
          int numDecimal = 0;
          int malformed = FALSE;

          for (int i = 0; i < strlen(value) && malformed == FALSE; i++){
            d = value[i];
            if(d == '.'){
              if((++numDecimal)!=1){
                malformed = TRUE;
              }
            } else if (!isdigit(d)){
              malformed = TRUE;
            }
          }

          if(malformed == TRUE){
            if(testing) printf("calendar version property present but malformed\n");
            free(contentLine);
            free(property);
            free(value);
            deleteCalendar((*obj));
            fclose(fp);
            return INV_VER;
          }

          //if it is ok
          (*obj)->version = atof(value); //assign the version to the calendar


      } else if(myStrCmp(property, "PRODID") == 0){
        //printf("case: prodid\n");
        if ((++numProdID)!=1){ //if it appears more than once
          if(testing) printf("the prod id property appears more than once\n");
          free(contentLine);
          free(property);
          free(value);
          deleteCalendar((*obj));
          fclose(fp);
          return DUP_PRODID;
        }

        if(value == NULL || strcmp(value, "")==0){ //malformed
          if(testing) printf("the prod id is malformed/value missing\n");
          free(contentLine);
          free(property);
          free(value);
          deleteCalendar((*obj));
          fclose(fp);
          return INV_PRODID;
        }

        strcpy((*obj)->prodID, value);

      } else {
        //printf("case: property\n");

        //create a new property object to be added to the list
        //extra memory to account for the flexible array

        if(strcmp(value,"")!=0){
          Property* newProp = malloc(sizeof(Property) + (sizeof(char) * (strlen(value))+10));

          strcpy(newProp->propName, property);
          strcpy(newProp->propDescr, value);

          //insert the new property object to the end of the list
          insertBack((*obj)->properties, newProp);
        } else {

          free(contentLine);
          free(property);
          free(value);
          deleteCalendar((*obj));
          fclose(fp);
          return INV_CAL;
        }
        /*if (strcmp(property, "")!= 0){
          Property* newProp = malloc(sizeof(Property) + (sizeof(char) * (strlen(value))+10));

          strcpy(newProp->propName, property);
          strcpy(newProp->propDescr, value);

          //insert the new property object to the end of the list
          insertBack((*obj)->properties, newProp);
        }*/

      }

      free(value);
      free(property);

    }

    free(contentLine);
  } while(toExit == FALSE && !feof(fp));

  if(numEvents < 1){
    if(testing) printf("missing an event\n");
    deleteCalendar((*obj));
    fclose(fp);
    return INV_CAL;
  }

  if(numProdID < 1){
    //calendar missing prod id tag
    if(testing) printf("missing prodid\n");
    deleteCalendar((*obj));
    fclose(fp);
    return INV_CAL;
  }

  if(numVersions < 1){
    if(testing) printf("missing version\n");
    deleteCalendar((*obj));
    fclose(fp);
    return INV_CAL;
  }

  if(toExit == FALSE){
    deleteCalendar((*obj));
    fclose(fp);
    return INV_CAL;
  }
  fclose(fp);
  return OK;
}

/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar* obj){
  if(obj== NULL){
    return;
  }
  freeList(obj->events);
  freeList(obj->properties);
  free(obj);
}

/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calndar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char* printCalendar(const Calendar* obj){

  if(obj == NULL){
    return NULL;
  }

  //printf("Enter printCalendar\n");
  //char* toString = (char*) malloc(sizeof(char)*1024);
  char* str = calloc(20000, sizeof(char));
  char version[100]; //8 char for "VERSION:" and 3 for value and 1 for null character
  char prodID[5000];
  char* properties = toString(obj->properties);
  char* events = toString(obj->events);

  //printf("EVENTS: \n%s\n", events);

  sprintf(version, "VERSION:%.1f\n", obj->version);
  sprintf(prodID, "PRODID:%s", obj->prodID);

  myStrCat(&str, "BEGIN:VCALENDAR\n");
  myStrCat(&str, version);
  myStrCat(&str, prodID);

  properties = realloc(properties, strlen(properties)+1);
  properties[strlen(properties)] = '\0';

  myStrCat(&str, properties);
  //myStrCat(&str, "\n");
  str[strlen(str)] = '\0';

  events = realloc(events, strlen(events)+1);
  events[strlen(events)] = '\0';

  myStrCat(&str, events);
  myStrCat(&str, "END:VCALENDAR\n");
  //myStrCat(&str, "\n");
  str[strlen(str)] = '\0';

  //printf("%s\n", str);

  free(properties);
  free(events);

  return str;
}

void deleteProperty(void* toBeDeleted){
    Property* tmpProperty;

    if (toBeDeleted == NULL){
      return;
    }

    tmpProperty = (Property*)toBeDeleted;

    free(tmpProperty); //since property features a flexible array, doesnt need to be freed
}

int compareProperties(const void* first, const void* second){
  return 0;
}

char* printProperty(void* toBePrinted){
  char* tmpStr;
  Property* tmpProperty;
  int len;

  if(toBePrinted == NULL){
    return NULL;
  }

  tmpProperty = (Property*)toBePrinted;

  //print: <PropertyName>:<PropertyDescription>
  /*
  Length of the string is:
  Length of the propertyName + PropertyDescription + 2 for '\0' and ':'
  */
  len = strlen(tmpProperty->propName) + strlen(tmpProperty->propDescr) + 10;
  tmpStr = malloc(sizeof(char)*len);

  sprintf(tmpStr, "%s:%s", tmpProperty->propName, tmpProperty->propDescr);

  return tmpStr;
}

void deleteEvent(void* toBeDeleted){
  if (toBeDeleted == NULL){
    return;
  }

  Event* tmpEvent;

  tmpEvent = (Event*)toBeDeleted;

  //deleteDate(&(tmpEvent->creationDateTime));
  //deleteDate(&(tmpEvent->startDateTime));
  //printf("propLength: %d\n", tmpEvent->properties->length);
  //if(tmpEvent->properties->length != 0) freeList(tmpEvent->properties);
  freeList(tmpEvent->properties);
  freeList(tmpEvent->alarms);
  free(tmpEvent);
}

int compareEvents(const void* first, const void* second){
  //compare the two strings
  return 0;
}

char* printEvent(void* toBePrinted){
  Event* tmpEvent;

  if (toBePrinted == NULL){
    return NULL;
  }

  tmpEvent = (Event*)toBePrinted;

  int len = strlen("BEGIN:VEVENT") + 10;
  char* str = malloc(sizeof(char)* len);

  char* strDate1 = printDate(&(tmpEvent->creationDateTime));
  char* strDate2 = printDate(&(tmpEvent->startDateTime));

  char* properties = toString(tmpEvent->properties);
  char* alarms = toString(tmpEvent->alarms);

  char uid[5000];

  sprintf(uid, "UID:%s\n", tmpEvent->UID);

  str[0] = '\0';

  strcpy(str,"BEGIN:VEVENT\n");
  myStrCat(&str, uid);
  myStrCat(&str, "DTSTAMP:");
  myStrCat(&str, strDate1);
  myStrCat(&str, "\nDTSTART:");
  //myStrCat(&str, "\n");
  myStrCat(&str, strDate2);
  //myStrCat(&str, "\n");
  myStrCat(&str, properties);
  myStrCat(&str, alarms);
  myStrCat(&str, "\nEND:VEVENT\n");
  str[strlen(str)] = '\0';


  free(strDate1);
  free(strDate2);
  free(properties);
  free(alarms);

  //printf("printEvent returns:\n%s", str);
  return str;
  //need to malloc for uid, createDateTime, startDateTime, properties and alarms

  //properties and alarms are strings so we can just use toString from the LinkedListAPI
}

void deleteDate(void* toBeDeleted){
  DateTime* tmpDate;

  if(toBeDeleted == NULL){
    return;
  }

  tmpDate = (DateTime*)toBeDeleted;

  free(tmpDate);
}

int compareDates(const void* first, const void* second){
  return 0;
}

char* printDate(void* toBePrinted){
  if(toBePrinted==NULL){
    //printf("DATE IS NULL\n");
    return NULL;
  }

  DateTime* tmpDate = (DateTime*) toBePrinted;
  char* str = malloc(20);

  str[0] = '\0';

  sprintf(str, "%s%s", tmpDate->date, tmpDate->time);
  if(tmpDate->UTC == true){
    strcat(str, "Z");
  }

  str[strlen(str)] = '\0';

  return str;
}

void deleteAlarm(void* toBeDeleted){
  if (toBeDeleted == NULL){
    return;
  }

  Alarm* tmpAlarm = (Alarm*)toBeDeleted;

  //dont need to free action because it is a static array
  free(tmpAlarm->trigger); //free trigger (char*)
  freeList(tmpAlarm->properties); //free list (List*)
  free(tmpAlarm); //free the object itself
}

int compareAlarms(const void* first, const void* second){
  return 0;
}

char* printAlarm(void* toBePrinted){
  if (toBePrinted == NULL){
    return NULL;
  }

  Alarm* tmpAlarm = (Alarm*) toBePrinted;

  int currentLength = 15;
  char* str = malloc(currentLength);

  char action[210];

  int triggerLength = strlen(tmpAlarm->trigger) + 10;
  char trigger[triggerLength];

  char* properties = toString(tmpAlarm->properties);
  properties = realloc(properties, strlen(properties)+2);
  properties[strlen(properties)] = '\0';

  sprintf(action, "ACTION:%s\n", tmpAlarm->action);
  sprintf(trigger, "TRIGGER:%s", tmpAlarm->trigger);

  str[0] = '\0';
  strcpy(str, "BEGIN:VALARM\n");
  str[strlen(str)] = '\0';

  myStrCat(&str, action);
  str[strlen(str)] = '\0';

  myStrCat(&str, trigger);
  str[strlen(str)] = '\0';

  myStrCat(&str, properties);
  str[strlen(str)] = '\0';

  myStrCat(&str, "\nEND:VALARM");
  str[strlen(str)] = '\0';

  //printf("%s", str);

  free(properties);

  return str;
}

/** Function to "convert" the ICalErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
char* printError(ICalErrorCode err){
  //char* errr = calloc (100, sizeof(char));
  char* error[] = {"OK", "INV_FILE", "INV_CAL", "INV_VER", "DUP_VER", "INV_PRODID", "DUP_PRODID", "INV_EVENT", "INV_DT", "INV_ALARM", "WRITE_ERROR", "OTHER_ERROR" };
  //char* toString = calloc(100, sizeof(char));
  //sprintf(toString, "%s", error[err]);
  //return toString;
  return error[err];
  //strcpy(errr,error[err]);
  //return errr;
}

/************************ ASSIGNMENT 2, MODULE 3 ******************************/

/**
 * Converts a DateTime struct to a string in JSON formate. The function must
 * return a newly allocated string in the following format:
 * {"date": "date val", "time":"time val", "isUTC":utcVal}
 * where quotations indicate that the the format is a string
 */
char* dtToJSON(DateTime dt){
  //if (&(dt) == NULL) return NULL;

  int length = 10000 + strlen(dt.date) + strlen(dt.time);
  char* json = calloc(length, sizeof(char));

  sprintf(json, "{\"date\":\"%s\",\"time\":\"%s\",\"isUTC\":%s}", dt.date, dt.time, dt.UTC ? "true": "false");

  return json;
}

/**
 * Converts an event struct to a string in JSON format
 */
char* eventToJSON(const Event* event){
  if (event == NULL) return "{}";

  char* dtjson = dtToJSON(event->startDateTime);

  //count the number of alarms
  int numAlarms = getLength(event->alarms);

  //add three to num prop for creationDateTime, startDateTime, and UID
  int numProp = getLength(event->properties) + 3;

  char* summary = calloc(10000, sizeof(char));
  strcpy(summary, "");

  //find summary
  ListIterator iter = createIterator(event->properties);

  Property* prop;
  while ((prop = nextElement(&iter))!=NULL && strcmp(summary, "") == 0){
    if (myStrCmp(prop->propName, "SUMMARY") == 0){
      int newLength = strlen(prop->propDescr);
      summary = realloc(summary, (10000 + newLength) * sizeof(char));
      strcpy(summary, prop->propDescr);
    }
  }

  int strlength = strlen(dtjson) + strlen(summary) + 2;

  char* json = calloc(10000+strlength, sizeof(char));

  sprintf(json, "{\"startDT\":%s,\"numProps\":%d,\"numAlarms\":%d,\"summary\":\"%s\"}", dtjson, numProp, numAlarms, summary);
  free(dtjson);
  free(summary);
  return json;
}

/**
 * conevrt a list of events into a json string
 */
char* eventListToJSON(const List* eventList){
  if (eventList == NULL) return "[]";

  int numEvents = getLength((List*)eventList);
  int currEvents = 0;
  char* json = calloc(10000, sizeof(char));

  strcpy(json, "[");

  ListIterator iter = createIterator((List*)eventList);

  char* temp = NULL;
  Event* ev;
  while ((ev = nextElement(&iter))!=NULL){
    currEvents++;
    temp = eventToJSON(ev);
    strcat(json, temp);
    free(temp);
    if (currEvents == numEvents){
      strcat(json, "]");
    } else {
      strcat(json, ",");
    }
  }

  return json;
}

/**
 * convert a calendar struct ot a string in JSON format
 */
char* calendarToJSON(const Calendar* cal){
  if (cal == NULL) return "{}";
  //List* temp = cal->events;
  char* evtVal = calloc(10000, sizeof(char));
  //int numEvents = temp->length;
  int numEvents = numEvents = getLength(cal->events);
  //printf("num events:%d*************\n", numEvents);

  if (numEvents == 0){
    strcpy(evtVal, "{}");
  } else {
    evtVal = realloc(evtVal, (10000 + numEvents)*sizeof(char));
    sprintf(evtVal, "%d", numEvents);
  }

  int length = strlen(evtVal) + strlen(cal->prodID);
  char* json = calloc(100000 + length, sizeof(char));

  int verVal = floor(cal->version);
  int propVal = getLength(cal->properties) + 2;

  sprintf(json, "{\"version\":%d,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%s}", verVal, cal->prodID, propVal, evtVal);

  free(evtVal);
  return json;
  //return NULL;
}

/**
 * create a very simple calendar object from a jSOn string, will be partially incomplete
 */
Calendar* JSONtoCalendar(const char* str){
  if (str == NULL) return NULL;

  Calendar* newCalendar   = calloc(1, sizeof(Calendar));
  //initialize the lists for properties and events
  newCalendar->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
  newCalendar->events     = initializeList(&printEvent,    &deleteEvent,    &compareEvents);

  char verVal[10000];
  int c = 0;
  int i = 0;

  while (str[i++]!=':' && i < strlen(str));
  if (i == strlen(str)){
    deleteCalendar(newCalendar);
    return NULL;
  }

  for (; i < strlen(str) && str[i]!=','; i++){
    verVal[c++] = str[i];
    verVal[c] = '\0';
  }

  newCalendar->version = atof(verVal);

  while (str[i++]!=':' && i < strlen(str));
  if (i == strlen(str)){
    deleteCalendar(newCalendar);
    return NULL;
  }

  char verProdID[10000];
  c = 0;
  for (i = i+1; i < strlen(str) - 2; i++){
    verProdID[c++] = str[i];
    verProdID[c] = '\0';
  }

  strcpy(newCalendar->prodID, verProdID);

  return newCalendar;
}

Event* JSONtoEvent(const char* str){
  if (str == NULL) return NULL;

  Event* newEvent = calloc(1, sizeof(Event));

  newEvent->properties = initializeList( &printProperty, &deleteProperty, &compareProperties );
  newEvent->alarms     = initializeList( &printAlarm,    &deleteAlarm,    &compareAlarms );
  int i = 0;
  while (str[i++]!=':' && i < strlen(str));
  if (i == strlen(str)){
    deleteEvent(newEvent);
    return NULL;
  }
  int c = 0;
  char uid[1000];
  for (i = i + 1; i < strlen(str) - 2 ; i++){
    uid[c++] = str[i];
    uid[c] = '\0';
  }

  strcpy(newEvent->UID, uid);

  return newEvent;
}

void addEvent(Calendar* cal, Event* toBeAdded){
  if (cal == NULL || toBeAdded == NULL) return;

  insertBack(cal->events, toBeAdded);

}

//used for filelog
char* icsToJSON(char* fileName){
  Calendar* cal;
  //char* temp = calloc(100, sizeof(char));
  //strcpy(temp, "../uploads/");
  //myStrCat(&temp, fileName);
  //createCalendar(temp, &cal);
  ICalErrorCode err = createCalendar(fileName, &cal);
  char* json = calloc(100000, sizeof(char));
  if (err == OK){
    int verVal = floor(cal->version);
    sprintf(json, "{\"filename\":\"%s\",\"version\":%d,\"prodID\":\"%s\",\"numProps\":%d,\"numEvents\":%d}", fileName, verVal, cal->prodID, getLength(cal->properties) + 2, getLength(cal->events));
    // /printf("%s\n",json);
    deleteCalendar(cal);
    return json;
  }
  //free(cal);
  //sprintf(json, "{\"filename\":\"%s\",\"version\":%d,\"prodID\":\"Invaild file\",\"numProps\":%d,\"numEvents\":%d}", fileName, 0, 0, 0);
  ///return json;
  free(json);
  return "{}";
}

char* icsToEventListJSON(char* fileName){
  Calendar* cal;
  ICalErrorCode err = createCalendar(fileName, &cal);
  if (err == OK){
    ICalErrorCode validate = validateCalendar(cal);
    if (validate == OK){
      char* json = eventListToJSON(cal->events);
      //deleteCalendar(cal);
      return json;
    }
  }
  return "{}";
}

char* optionalPropToJSON(Property* prop){
  char* json = calloc(1000, sizeof(char));
  sprintf(json, "{\"%s\":\"%s\"}", prop->propName, prop->propDescr);
  return json;
}

char* optionalPropsToJSON(Event* ev){
  if (ev == NULL) return "[]";

  int currEvents = 0;
  char* json = calloc(10000, sizeof(char));

  strcpy(json, "[");
  List* propList = ev->properties;
  int numEvents = getLength((List*)propList);
  ListIterator iter = createIterator((List*)propList);

  char* temp = NULL;
  Property* prop;
  while ((prop = nextElement(&iter))!=NULL){
    currEvents++;
    temp = optionalPropToJSON(prop);
    strcat(json, temp);
    //free(temp);
    if (currEvents == numEvents){
      strcat(json, "]");
      return json;
    } else {
      strcat(json, ",");
    }
  }
  //free(json);
  return "[]";
}

char* listOfOptionalPropJSON(char* fileName, int eventIndex){
  Calendar* cal;
  ICalErrorCode err = createCalendar(fileName, &cal);
  if (err == OK){
    List* eventList = cal->events;
    ListIterator iter = createIterator((List*)eventList);
    Event* ev = NULL;
    for(int i = 0; i < eventIndex && ((ev = nextElement(&iter))!=NULL); i++);
    char* json = optionalPropsToJSON(ev);
    //deleteCalendar(cal);
    return json;
  }

  return "{}";
}

char* alarmToJSON(Alarm* alarm){
  if (alarm == NULL) return "{}";

  char* json = calloc(1000, sizeof(char));

  sprintf(json, "{\"action\":\"%s\",\"trigger\":\"%s\",\"numProps\":%d}", alarm->action, alarm->trigger, getLength(alarm->properties));
  return json;
}

char* alarmListToJSON(List* alarmList){
  if (alarmList == NULL) return "[]";

  ListIterator iter = createIterator(alarmList);
  int numAlarms = getLength(alarmList);
  int currAlarm = 0;
  char* json = calloc(10000, sizeof(char));

  strcpy(json, "[");
  Alarm* alr = NULL;
  while ((alr = nextElement(&iter))!=NULL){
    currAlarm++;
    char* temp = alarmToJSON(alr);
    strcat(json, temp);
    free(temp);
    if (currAlarm == numAlarms){
      strcat(json, "]");
      return json;
    } else {
      strcat(json, ",");
    }
  }
  free(json);
  return "[]";
}

char* alarmListToJSONWrapper(char* fileName, int eventIndex){
  Calendar* cal;
  ICalErrorCode err = createCalendar(fileName, &cal);
  if (err == OK){
    ICalErrorCode validate = validateCalendar(cal);
    if (validate == OK){
      List* eventList = cal->events;
      ListIterator iter = createIterator((List*)eventList);
      Event* ev = NULL;
      for(int i = 0; i < eventIndex && ((ev = nextElement(&iter))!=NULL); i++);
      char* json = alarmListToJSON(ev->alarms);
      //deleteCalendar(cal);
      return json;
    }
  }
  return "[]";
}

char* newICSFile(char* filename, char* version, char* prodID, char* uid, char* startDT, char* creationDT, char* summary){
  ICalErrorCode err = OK;
  Calendar* newCal = calloc(1, sizeof(Calendar));
  char* newVersion = substring(version, 2, strlen(version)-2);
  newCal->version = atof(newVersion);
  strcpy(newCal->prodID, substring(prodID,2,strlen(prodID)-2));

  newCal->properties = initializeList(&printProperty, &deleteProperty, &compareProperties);
  newCal->events     = initializeList(&printEvent,    &deleteEvent,    &compareEvents);

  Event* newEvent = calloc(1, sizeof(Event));
  DateTime* startDateTime = calloc(1, sizeof(DateTime));
  DateTime* creationDateTime = calloc(1, sizeof(DateTime));

  char* newStartDT = substring(startDT, 2, strlen(startDT)-2);

  err = createDate(newStartDT, startDateTime);
  if (err!=OK){
    char* error = calloc(100, sizeof(char));
    sprintf(error, "failure to create startDT (given string %s)", newStartDT);
    //return error;
    //return printError(err);
    //sprintf(error, "%s: %d",newStartDT,strlen(newStartDT) );
    return error;
  }
  char* newCreationDT = substring(creationDT, 2, strlen(creationDT)-2);

  err = createDate(newCreationDT, creationDateTime);
  if (err!=OK){
    char* error = calloc(100, sizeof(char));

    sprintf(error, "failure to create creationDT (given string %s)", newCreationDT);
    return error;
    //return printError(err);

  }

  strcpy(newEvent->UID, substring(uid,2,strlen(uid)-2));
  newEvent->startDateTime = *startDateTime;
  newEvent->creationDateTime = *creationDateTime;
  //initialize event lists
  newEvent->properties = initializeList( &printProperty, &deleteProperty, &compareProperties );
  newEvent->alarms     = initializeList( &printAlarm,    &deleteAlarm,    &compareAlarms );

  char* newSummary = substring(summary, 2, strlen(summary)-2);
  if (strcmp(newSummary,"")!=0){
    Property* summary = calloc(1, sizeof(Property)+ sizeof(char[999]));
    strcpy(summary->propName, "SUMMARY");
    strcpy(summary->propDescr, newSummary);
    insertBack(newEvent->properties, summary);
  }

  insertBack(newCal->events, newEvent);

  err = validateCalendar(newCal);
  if (err!=OK){
    return printError(err);
    //return "calendar to create failed validation tests";
  }
  char* newFileName = substring(filename, 2, strlen(filename)-2);
  err = writeCalendar(newFileName, newCal);
  if (err !=OK){
    char* error = calloc(100, sizeof(char));

    sprintf(error, "failure to write calendar into %s", newFileName);
    return error;
    //return "failure to write calendar into ";
  }
  //deleteCalendar(newCal);
  return "SUCCESS";
  /*char* params = calloc(10000, sizeof(char));
  sprintf(params,"%s%s%s%s%s%s",filename,version,prodID,uid,startDT,creationDT );
  return params;*/
}

char* addEventFromFileName(char* path, char* quotedfilename, char* uid, char* startDT, char* creationDT, char* summary){
  Calendar* newCal = calloc(1, sizeof(Calendar));
  char* filename = substring(quotedfilename,2,strlen(quotedfilename)-2);
  char* newPath = substring(path, 2, strlen(path)-2);
  ICalErrorCode err = createCalendar(newPath,&newCal);
  if (err == OK){
    DateTime* startDateTime = calloc(1, sizeof(DateTime));
    DateTime* creationDateTime = calloc(1, sizeof(DateTime));

    char* newStartDT = substring(startDT, 1, strlen(startDT));

    err = createDate(newStartDT, startDateTime);
    if (err!=OK){
      char* error = calloc(100, sizeof(char));
      sprintf(error, "failure to create startDT (given string %s)", newStartDT);
      //return error;
      //return printError(err);
      //sprintf(error, "%s: %d",newStartDT,strlen(newStartDT) );
      return error;
    }
    char* newCreationDT = substring(creationDT, 1, strlen(creationDT));

    err = createDate(newCreationDT, creationDateTime);
    if (err!=OK){
      char* error = calloc(100, sizeof(char));

      sprintf(error, "failure to create creationDT (given string %s)", newCreationDT);
      return error;
      //return printError(err);

    }
    Event* newEvent = calloc(1,sizeof(Event));
    strcpy(newEvent->UID, substring(uid,2,strlen(uid)-2));
    newEvent->startDateTime = *startDateTime;
    newEvent->creationDateTime = *creationDateTime;
    //initialize event lists
    newEvent->properties = initializeList( &printProperty, &deleteProperty, &compareProperties );
    newEvent->alarms     = initializeList( &printAlarm,    &deleteAlarm,    &compareAlarms );

    //char* newSummary = substring(summary, 1, strlen(summary)-1);
    if (strcmp(summary,"")!=0){
      Property* sum = calloc(1, sizeof(Property)+ sizeof(char[999]));
      strcpy(sum->propName, "SUMMARY");
      strcpy(sum->propDescr, summary);
      insertBack(newEvent->properties, sum);
    }

    insertBack(newCal->events, newEvent);
    err = validateCalendar(newCal);
    if (err!=OK){
      char* error = calloc(100, sizeof(char));

      sprintf(error, "failure to validate new event to %s due to %s", filename, printError(err));
      return error;
      //return printError(err);
      //return "calendar to create failed validation tests";
    }

    err = writeCalendar(newPath, newCal);
    if (err !=OK){
      char* error = calloc(100, sizeof(char));

      sprintf(error, "failure to write new event to %s due to %s", filename, printError(err));
      return error;
      //return "failure to write calendar into ";
    }
    //deleteCalendar(newCal);
    return "SUCCESS";
  }

  char* error = calloc(100, sizeof(char));

  sprintf(error, "failure to create calendar in %s (%s)", filename, printError(err));
  return error;
  /*char* params = calloc(10000, sizeof(char));
  sprintf(params,"%s%s%s%s%s",path, quotedfilename,uid,startDT,creationDT );
  return params;*/
}

char *substring(char *string, int position, int length){
   char *pointer;
   int c;

   pointer = malloc(length+1);
   if (pointer == NULL){
      printf("Unable to allocate memory.\n");
      exit(1);
   }
   for (c = 0 ; c < length ; c++){
      *(pointer+c) = *(string+position-1);
      string++;
   }
   *(pointer+c) = '\0';
   return pointer;
}
