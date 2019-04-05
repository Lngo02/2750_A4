// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    disableDatabase();

    if (sessionStorage.getItem('user')!=null){
      let username = sessionStorage.getItem('user');

      console.log(username);
      let password = sessionStorage.getItem('pass');
      console.log(password);
      let database = sessionStorage.getItem('database');
      console.log(database);

      $.ajax({
        type: 'get',
        url: '/login',
        data: {user: username, pass: password, database: database},
        success: function(data){
          if (data != true){
            $('#status').val($('#status').val() + "connection to database has errored: " + data.sqlMessage +  ". Please login to the database again." + "\n");
            console.log(data);
            scrollToBottom();
          } else {
            enableDatabase();

          }
        }
      });
    }

    //If the page refreshed because of an uploaded file, restore its revious state
    if (sessionStorage.getItem('statusBar') != null){
      document.querySelector('#status').value = sessionStorage.getItem('statusBar');
      scrollToBottom();
      sessionStorage.removeItem('statusBar');
    }

    if (sessionStorage.getItem('file')!=null){
      let fileUploaded = sessionStorage.getItem('file');
      if (isNULL(fileUploaded)){
        $('#status').val($('#status').val() + "No file selected to be uploaded" + "\n");
        scrollToBottom();
      } else {
        //ajax call to validate the new file
        $.ajax({
          type: 'get',
          url: '/validateFile',
          data: {filename: fileUploaded},
          success: function(data){
            console.log('/validateFile outcome ' + data);
            if(data=="VALID"){
              $('#status').val($('#status').val() + fileUploaded + " has been uploaded to the server" + "\n");
              scrollToBottom();
            } else {
              $('#status').val($('#status').val() + fileUploaded + " is an invalid file. File will be deleted from server." + "\n");
              scrollToBottom();
            }

          },
          fail: function(error){
            console.log(error);
          }
        });
      }
      sessionStorage.removeItem('file');
    }

    //Set page to default elements

    document.getElementById("fileUploadedCheckbox").checked = false;
    $('#upload').prop('disabled', true);
    $("#status").scrollTop($("#status").height());
    scrollToBottom();
    isEmpty("noFiles", "fileLogTable");
    $('#extract').prop('disabled', true);
    $('#show-alarms').prop('disabled', true);
    removeAllOptions('dynamic-select');
    removeAllOptions('file-select');
    removeAllOptions('querySelect');
    //get list of file names and update the drop down menus
    let files = [];

    $.ajax({
      type:'get',
      url: '/uploads',
      success: function(data){
        removeAllOptions('dynamic-select');
        removeAllOptions('file-select');
        removeAllOptions('querySelect');
        //console.log(data);
        files = data;
        let calendarJSON;
        let calendar;
        console.log(data);
        for (let i = 0; i < data.length; i++){
          addOption('dynamic-select',data[i]);
          addOption('file-select', data[i]);
          addOption('querySelect', data[i]);
        }
      },
      fail: function(){
        console.log(error);
      },
    });


    //update the file log table when the website initially loads
    //gets all the files in the server (in uploads) and posts them on the file log table
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/initFileLog',
      success: function(data){

        let label = document.getElementById("noFiles");
        let table = document.getElementById("fileLogTable");
        let tableRef = document.getElementById("fileLogTable").getElementsByTagName('tbody')[0]; //access <table> element

        if (data.length != 0){
          label.style.visibility = "hidden";
          for (let i = 0; i < data.length; i++){
            //insert a row in the table t the last row
            let row = tableRef.insertRow(tableRef.rows.length);

            let fileName = row.insertCell(0);
            let version = row.insertCell(1);
            let prodID = row.insertCell(2);
            let numEvents = row.insertCell(3);
            let numProps = row.insertCell(4);

            let fileNameOnly = data[i].filename.substring(10);
            fileName.innerHTML = "<a href=/uploads/" + fileNameOnly + ">" + fileNameOnly + " </a>";
            version.innerHTML = data[i].version;
            prodID.innerHTML = data[i].prodID;
            numEvents.innerHTML = data[i].numEvents;
            numProps.innerHTML = data[i].numProps;
          }
        }
      },
      fail: function(error){
        console.log(error);
      }
    });


    //the view button views the calendar selected
    //and adds the events to the calendar view table
    let view = document.querySelector('#view');
    view.addEventListener('click', function(){
      let file = $('#dynamic-select option:selected').text();
      //console.log("View file: " + file);
      deleteAllRows('calendarViewTable');
      //start of ajax to get event select
      $.ajax({
        type: 'get',
        url: '/viewCalendar',
        data: {fileSelected: file},
        success: function(data){
          //console.log(data);
          console.log("Viewing: "  + file);
          removeAllOptions('event-select');
          for (let i = 0; i < data.length; i++){
            addRowCalendarView(i+1, data[i]);
            let optionTxt = i+1;
            addOption('event-select', optionTxt);
          }
        },
        fail: function(error){
          console.log(error);
        }
      });
      //end of AJAX
      $('#extract').prop('disabled', false);
      $('#show-alarms').prop('disabled', false);
    });
    //extract the optional properties of the event
    let extract = document.querySelector('#extract');
    extract.addEventListener('click',function(){
      let file = $('#dynamic-select option:selected').text();
      let eventNumSelected = $('#event-select option:selected').text();
      //console.log("Extract optional properties for event number: " + eventNumSelected);
      //start ajax
      $.ajax({
        type: 'get',
        dataType:'json',
        url: '/extract',
        data: {fileSelected: file, eventNoSelected: eventNumSelected},
        success: function(data){
          //console.log("Extracting: " + file);
          //console.log(data);
          if (data.length == 0){
            $('#status').val($('#status').val() + "Event No. " + eventNumSelected + " of " + file + " does not have any optional properties." + "\n");
            scrollToBottom();
          } else {
            $('#status').val($('#status').val() + "Event No. " + eventNumSelected + " of " + file + " has " + data.length + " optional properties." + "\n");
            scrollToBottom();
            for(let i = 0; i < data.length; i++){
              let string = JSON.stringify(data[i]);
              let length  = string.length;
              $('#status').val($('#status').val() + "    " + string.substring(1, length - 1)+ "\n");
              scrollToBottom();
            }
          }
        },
        fail: function(error){
          console.log(error);
        }
      });
      //end ajax;
    });

    //see the alarms of the event
    let show = document.querySelector('#show-alarms');
    show.addEventListener('click', function(){
      let file = $('#dynamic-select option:selected').text();
      let eventNumSelected = $('#event-select option:selected').text();
      //console.log("Showing alarm for event number: " + eventNumSelected);
      $.ajax({
        type: 'get',
        dataType:'json',
        url: '/getAlarms',
        data: {fileSelected: file, eventNoSelected: eventNumSelected},
        success: function(data){
        //  document.getElementsByID('status')[0].scrollTop=document.getElementsByID('status')[0].scrollHeight;
          $("#status").scrollTop($("#status").height());
          $('#status').val($('#status').val() + "Event No. " + eventNumSelected + " of file " + file);
          scrollToBottom();
          if (data.length == 0){
            $('#status').val($('#status').val() + " does not have any alarms." + "\n");
          } else {
            $('#status').val($('#status').val() + " has " + data.length + " alarms." + "\n");
            $("#status").scrollTop($("#status").height());
            scrollToBottom();
            for(let i = 0; i < data.length; i++){
              let string = JSON.stringify(data[i]);
              let length  = string.length;
              $('#status').val($('#status').val() + "    " + string.substring(1, length - 1)+"\n");
              scrollToBottom();
              //console.log(string);
            }
            $("#status").scrollTop($("#status").height());
            scrollToBottom();
          }
        },
        fail: function(error){
          console.log(error);
        }
      });
    });

    let filename ="";
    let version ="";
    let prodID ="";
    let isToggle=false;
    $("#next").click(function(){
      let valid = true;
      isToggle = (document.getElementById("existingFileToggle").checked == true);
      if (isToggle){
        filename = $('#file-select option:selected').text();
      } else {
        filename = document.querySelector('#filename').value;

        //validate the file
        if (isNULL(filename)){
          //console.log("invalid file input");
          $('#status').val($('#status').val() + "Create Calendar button pressed but no file name was inputted by user." + "\n");
          valid = false;
          scrollToBottom();

        } else if (filename.lastIndexOf(".ics") < 0){
          $('#status').val($('#status').val() + "User attempted to create calendar with file name " + filename + " but file names must have extenstion .ics" + "\n");
          valid = false;
          scrollToBottom();
        } else if (filename.indexOf(".") != filename.lastIndexOf(".")){
            $('#status').val($('#status').val() + "User attempted to create calendar with file name " + filename + ", invalid filename" + "\n");
            valid = false;
            scrollToBottom();
        }
        scrollToBottom();

        //get version and get prod id
        version = document.querySelector('#version').value;
        prodID = document.querySelector('#product-id').value;
        //console.log("version: " + version);
        //console.log("productID: " + prodID);
        if (isNULL(version)){
          valid = false;
          $('#status').val($('#status').val() + "User attempted to create calendar but version number is missing" + "\n");
          scrollToBottom();
        }

        if (isNULL(prodID)){
          $('#status').val($('#status').val() + "User attempted to create calendar but product id is missing" + "\n");
          valid = false;
          scrollToBottom();
        }
      }

      if (valid == true){
        let button = $(this);
        let currentSection = button.parents(".section");
        let currentSectionIndex = currentSection.index();
        let headerSection = $('.steps li').eq(currentSectionIndex);
        currentSection.removeClass("is-active").next().addClass("is-active");
        headerSection.removeClass("is-active").next().addClass("is-active");
        if (isToggle){
          document.getElementById("createCal").value = "Add Event";
          document.getElementById("outcome").textContent = "Attempting to add new event to " + filename + "...";
          scrollToBottom();
        } else {
          document.getElementById("createCal").value = "Create Calendar";
          document.getElementById("outcome").textContent = "Attempting to create new calendar file " + filename + "...";
          scrollToBottom();
        }
      }
    });

    $("#createCal").click(function(){

      let valid = true;

      //Create Event
      let uid = $('#event-uid').val();
      if (isNULL(uid)){
        valid = false;
        $('#status').val($('#status').val() + "User attempted to create calendar but event UID is missing" + "\n");
        scrollToBottom();
      }

      let startDate = new Date($('#startDT-date').val());
      if (isNaN(startDate)){
        $('#status').val($('#status').val() + "Start Date date input is missing for event for new calendar" + "\n");
        valid = false;
        scrollToBottom();
      }

      let startDateHour = $('#startDT-hour').val();
      let startDateMin = $('#startDT-minute').val();
      let startDateSec = $('#startDT-second').val();
      let startDT_String;
      let startUTC = false;
      if (isNULL(startDateHour) || isNULL(startDateMin) || isNULL(startDateSec)){
        $('#status').val($('#status').val() + "Start date time is mssing for event for new calendar" + "\n");
        valid = false;
        scrollToBottom();
      } else if (invalidTime(startDateHour, startDateMin, startDateSec)){
        $('#status').val($('#status').val() + "Invalid start date time for event for new calendar" + "\n");
        valid = false;
        scrollToBottom();
      } else {
        startDate.setUTCHours(startDateHour, startDateMin, startDateSec);
        if (document.getElementById('startDT-isUTC').checked){
          startUTC = true;
        }
      }

      let creationDate = new Date($('#creationDT-date').val());
      if (isNaN(creationDate)){
        $('#status').val($('#status').val() + "Creation Date date input is missing for event for new calendar" + "\n");
        valid = false;
        scrollToBottom();
      }
      let creationDateHour = $('#creationDT-hour').val();
      let creationDateMin = $('#creationDT-minute').val();
      let creationDateSec = $('#creationDT-second').val();
      let creationDT_String;
      let creationUTC = false;
      if (isNULL(creationDateHour) || isNULL(creationDateMin) || isNULL(creationDateSec)){
        $('#status').val($('#status').val() + "Creation Date time is mssing for event for new calendar" + "\n");
        valid = false;
        scrollToBottom();
      } else if (invalidTime(creationDateHour, creationDateMin, creationDateSec)){
        $('#status').val($('#status').val() + "Creation Date time for event for new calendar" + "\n");
        valid = false;
        scrollToBottom();
      } else {
        creationDate.setUTCHours(creationDateHour, creationDateMin, creationDateSec);

        if (document.getElementById('creationDT-isUTC').checked){
          creationUTC = true;
        }
        //console.log(creationDate);
      }

      let summary = document.getElementById('createCal-summary').value;;
      if (isNULL(summary)){
        summary = "";
      }

      scrollToBottom();


      //ajax call
      if (valid == true){
        //version
        //prodID
        //UID
        //startDT
        let startDateString = getDateString(startDate, startUTC);
        //console.log(startDateString);

        //creationDT
        let creationDateString = getDateString(creationDate, creationUTC);
        //console.log(creationDateString);
        let versionString = quotes(version);
        let prodIDString = quotes(prodID);
        let uidString = quotes(uid);
        let startDateStringQ = quotes(startDateString);
        let creationDateStringQ = quotes(creationDateString);
        let summaryString = quotes(summary);
        let status = "SUCCESS";

        if (isToggle){
          let startDateString = getDateString(startDate, startUTC);
          //console.log(startDateString);

          //creationDT
          let creationDateString = getDateString(creationDate, creationUTC);


          //console.log("AJAX");
          $.ajax({
            type: 'get',            //Request type
            url: '/addEvent',   //The server endpoint we are connecting to
            data: {filename: filename,eventUID:uid,startDT:startDateString,creationDT:creationDateString, summary: summary},
            success: function (data) {
              //console.log(data);
              status = data;
              if (data != "SUCCESS"){
                $('#status').val($('#status').val() + "Failed to create event and add to calendar of file " + filename + " to the server due to " + data + "\n" );
                scrollToBottom();
              } else {
                $('#status').val($('#status').val() + "Successfully added Event to " + filename + " to the server" + "\n" );
                scrollToBottom();
              }
            },
            fail: function(error) {
                status = "ERROR";
                //console.log("error");
                // Non-200 return, do something with error
                $('#status').val($('#status').val() + "Failed to create calendar and upload " + filename + " to the server due to failed ajax call" + "\n");
                console.log(error);
                scrollToBottom();
            }
          });
          //gets all the files in the server (in uploads) and posts them on the file log table
          deleteAllRows('fileLogTable');
          $.ajax({
            type: 'get',
            dataType: 'json',
            url: '/initFileLog',
            success: function(data){
              //console.log("data = " + data);
              let label = document.getElementById("noFiles");
              let table = document.getElementById("fileLogTable");
              let tableRef = document.getElementById("fileLogTable").getElementsByTagName('tbody')[0]; //access <table> element

              if (data.length != 0){
                label.style.visibility = "hidden";
                for (let i = 0; i < data.length; i++){
                  //insert a row in the table t the last row
                  let row = tableRef.insertRow(tableRef.rows.length);

                  let fileName = row.insertCell(0);
                  let version = row.insertCell(1);
                  let prodID = row.insertCell(2);
                  let numEvents = row.insertCell(3);
                  let numProps = row.insertCell(4);

                  let fileNameOnly = data[i].filename.substring(10);
                  fileName.innerHTML = "<a href=/uploads/" + fileNameOnly + ">" + fileNameOnly + " </a>";
                  version.innerHTML = data[i].version;
                  prodID.innerHTML = data[i].prodID;
                  numEvents.innerHTML = data[i].numEvents;
                  numProps.innerHTML = data[i].numProps;
                }
              }
            },
            fail: function(error){
              console.log(error);
            }
          });
          deleteAllRows('calendarViewTable');
        } else {
          $.ajax({
            type: 'get',            //Request type
            url: '/uploadNewCalendar',   //The server endpoint we are connecting to
            data: {uploadFile:filename,version:versionString,prodID:prodIDString,eventUID:uidString,startDT:startDateStringQ,creationDT:creationDateStringQ,summary:summaryString},
            success: function (data) {
              //console.log(filename +","+ versionString +","+ prodIDString + ","+uidString + ","+startDateStringQ +","+ creationDateStringQ);
              //console.log(data);
              status = data;
              if (data != "SUCCESS"){
                $('#status').val($('#status').val() + "Failed to create calendar and upload " + filename + " to the server due to " + data + "\n" );
                scrollToBottom();
              } else {
                //console.log(status);
                //console.log(data);
                $('#status').val($('#status').val() + "Successfully created calendar and uploaded " + filename + " to the server" + "\n" );
                scrollToBottom();
              }
            },
            fail: function(error) {
                status = "ERROR";
                // Non-200 return, do something with error
                $('#status').val($('#status').val() + "Failed to create calendar and upload " + filename + " to the server" + "\n");
                console.log(error);
                scrollToBottom();
            }
          });

          scrollToBottom();
          //get list of file names and update the drop down menus
          if (status == "SUCCESS"){
            removeAllOptions('dynamic-select');
            removeAllOptions('file-select');
            removeAllOption('querySelect');
            $.ajax({
              type:'get',
              url: '/uploads',
              success: function(data){
                console.log(data);
                let calendarJSON;
                let calendar;
                for (let i = 0; i < data.length; i++){
                  addOption('dynamic-select',data[i]);
                  //addOption('file-select-AddEvent', data[i]);
                  addOption('file-select', data[i]);
                  addOption('querySelect', data[i]);
                }
              },
              fail: function(){
                console.log(error);
              },
            });

            //gets all the files in the server (in uploads) and posts them on the file log table
            deleteAllRows('fileLogTable');
            $.ajax({
              type: 'get',
              dataType: 'json',
              url: '/initFileLog',
              success: function(data){
                console.log("data = " + data);
                let label = document.getElementById("noFiles");
                let table = document.getElementById("fileLogTable");
                let tableRef = document.getElementById("fileLogTable").getElementsByTagName('tbody')[0]; //access <table> element

                if (data.length != 0){
                  label.style.visibility = "hidden";
                  for (let i = 0; i < data.length; i++){
                    //insert a row in the table t the last row
                    let row = tableRef.insertRow(tableRef.rows.length);

                    let fileName = row.insertCell(0);
                    let version = row.insertCell(1);
                    let prodID = row.insertCell(2);
                    let numEvents = row.insertCell(3);
                    let numProps = row.insertCell(4);

                    let fileNameOnly = data[i].filename.substring(10);
                    fileName.innerHTML = "<a href=/uploads/" + fileNameOnly + ">" + fileNameOnly + " </a>";
                    version.innerHTML = data[i].version;
                    prodID.innerHTML = data[i].prodID;
                    numEvents.innerHTML = data[i].numEvents;
                    numProps.innerHTML = data[i].numProps;
                  }
                }
              },
              fail: function(error){
                console.log(error);
              }
            });
            /*
            let button = $(this);
            let currentSection = button.parents(".section");
            let currentSectionIndex = currentSection.index();
            let headerSection = $('.steps li').eq(currentSectionIndex);
            currentSection.removeClass("is-active").next().addClass("is-active");
            headerSection.removeClass("is-active").next().addClass("is-active");*/
          }
        }
      }
      let button = $(this);
      let currentSection = button.parents(".section");
      let currentSectionIndex = currentSection.index();
      let headerSection = $('.steps li').eq(currentSectionIndex);
      currentSection.removeClass("is-active").next().addClass("is-active");
      headerSection.removeClass("is-active").next().addClass("is-active");
      scrollToBottom();
    });

    $('#addAnotherCal').click(function(){
      let button = $(this);
      let currentSection = button.parents(".section");
      let currentSectionIndex = currentSection.index();
      let headerSection = $('.steps li').eq(currentSectionIndex);
      currentSection.removeClass("is-active").next().addClass("is-active");
      headerSection.removeClass("is-active").next().addClass("is-active");
      if(currentSectionIndex===2){
        $(document).find(".form-wrapper .section").first().addClass("is-active");
        $(document).find(".steps li").first().addClass("is-active");
      }
    })

    let input = document.querySelector('#clear');
    let textarea = document.querySelector('#status');

    input.addEventListener('click', function () {
        textarea.value = '';
    }, false);


});

