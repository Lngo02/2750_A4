'use strict'

// C library API
const ffi = require('ffi');
const ref = require("ref"); //c pointer
const mysql = require('mysql');

var connection; //error if const
var validFiles = [];
var calendars = [];


var Calendar = ref.types.void;
var CalendarPtr = ref.refType(Calendar);


//create new object called sharedLib and the C functions becomes its methods
let parserLib = ffi.Library('./libcal', {
  'icsToJSON': [ 'string', [ 'string' ] ], //returns calendar json but with filename, takes in filename
  'icsToEventListJSON': [ 'string', [ 'string' ] ],
  'listOfOptionalPropJSON': [ 'string', [ 'string', 'int' ] ],
  'alarmListToJSONWrapper': [ 'string', ['string', 'int' ] ],
  'newICSFile': ['string', ['string', 'string', 'string', 'string', 'string', 'string', 'string']],
  'addEventFromFileName':['string', ['string','string','string','string','string','string']],
});

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});


//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  let uploadFile = req.files.uploadFile;

  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** /


function getFiles(dir){
  return fs.readdirSync(dir);
}


//gets list of files in the uploads folder
app.get('/uploads', function(req, res){
  /*let folder = './uploads/';
  let files = fs.readdirSync(folder);*/
  validFiles=[];
  calendars=[];
  let files = getFiles('./uploads');

  for (let i = 0; i < files.length; i++){
    let currentFile = __dirname+"/uploads/" + files[i];
    let jsonStr = parserLib.icsToJSON(currentFile);
    if (jsonStr!="{}"){
      calendars.push(JSON.parse(jsonStr));
      validFiles.push(files[i]);
    }
  }
  res.send(validFiles);
});

//get content for the file log panel when the file is loaded intially
app.get('/initFileLog', function(req, res){
  let files = getFiles('./uploads');
  let fileLog = [];
  for (let i = 0; i < files.length; i++){
    let currentFile = "./uploads/" + files[i];
    //function icsToJSON creates a calendar from the file
    //then converts the calendar to JSON
    //returns a JSON in the form {"filename":filename,<calendarJSON>}
    let jsonStr = parserLib.icsToJSON(currentFile);
    if (jsonStr!="{}"){
      let jsonObj = JSON.parse(jsonStr);
      fileLog.push(jsonObj);
    }
    //let jsonStr = '{"filename":' + currentFile +

    //console.log(currentFile);
  }
  res.send(fileLog);
});

app.get('/viewCalendar', function(req, res){
  let file = req.query.fileSelected;
  let path = __dirname+"/uploads/" + file;
  let eventsJSON = parserLib.icsToEventListJSON(path);
  let json = [];
  if(eventsJSON != "[]"){
    json = JSON.parse(eventsJSON);
  }
  res.send(json);
});

app.get('/extract', function(req, res){
  let file = req.query.fileSelected;
  let path = __dirname+"/uploads/"+ file;
  let eventIndex = req.query.eventNoSelected;
  let optJSON = parserLib.listOfOptionalPropJSON(path, eventIndex);
  let json = JSON.parse(optJSON);
  res.send(json);
});

app.get('/getAlarms', function(req, res){
  let file = req.query.fileSelected;
  let path = __dirname+"/uploads/" + file;
  let eventIndex = req.query.eventNoSelected;
  let alarmListJSON = parserLib.alarmListToJSONWrapper(path, eventIndex);
  let json = JSON.parse(alarmListJSON);
  res.send(json);
});


app.get('/uploadNewCalendar', function(req, res) {
  let file = req.query.uploadFile;
  let version = req.query.version;
  let prodID = req.query.prodID;
  let uid = req.query.eventUID;
  let startDT = req.query.startDT;
  let creationDT = req.query.creationDT;
  let summary = req.query.summary;
  let path = "\"" + __dirname+"/uploads/" + file + "\"" ;
  let params = path + version + prodID + uid + startDT + creationDT;

  let outcome = parserLib.newICSFile(path, version, prodID, uid, startDT, creationDT,summary);
  res.send(outcome);
  //res.send(params);
});

app.get('/addEvent', function(req, res) {
  let file = "\"" + req.query.filename + "\"";
  let uid = req.query.eventUID;
  let startDT = req.query.startDT;
  let creationDT = req.query.creationDT;
  let summary = req.query.summary;
  let path = "\"" + __dirname+"/uploads/" +req.query.filename + "\"" ;
  let outcome = parserLib.addEventFromFileName(path,file, uid, startDT, creationDT,summary);
  res.send(outcome);
  //res.send(params);
  //res.send("null");
});

