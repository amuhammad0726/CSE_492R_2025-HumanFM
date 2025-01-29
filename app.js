"use strict";

const express = require("express");
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