function addRowFileLog(filename, calendar){
  let label = document.getElementById("noFiles");
  let table = document.getElementById("fileLogTable");
  label.style.visibility = "hidden";


  let tableRef = document.getElementById("fileLogTable").getElementsByTagName('tbody')[0]; //access <table> element

  //insert a row in the table t the last row
  let row = tableRef.insertRow(tableRef.rows.length);

  //var table = document.getElementById("myTable"); //access <table> element
  //var row = table.insertRow(0);
  let fileName = row.insertCell(0);
  let version = row.insertCell(1);
  let prodID = row.insertCell(2);
  let numEvents = row.insertCell(3);
  let numProps = row.insertCell(4);

  //fileName.innerHTML = "<a href=/uploads/testCalSimpleUTC.ics > dude </a>";
  fileName.innerHTML = "<a href=/uploads/" + filename + ">" + filename + " </a>";
  version.innerHTML = calendar.version;
  prodID.innerHTML = calendar.productID;
  numEvents.innerHTML = calendar.numEvents;
  numProps.innerHTML = calendar.numProp;
}

//function to create and delete rows
function addRowCalendarView(eventNum, event){
  let tableRef = document.getElementById("calendarViewTable").getElementsByTagName('tbody')[0]; //access <table> element

  //insert a row in the table t the last row
  let row = tableRef.insertRow(tableRef.rows.length);

  //var table = document.getElementById("myTable"); //access <table> element
  //var row = table.insertRow(0);
  let eventNo = row.insertCell(0);
  let startDate = row.insertCell(1);
  let startTime = row.insertCell(2);
  let summary = row.insertCell(3);
  let props = row.insertCell(4);
  let alarms = row.insertCell(5);

  let startDateString = JSON.stringify(event.startDT);
  let startDateInsert = startDateString.substring(9, 13) + '/' + startDateString.substring(13, 15) + '/' + startDateString.substring(15, 17);
  let startTimeString = JSON.stringify(event.startDT.time);
  if (event.startDT.isUTC === true){
    let startTimeInsert = startTimeString.substring(1,3) + ':' + startTimeString.substring(3, 5) + ':' + startTimeString.substring(5,7) + ' (UTC)';
    startTime.innerHTML = startTimeInsert;
  } else {
    let startTimeInsert = startTimeString.substring(1,3) + ':' + startTimeString.substring(3, 5) + ':' + startTimeString.substring(5,7);
    startTime.innerHTML = startTimeInsert;
  }
  eventNo.innerHTML = eventNum;
  //startDate.innerHTML = JSON.stringify(event.startDT);
  startDate.innerHTML = startDateInsert;
  //startTime.innerHTML = event.startTime;
  summary.innerHTML = event.summary;
  props.innerHTML = event.numProps;
  alarms.innerHTML = event.numAlarms;
}

