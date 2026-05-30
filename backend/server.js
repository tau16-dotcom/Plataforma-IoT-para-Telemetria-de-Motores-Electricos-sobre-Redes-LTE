const express = require('express');
const cors = require('cors');
const Database = require('better-sqlite3');
const path = require('path');

const app = express();
const db = new Database('/home/ubuntu/iot-monitor/sensores.db');
const PORT = 3000;

db.exec(`
  CREATE TABLE IF NOT EXISTS lecturas (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    temp_ambiente REAL,
    temp_objeto   REAL,
    corriente     REAL,
    timestamp     TEXT
  )
`);

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

app.post('/api/data', (req, res) => {
  const { temp_ambiente, temp_objeto, corriente } = req.body;

  if (temp_ambiente == null || temp_objeto == null) {
    return res.status(400).json({ error: 'Faltan datos' });
  }

  const timestamp = new Date().toLocaleString('sv-SE', { timeZone: 'America/Bogota' });

  const stmt = db.prepare(
    'INSERT INTO lecturas (temp_ambiente, temp_objeto, corriente, timestamp) VALUES (?, ?, ?, ?)'
  );
  stmt.run(temp_ambiente, temp_objeto, corriente ?? null, timestamp);

  console.log(`[${timestamp}] T.amb=${temp_ambiente}C  T.obj=${temp_objeto}C  I=${corriente ?? 'N/A'}A`);
  res.json({ ok: true });
});

app.get('/api/latest', (req, res) => {
  const row = db.prepare(
    'SELECT * FROM lecturas ORDER BY id DESC LIMIT 1'
  ).get();
  res.json(row ?? {});
});

app.get('/api/history', (req, res) => {
  const n = parseInt(req.query.n) || 50;
  const rows = db.prepare(
    'SELECT * FROM lecturas ORDER BY id DESC LIMIT ?'
  ).all(n);
  res.json(rows.reverse());
});

app.listen(PORT, '0.0.0.0', () => {
  console.log(`Servidor corriendo en puerto ${PORT}`);
});
