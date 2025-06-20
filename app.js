"use strict";

const express = require("express");
const net = require('net');
const app = express();
const multer = require("multer");
const sqlite3 = require("sqlite3");
const sqlite = require("sqlite");
const USER_ERROR = 400;
const SERVER_ERROR = 500;
const PORT_NUM = 8000;

app.use(express.urlencoded({extended: true}));
app.use(express.json());
app.use(multer().none());

// // Temporary global variables to handle incoming C++ server data.
// let activeClasses = [];
// let soundLevels = [];

// // Create a new socket and connect to C++ server running on port 4000.
// const client = new net.Socket();
// client.connect(4000, '127.0.0.1', () => {
//     console.log('Connected to C++ server!');
// });

// // Handle incoming data from the server.
// client.on('data', (data) => {
//   let recClasses = [];
//   let recSounds = [];

//   // Read data related to active classes.
//   for (let i = 0; i < 20; i++) {
//     recClasses.push(data.readInt32LE(4 + i * 4));
//   }

//   // Read data related to sound levels for each class.
//   let offset = 4 + 20 * 4;
//   for (let i = 0; i < 20; i++) {
//     recSounds.push(data.readFloatLE(offset + i * 4));
//   }

//   activeClasses = recClasses;
//   soundLevels = recSounds;

//   console.log("Received activeClasses:", recClasses);
//   console.log("Received soundLevels:", recSounds);
// });

// // Temporary function to handle sending data to the server.
// function sendClientData() {
//   // Prepare buffer for sending updated data.
//   let buffer = Buffer.alloc(164);

//   // Writing our active classes to the buffer and introducing
//   // variable sound data.
//   for (let i = 0; i < 20; i++) {
//     buffer.writeInt32LE(activeClasses[i], 4 + i * 4);
//   }

//   // Only modifying the active classes.
//   let offset = 4 + 20 * 4;
//   for (let i = 0; i < 20; i++) {
//     if (activeClasses[i] == 1) {
//       let randFloat = Math.round(Math.random() * 100) / 100;
//       buffer.writeFloatLE(parseFloat(randFloat), offset + i * 4);
//     } else {
//       buffer.writeFloatLE(soundLevels[i], offset + i * 4);
//     }
//   }

//   // Simulate termination condition.
//   if (Math.random() < 0.1) {
//       buffer.writeInt32LE(1, 0);
//       console.log("Sending termination signal...");
//       return -1;
//   }

//   client.write(buffer);
//   return 0;
// }

// // Call the send function under a time interval.
// let intervalId = setInterval(() => {
//   console.log("Attempting to send server data.");
//   let termStatus = sendClientData();
//   if (termStatus == -1) {
//     clearInterval(intervalId);
//   }
// }, 10000);

// // Handle errors.
// client.on("error", (err) => {
//   console.error("Socket error:", err.message);
// });

// // Handle connection close.
// client.on('close', () => {
//     console.log('Connection closed.');
// });

// This function extracts data related to the classes from the database.
app.get("/HumanFM/classes", async function(req, res) {
  try {
    let classesTable = await getDBConnection();
    let randId = Math.floor(Math.random() * 20);
    let allClasses = await classesTable.all(
      "SELECT * FROM classes WHERE id = ?",
      randId
    );

    await classesTable.close();
    res.json(allClasses);
  } catch (err) {
    res.status(SERVER_ERROR).send("An error occurred on the server. Try again later");
  }
});

// This function updates the state of the database to account for changes in audio levels.
app.post("/HumanFM/classes/update", async function(req, res) {
  try {
    if (req.body) {
      let classesTable = await getDBConnection();
      let classId = req.body.id;
      let classLevel = req.body.level;

      await classesTable.run("UPDATE classes SET level = ? WHERE id = ?", [classLevel, classId]);
      await classesTable.close();
      res.type("text").send("success");
    } else {
      res.status(USER_ERROR).send("An error occurred extracting form data. Try again later");
    }
  } catch (err) {

  }
});

// This function presents the detected audio from the ML model based on the most
// recent live audio chuck from the user.
app.post("/HumanFM/classify", (req, res) => {
  try {
    if (req.body) {
      console.log("Received class level: " + req.body.class);
      res.status(200).send("success");
    } else {
      res.status(400).send("error");
    }
  } catch (err) {

  }
})

/**
 * Establishes a database connection to the database and returns the database object.
 * Any errors that occur should be caught in the function that calls this one.
 * @returns {Object} - The database object for the connection.
 */
async function getDBConnection() {
  const db = await sqlite.open({
    filename: 'humanfm.db',
    driver: sqlite3.Database
  });
  return db;
}

app.use(express.static('public'));
const PORT = process.env.PORT || PORT_NUM;
app.listen(PORT);