function deleteRow(id){
  let tableRef = document.getElementById(id).getElementsByTagName('tbody')[0]; //access <table> element

  tableRef.deleteRow(tableRef.rows.length-1);
  //document.getElementById("myTable").deleteRow(0);
}

function deleteAllRows(id){
  let tableRef = document.getElementById(id).getElementsByTagName('tbody')[0]; //access <table> element

  for (let i = tableRef.rows.length-1; i >= 0; i--){
    tableRef.deleteRow(i);
  }
}

function addOption(selectID,optionText){
  let select = document.getElementById(selectID);
  select.options[select.options.length] = new Option(optionText, '0');
}

//removes the last list element
function removeOption(){
  let select = document.getElementById("dynamic-select");
  select.options[select.options.length - 1] = null;
}

function removeAllOptions(selectID){
	let select = document.getElementById(selectID);
	select.options.length = 1;
}

function isEmpty(labelID, tableID){
  let label = document.getElementById(labelID);
  let table = document.getElementById(tableID);
  if (table.rows.length == 1){
    label.style.visibility = "visible";
  } else {
    label.style.visibility = "hidden";
  }
}

function isNULL(str){
  return (str == undefined || isBlank(str) || isEmpty(str) || isWhiteSpace(str))
}

function isEmpty(str){
  return (!str || 0 === str.length);
}