app.get('/validateFile', function(req, res){
  let path = __dirname+"/uploads/" +req.query.filename;
  let jsonStr = parserLib.icsToJSON(path);
  if (jsonStr!="{}"){
    res.send("VALID");
  } else {
    fs.unlinkSync(path);
    res.send(jsonStr);
  }
})

app.get('/mylogin', function(req, res){
  connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : 'lngo02',
    password : '1004683',
    database : 'lngo02'
  });

  connection.connect();
});

app.get('/login', function(req, res){
  let username = req.query.user;
  let password = req.query.pass;
  let database = req.query.database;
  //console.log(username + password + username);
  connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : username,
    password : password,
    database : database
  });

  connection.connect(function(err){
    if (err){
      res.send(err);
    } else {
      res.send(true);
    }
  });
});

//Sample endpoint
app.get('/someendpoint', function(req , res){
  res.send({
    foo: "bar"
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);

app.post('/storeAllFiles', function(req, res){
  if (connection != null){
    //store data for the file log
    //console.log("storeAllFiles");
    //create tables
    createFileTable();
    createEventTable();
    createAlarmTable();

    deleteTables();


    //populate file table
    /*for (let i = 0; i < validFiles.length; i++){
      //console.log("validFiles[i] = " + validFiles[i]);
      //console.log("calendars[i].filename = " + calendars[i].filename);
      let fileQuery = insertFileIntoDB(calendars[i]);
      //console.log("inserting file");
      connection.query(fileQuery, function(err, results, fields){
        if (err) {
          console.log(err.message);
        }
      });
    }*/
    let fileQuery = insertFilesIntoDB();
    connection.query(fileQuery, function(err, results, fields){
      if (err){
        console.log(err);
      }
    });
    /*connection.query(fileQuery, function(err, results, fields){
      if (err){
        console.log(err);
      } else {
        connection.query("SELECT * FROM FILE;", function(error, results, fields){
          if (error){
            console.log(error.message);
            console.log(error);
          } else {
            for (let calendar of results){
              let path = __dirname+'/uploads/' + calendar.file_Name;
              let eventsJSON = parserLib.icsToEventListJSON(path);
              //console.log(eventsJSON);
              let events = [];
              events = JSON.parse(eventsJSON);
              //console.log(events);
              for (let j = 1; j <= events.length; j++){
                let optionalProps = parserLib.listOfOptionalPropJSON(path, j);
                let props = JSON.parse(optionalProps);
                let eventQuery = insertEventIntoDB(events[j-1], props, calendar.cal_id);
                //console.log("eventQuery" + eventQuery);
                connection.query(eventQuery, function (error, result){
                  if (error){
                    console.log(error);
                  } else {
                    //console.log("created event row!");
                    //console.log(results);
                    let eventID = result.insertId;
                    //console.log("eventID = " + eventID);
                    let alarmList = parserLib.alarmListToJSONWrapper(path, j);
                    let alarms = JSON.parse(alarmList);
                    //console.log("alarmList: " + JSON.stringify(alarms));
                    //add each alarm to the alarm table
                    for (let k = 0; k < alarms.length; k++){
                      //console.log("eventID = " + eventID);
                      let alarmQuery = insertAlarmIntoDB(alarms[k], eventID);
                      //console.log(alarmQuery);
                      connection.query(alarmQuery, function(error, results){
                        if (error){
                          console.log(error);
                        }
                      });
                    }
                  }
                });
              }
            }
          }
        });
      }
    });*/

    //store data of the events

    //populate event and alarm table
    //console.log("insert the events");
    connection.query("SELECT * FROM FILE;", function(error, results, fields){
      if (error){
        console.log(error);
      } else {
        for (let calendar of results){
          let path = __dirname+'/uploads/' + calendar.file_Name;
          let eventsJSON = parserLib.icsToEventListJSON(path);
          //console.log(eventsJSON);
          let events = [];
          events = JSON.parse(eventsJSON);
          //console.log(events);
          for (let j = 1; j <= events.length; j++){
            let optionalProps = parserLib.listOfOptionalPropJSON(path, j);
            //console.log(calendar.file_Name);
            //console.log(j);
            //console.log("optionalProps:" + optionalProps);
            let props = JSON.parse(optionalProps);
            //console.log("props: " + props);
            //console.log("event[j]: " + events[j-1]);
            //console.log(JSON.stringify(events[j-1]));
            //console.log("calendar.fileID: " + calendar.cal_id);
            let eventQuery = insertEventIntoDB(events[j-1], props, calendar.cal_id);
            //console.log("eventQuery" + eventQuery);
            connection.query(eventQuery, function (error, result){
              if (error){
                console.log(error);
              } else {
                //console.log("created event row!");
                //console.log(results);
                let eventID = result.insertId;
                //console.log("eventID = " + eventID);
                let alarmList = parserLib.alarmListToJSONWrapper(path, j);
                let alarms = JSON.parse(alarmList);
                //console.log("alarmList: " + JSON.stringify(alarms));
                //add each alarm to the alarm table
                for (let k = 0; k < alarms.length; k++){
                  //console.log("eventID = " + eventID);
                  let alarmQuery = insertAlarmIntoDB(alarms[k], eventID);
                  //console.log(alarmQuery);
                  connection.query(alarmQuery, function(error, results){
                    if (error){
                      console.log(error);
                    }
                  });
                }
              }
            });
          }
        }
      }
    });

  }
});


app.post('/clearAllData', function(req, res){
  if (connection != null){
    deleteTables();
  }
});

app.get('/displayDBStatus', function(req, res){
  if (connection != null){
    let fileCount = 0;
    let eventCount = 0;
    let alarmCount = 0;

    connection.query('SELECT COUNT(*) AS numFiles FROM FILE;', function(err, result){
      if (err){
        console.log(err);
      } else {
        //fileCount = result.length;
        //console.log(result);
        fileCount = result[0].numFiles;
        connection.query('SELECT COUNT(*) AS numEvents FROM EVENT;', function(err, result){
          if (err){
            console.log(err);
          } else {
            eventCount = result[0].numEvents;
            connection.query('SELECT COUNT(*) AS numAlarms FROM ALARM;', function(err, result){
              if (err){
                console.log(err);
              } else {
                alarmCount = result[0].numAlarms;
                let status = "Database has " + fileCount + " files, " + eventCount + " events, and " + alarmCount + " alarms";
                //console.log(status);
                res.send(status);
              }
            });
          }
        });
      }
    });

  }
});



app.get('/executeQuery', function(req, res){
  if (connection != null){
    connection.query(req.query.query, function(err, result){
      if (err){
        console.log(err);
      } else {
        //console.log(result);
        res.send(result);
      }
    })
  } else {
    //console.log("not connected to database");
  }
});

/* functions to create mysql tables*/
function createFileTable(){
  let createFileTable = "CREATE TABLE IF NOT EXISTS FILE ("
                           + "cal_id INT AUTO_INCREMENT, "
                           + "file_Name VARCHAR(60) NOT NULL, "
                           + "version INT NOT NULL, "
                           + "prod_id VARCHAR(256) NOT NULL, "
                           + "PRIMARY KEY (cal_id) )";
  //console.log(createFileTable);
  connection.query(createFileTable, function(err, results, fields){
    if(err) {
      console.log(err.message);
    }
  });
  //deleteTable("FILE");
}

function insertFilesIntoDB(){
  let heading = "(file_Name, version, prod_id)";

  let values = "";

  if(validFiles.length == 1){
    let path = calendars[0].filename;
    let filename = path.substring(path.lastIndexOf('/') + 1);
    values = "('" + filename + "', '"
                  + calendars[0].version  + "', '"
                  + calendars[0].prodID  + "')";
  } else {
    for (let i = 0; i < (validFiles.length-1); i++){
      let path = calendars[i].filename;
      let filename = path.substring(path.lastIndexOf('/') + 1);
      values = values + "('" + filename + "', '"
                      + calendars[i].version  + "', '"
                      + calendars[i].prodID  + "'),";
    }

    //for the last one no comma
    let lastElem = validFiles.length-1;
    let path = calendars[lastElem].filename;
    let filename = path.substring(path.lastIndexOf('/') + 1);
    values = values + "('" + filename + "', '"
                    + calendars[lastElem].version  + "', '"
                    + calendars[lastElem].prodID  + "')";
  }

  let table = "INSERT INTO FILE "  + heading + " VALUES " + values + ";";
  return table;
}

function insertFileIntoDB(data){
  let path = data.filename;
  let filename = path.substring(path.lastIndexOf('/') + 1);
  let heading = "(file_Name, version, prod_id)";
  let values = "('" + filename + "', '"
                    + data.version  + "', '"
                    + data.prodID  + "')";
  let tableRow = "INSERT INTO FILE "  + heading + " VALUES " + values + ";";
  return tableRow;
}

function createEventTable(){
  let createEventTable = "CREATE TABLE if not exists EVENT ("
    + "event_id  INT AUTO_INCREMENT,"
    + "summary VARCHAR(1024),"
    + "start_time DATETIME NOT NULL,"
    + "location VARCHAR(60),"
    + "organizer VARCHAR(256),"
    + "cal_file INT NOT NULL,"
    + "PRIMARY KEY(event_id),"
    + "FOREIGN KEY(cal_file) REFERENCES FILE(cal_id) ON DELETE CASCADE )";

  connection.query(createEventTable, function(err, results, fields){
    if (err){
      console.log(err.message);
    }
  });
}

function insertEventIntoDB(ev, optionalProps, fileID){
  let location = "NULL";
  let organizer = "NULL";
  let summary = "NULL";
  let props = optionalProps;
  //console.log(props);
  //console.log(JSON.stringify(props));
  /*if (props != undefined){
    if (props.hasOwnProperty("location")){
      location = props.location;
    }
    if (props.hasOwnProperty("organizer")){
      organizer = props.organizer;
    }
  }*/
  for (let prop of props){
    if (prop.hasOwnProperty("LOCATION")){
      location = prop.LOCATION;
      //console.log("location: " + location);
    }
    if (prop.hasOwnProperty("ORGANIZER")){
      organizer = prop.ORGANIZER;
      //console.log("organizer: " + organizer);
    }
  }



  //console.log("event = " + ev);

  if (ev.summary != ""){
    summary = ev.summary;
  }

  let startTime = dtToSQL(ev.startDT);
  /*console.log("location: " + location);
  console.log("organizer: " + organizer);
  console.log("summary: " + summary);
  console.log("start date: " + startTime);
  console.log("fileID: " + fileID);*/

  let heading = "(summary, start_time, location, organizer, cal_file)";
  let values = "('" + summary + "', '"
                    + startTime + "', '"
                    + location + "', '"
                    + organizer + "', '"
                    + fileID + "')";
  let tableRow = "INSERT INTO EVENT " + heading + " VALUES " + values + ";";
  return tableRow;
}

function createAlarmTable(){
    let createAlarmTable = "CREATE TABLE if not exists ALARM ("
      + "alarm_id INT AUTO_INCREMENT,"
      + "action VARCHAR(256) NOT NULL,"
      + "`trigger` VARCHAR(256) NOT NULL,"
      + "event INT NOT NULL,"
      + "PRIMARY KEY(alarm_id),"
      + "FOREIGN KEY(event) REFERENCES EVENT(event_id) ON DELETE CASCADE )";

    connection.query(createAlarmTable, function(err, results, fields){
      if (err){
        console.log(err.message);
      }
    });
}

function insertAlarmIntoDB(data, calID){
  let heading = "(action, `trigger`, event)";
  let values = "('" + data.action + "', '"
                    + data.trigger  + "', '"
                    + calID + "')";
  let tableRow = "INSERT INTO ALARM " + heading + " VALUES " + values + ";";
  return tableRow;
}

function deleteTable(table){
  let query = "DELETE FROM " + table + ";";
  connection.query(query, function(err, result){
    if (err){
      console.log(err.message);
    } else {
      //console.log("successfully deleted table");
    }
  });
}

function dtToSQL(datetime){
  let date = datetime.date;
  let time = datetime.time;

  //YYYY-MM-DD HH:MM:SS
  let year = date.substring(0,4);
  let month = date.substring(4,6);
  let day = date.substring(6,8);
  let hour = time.substring(0,2);
  let minute = time.substring(2,4);
  let second = time.substring(4,6);
  let sqlDate = year + "-" + month + "-" + day;
  let sqlTime = hour + ":" + minute + ":" + second;

  let sql = sqlDate + " " + sqlTime;

  return sql;

}

function deleteTables(){
  deleteTable("FILE");
  deleteTable("EVENT");
  deleteTable("ALARM");

  //set all their auto increments to 1
  connection.query("ALTER TABLE FILE AUTO_INCREMENT = 1", function(err, result){
    if (err) {
      console.log(err);
    }
  });

  //set all their auto increments to 1
  connection.query("ALTER TABLE EVENT AUTO_INCREMENT = 1", function(err, result){
    if (err) {
      console.log(err);
    }
  });

  //set all their auto increments to 1
  connection.query("ALTER TABLE ALARM AUTO_INCREMENT = 1", function(err, result){
    if (err) {
      console.log(err);
    }
  });
}