function isBlank(str){
  return (!str || /^\s*$/.test(str));
}

function isWhiteSpace(str){
  return (!str.trim());
}

function invalidTime(hour, minute, second){
  if (hour < 0 || hour > 23){
    return true;
  }

  if (minute < 0 || minute > 59){
    return true;
  }

  if (second < 0 || second > 59){
    return true;
  }
}

function getTimeString(hour, minute, second){
  let hourStr = hour;
  let minuteStr = minute;
  let secondStr = second;
  if (hour < 10){
    hourStr = "0" + hour;
  }
  if (minute < 10){
    minuteStr = "0" + minute;
  }
  if (second < 10){
    secondStr = "0" + second;
  }

  return (hourStr+""+minuteStr+""+secondStr);
}

function getDateString(date, utc){
  let date_Month_String;
  if ((date.getMonth()+1) < 10){
    date_Month_String = "0" + (date.getMonth()+1);
  } else {
    date_Month_String = date.getMonth()+1;
  }

  let date_Day_String;
  if (date.getUTCDate() < 10){
    date_Day_String = "0" + date.getUTCDate();
  } else {
    date_Day_String = date.getUTCDate();
  }

  date_TimeStr = getTimeString(date.getUTCHours(), date.getUTCMinutes(), date.getUTCSeconds() );
  if (utc){
    let string = ""+date.getFullYear() +""+ date_Month_String +""+ date_Day_String + "T"+ date_TimeStr + "Z";
    return string;
  } else {
    let string = "" +date.getFullYear() +""+ date_Month_String +""+ date_Day_String + "T"+ date_TimeStr;
    return string;
  }
}

function quotes(string){
  let newString = "\"" + string +"\"";
  return newString;
}

function showStatusSnackBar(message, type){
  //get the snack bar div
  let statusSnackBar = document.getElementsByID("snackbar-status");
  //add the show class to div
  statusSnackBar.className = "show";

  //after 3 seconds, remove the show class from div
  setTimeout(function(){
    statusSnackBar.className = statusSnackBar.className.replace("show","");
  }, 3000);
}

function saveStatusBar(){
    let textarea = document.querySelector('#status');
    let statusVal = textarea.value;
    sessionStorage.setItem('statusBar', textarea.value);
    //$('#upload').prop('disabled', false);
    //document.getElementById("fileUploadedCheckbox").checked = true;

  //console.log("Attempting to save staus bar...");


  //console.log(statusVal);
  //console.log(sessionStorage.getItem('statusBar'));
}

function saveFile(){
  if(document.getElementById("fileUploadedCheckbox").checked == true){
    let file = document.querySelector('#fileToUpload').value;
    //console.log(file);
    if (file == 'No file selected' || isNULL(file)){
      $('#status').val($('#status').val() + "No file selected to be uploaded.\n");
      $('#upload').prop('disabled', true);
      document.getElementById("fileUploadedCheckbox").checked = false;
    } else {
      let start = file.lastIndexOf('\\');
      file = file.substring(start+1, file.length);
      //get list of file names and update the drop down menus
      $.ajax({
        type:'get',
        url: '/uploads',
        success: function(data){
          let isDup = false;
          //console.log(data);
          for (let i = 0; i < data.length; i++){
            //console.log(filename);
            //console.log(data[i]);
            //console.log(filename===data[i]);
            if (file === data[i]){
              isDup = true;
              $('#status').val($('#status').val() + file + " already exists in server. Cannot upload files with the same name \n");
            }
          }
          if (isDup==false){
            $('#upload').prop('disabled', false);
            sessionStorage.setItem('file', file);
          }

        },
        fail: function(){
          console.log(error);
        },
      });
      /*
      $('#upload').prop('disabled', false);
      let start = file.lastIndexOf('\\');
      file = file.substring(start+1, file.length);
      sessionStorage.setItem('file', file);*/
      //console.log(sessionStorage.getItem('file'));
    }
  } else {
    $('#upload').prop('disabled', true);
  }
}

function toggleInput(){
  if (document.getElementById("existingFileToggle").checked == true){
    $('#file-select').prop('disabled', false);
    $('#filename').prop('disabled', true);
    $('#version').prop('disabled', true);
    $('#product-id').prop('disabled', true);
  } else {
    $('#file-select').prop('disabled', true);
    $('#filename').prop('disabled', false);
    $('#version').prop('disabled', false);
    $('#product-id').prop('disabled', false);
  }
}

function scrollToBottom(){
  $('#status').scrollTop($('#status')[0].scrollHeight);
}

function openLogin(){
  let loginForm = document.getElementById("loginForm");
  loginForm.style.visibility = "visible";
  loginForm.style.opacity = "1";
}

function closeLogin(){
  let loginForm = document.getElementById("loginForm");
  loginForm.style.visibility = "hidden";
  loginForm.style.opacity = "0";
}

function dbLogin(){
  document.getElementById('alert').style.visibility = "hidden";
  $('#username').removeClass('wrong-entry');
  $('#password').removeClass('wrong-entry');
  $('#database').removeClass('wrong-entry');

  let loginForm = document.getElementById("loginForm");

  let username = document.getElementById("username").value;
  let password = document.getElementById("password").value;
  let database = document.getElementById("database").value;

  console.log("username: " + username + " password: " + password + " database: " + database);

  let valid = true;

  if (username == ""){
    $('#username').addClass('wrong-entry');
    valid = false;
  }
  if (password == ""){
    $('#password').addClass('wrong-entry');
    valid = false;
  }

  if (database == ""){
    $('#database').addClass('wrong-entry');
    valid = false;
  }

  if (valid){
    $.ajax({
      type: 'get',
      url: '/login',
      data: {user: username, pass: password, database: database},
      success: function(data){

        if (data != true){
          //add to status panel "failed to log in to database as [userid]"
          $('#status').val($('#status').val() + "Failed to connect to database: "  + data.sqlMessage + ". Re-enter login credentials." + "\n");
          scrollToBottom();
          addNotifcation();
          //document.getElementById('alert').style.visibility = "visible";
          closeLogin();
        } else {
          //add to statis panel "successfully logged in to database as [userid]""
          console.log("connected");
          $('#status').val($('#status').val() + "successfully connected to database" + "\n");
          scrollToBottom();
          enableDatabase();
          //store the data of the login
          sessionStorage.setItem('user', username);
          sessionStorage.setItem('pass', password);
          sessionStorage.setItem('database', database);
          closeLogin();
        }
      },
      error: function(error){
        console.log(error);
      }
    });
  }


}

function dbStoreFiles(){
  let numFiles = $('#querySelect option').length;

  if (numFiles === 1){
    updateStatus("No files uploaded to store");
    addNotifcation();
  } else {
    //alert('Store all files');
    $.ajax({
      type: 'post',
      url: '/storeAllFiles',
      success: function(data){
        console.log("files stored");
      },
      error: function(error){
        console.log("error storing files");
        console.log(error);
      }
    });
  }

  $('#status').val($('#status').val() + "Store all files: ");
  dbStatus();
  $('#execBtn').prop('disabled', false);
  //showFileTable();
}

function dbClear(){
  //ajax call
  $.ajax({
    type: 'post',
    url: '/clearAllData',
    success: function(data){
      console.log("db cleared");
    },
    error: function(error){
      console.log(error);
    }
  });

  //update the html table
  $('#dbTable thead').remove();
  $('#dbTable tbody').remove();

  //add the basic table back
  let base = "<thead><tr><th> Database Query Table </th></tr></thead><tbody><tr><td> The output of the queries will be placed in this table. </td></tr></tbody>";
  $('#dbTable').append(base);

  //clear the file table HTML
  $('#fileDB thead').remove();
  $('#fileDB tbody').remove();
  let fileBase = "<thead><tr><th> Database FILE Table </th></tr></thead><tbody><tr><td> The contents of the FILE table will be displayed here once data is stored in database. </td></tr></tbody>";
  $('#fileDB').append(fileBase);

  //clear the event table HTML
  $('#eventDB thead').remove();
  $('#eventDB tbody').remove();
  let eventBase = "<thead><tr><th> Database EVENT Table </th></tr></thead><tbody><tr><td> The contents of the EVENT table will be displayed here once data is stored in database. </td></tr></tbody>";
  $('#eventDB').append(eventBase);

  //clear the alarm table HTML
  $('#alarmDB thead').remove();
  $('#alarmDB tbody').remove();
  let alarmBase = "<thead><tr><th> Database ALARM Table </th></tr></thead><tbody><tr><td> The contents of the ALAMR table will be displayed here once data is stored in database. </td></tr></tbody>";
  $('#alarmDB').append(alarmBase);

  $('#status').val($('#status').val() + "Clear all data: ");
  dbStatus();

  //deactivate the execute button
  $('#execBtn').prop('disabled', true);
}

function dbExecute(){
  //alert('Execute');
  //current date - used for query 4

  let queries = [
    'select * from EVENT order by start_time',
    'select * from EVENT, FILE where (EVENT.cal_file = FILE.cal_id AND FILE.file_Name = \'' + $('#querySelect option:selected').text() + '\')',
    'select * from EVENT as e1, EVENT as e2 where e1.start_time = e2.start_time AND e1.event_id <> e2.event_id',
    'select * from ALARM, EVENT, FILE where (ALARM.event = EVENT.event_id AND EVENT.cal_file = FILE.cal_id AND FILE.file_Name = \'' + $('#querySelect option:selected').text() + '\')',
    'select COUNT(*) AS NUM_EVENTS from EVENT, FILE where (EVENT.cal_file = FILE.cal_id AND FILE.file_Name = \'' + $('#querySelect option:selected').text() + '\')',
    'select * from FILE left outer join EVENT on FILE.cal_id = EVENT.cal_file'
  ]

  //query option
  let sel = document.getElementById('query-options');
  let opt = sel.options.selectedIndex;

  //file option
  let fileSelected = document.getElementById('querySelect').options.selectedIndex;

  let valid = true;
  if (opt == 0){
    updateStatus("No query was selected");
    addNotifcation();
    valid = false;
  } else if (fileSelected == 0){
    if (opt === 2 || opt == 4 || opt == 5){
      let query = queries[opt-1];
      let error = "No file was selected for query number " + opt;
      updateStatus(error);
      addNotifcation();
      valid = false;
    }
  }
  if (valid){
    //console.log(queries[opt-1]);
    $.ajax({
      type: 'get',
      data: { query: queries[opt-1] } ,
      url: '/executeQuery',
      success: function(data){
        if (data.length != 0){
          //console.log(data);
          createQueryTable(data);
        } else {
          updateStatus("Execute Query: Nothing to display");
          addNotifcation();
        }
      },
      error: function(error){
        console.log(error);
      }
    });
  }

}

function createQueryTable(data){
  $('#dbTable thead').remove();
  $('#dbTable tbody').remove();

  /* creating the head of the table */
  let header = "";
  header = "<thead><tr>";

  let keys = Object.keys(data[0]);
  for (let i = 0; i < keys.length; i++){
    header = header + "<th>" + keys[i] + "</th>";
  }

  header = header + "</tr></thead>";

  $('#dbTable').append(header);

  //create the body
  let body = "";
  body = "<tbody><tr>"
  for (let i = 0; i < data.length; i++){
    let values = Object.values(data[i]);
    for (let j = 0; j < values.length; j++){
      body = body + "<td>" + values[j] + "</td>";
    }
    body = body + "</tr></tbody>";
  }

  $('#dbTable').append(body);


}

function showFileTable(){
  $.ajax({
    type: 'get',
    data: { query: 'select * from FILE order by cal_id' } ,
    url: '/executeQuery',
    success: function(data){
      if (data.length != 0){
        //console.log(data);

        createFileTable(data);
        showEventTable();
      } else {
        updateStatus("Database is empty. Nothing to display.");
        addNotifcation();
        console.log(data);
      }

    }
  });
}

function createFileTable(data){
  //$(tableHeader).remove();
  //$(tableBody).remove();
  $('#fileDB thead').remove();
  $('#fileDB tbody').remove();
  /* creating the head of the table */
  let header = "";
  header = "<thead><tr>";

  let keys = Object.keys(data[0]);
  for (let i = 0; i < keys.length; i++){
    header = header + "<th>" + keys[i] + "</th>";
  }

  header = header + "</tr></thead>";

  //$(table).append(header);
  $('#fileDB').append(header);
  //create the body
  let body = "";
  body = "<tbody><tr>"
  for (let i = 0; i < data.length; i++){
    let values = Object.values(data[i]);
    for (let j = 0; j < values.length; j++){
      body = body + "<td>" + values[j] + "</td>";
    }
    body = body + "</tr></tbody>";
  }

  //$(table).append(body);
  $('#fileDB').append(body);
}

function showEventTable(){
  $.ajax({
    type: 'get',
    data: { query: 'select * from EVENT order by event_id' } ,
    url: '/executeQuery',
    success: function(data){
      if (data.length != 0){
        console.log(data);
        console.log("select success");
        createEventTable(data);
        showAlarmTable();
      } else {
        console.log("event table empty");
      }
    }
  });
}

function createEventTable(data){
  console.log("create event table");
  $('#eventDB thead').remove();
  $('#eventDB tbody').remove();
  /* creating the head of the table */
  let header = "";
  header = "<thead><tr>";

  let keys = Object.keys(data[0]);
  for (let i = 0; i < keys.length; i++){
    header = header + "<th>" + keys[i] + "</th>";
  }

  header = header + "</tr></thead>";

  $('#eventDB').append(header);
  //create the body
  let body = "";
  body = "<tbody><tr>"
  for (let i = 0; i < data.length; i++){
    let values = Object.values(data[i]);
    for (let j = 0; j < values.length; j++){
      body = body + "<td>" + values[j] + "</td>";
    }
    body = body + "</tr></tbody>";
  }

  $('#eventDB').append(body);
}

function showAlarmTable(){
  $.ajax({
    type: 'get',
    data: { query: 'select * from ALARM order by alarm_id' } ,
    url: '/executeQuery',
    success: function(data){
      if (data.length != 0){
        console.log(data);
        console.log("select success");
        createAlarmTable(data);
      } else {
        console.log("cannot create table");
        console.log(data);
      }
    }
  });
}

function createAlarmTable(data){
  console.log("create alarm table");
  $('#alarmDB thead').remove();
  $('#alarmDB tbody').remove();
  /* creating the head of the table */
  let header = "";
  header = "<thead><tr>";

  let keys = Object.keys(data[0]);
  for (let i = 0; i < keys.length; i++){
    header = header + "<th>" + keys[i] + "</th>";
  }

  header = header + "</tr></thead>";

  $('#alarmDB').append(header);
  //create the body
  let body = "";
  body = "<tbody><tr>"
  for (let i = 0; i < data.length; i++){
    let values = Object.values(data[i]);
    for (let j = 0; j < values.length; j++){
      body = body + "<td>" + values[j] + "</td>";
    }
    body = body + "</tr></tbody>";
  }

  $('#alarmDB').append(body);
}

function dbStatus(){
  $.ajax({
    type: 'get',
    url: '/displayDBStatus',
    success: function(data){
      console.log("display data:" + data);
      updateStatus(data);
      scrollToBottom();
      //addNotifcation();
    },
    error: function(error){
      console.log(error);
    }
  });
}

//display file selection if applicable
function displayFileSelect(){
  //console.log("select");
  let sel = document.getElementById('query-options');
  let opt = sel.options.selectedIndex;
  //console.log(opt);
  if (opt === 2 || opt == 4 || opt == 5){
    $('#querySelect').prop('disabled', false);
  } else {
    $('#querySelect').prop('disabled', true);
  }
}

function disableDatabase(){
  let text = "To enable the database UI, you must first login. Enter your database credentials by clicking the login button at the top right corner.";
  $('#dbInstruct').text(text)
  $('#query-options').prop('disabled', true);
  $('#querySelect').prop('disabled', true);
  $('#showTablesBtn').prop('disabled', true);
  $('#storeDataBtn').prop('disabled', true);
  $('#clearDataBtn').prop('disabled', true);
  $('#statusBtn').prop('disabled', true);
  $('#execBtn').prop('disabled', true);
}

function enableDatabase(){
  $('#dbInstruct').text("Connected to database.")
  $('#query-options').prop('disabled', false);
  $('#querySelect').prop('disabled', false);
  $('#showTablesBtn').prop('disabled', false);
  $('#storeDataBtn').prop('disabled', false);
  $('#clearDataBtn').prop('disabled', false);
  $('#statusBtn').prop('disabled', false);
  $('#execBtn').prop('disabled', false);
}

function updateStatus(text){
  $('#status').val($('#status').val() + text + "\n");
  scrollToBottom();
}

function addNotifcation(){
  let el = document.getElementById('notification');
  //console.log("add notification");
  document.getElementById('notification').style.visibility = "visible";
  //let val = $('.notification-box span').val();
  //console.log(val);
  //let newVal = val + 1;
  let count = Number(el.getAttribute('data-count')) || 0;
  el.setAttribute('data-count', count + 1);
  //console.log(newVal);
  $('.notification-box span').html(count+1);
}

function viewNotification(){
  let el = document.getElementById('notification');
  el.style.visibility = "hidden";
  el.setAttribute('data-count', 0);
  window.scrollTo({ top: 0, behavior: 'smooth' });
}

function removeError(id){
  $(id).removeClass('wrong-entry');
  //$('#password').removeClass('wrong-entry');
  //$('#password').removeClass('wrong-entry');
}

function cancelLogin(){
  sessionStorage.removeItem('user');
  sessionStorage.removeItem('pass');
  sessionStorage.removeItem('database');
  closeLogin();

}
