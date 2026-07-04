const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline');
const sqlite3 = require('sqlite3').verbose();
const WebSocket = require('ws');

const PUERTO_SERIAL = '/dev/ttyACM0'
const BAUDIOS = 57600;
const DB_FILE = 'atenea_flight_data.db';

let datos_actuales = {
    pitch: 0.0,
    roll: 0.0,
    altitud: 0.0,
    velocidad: 0.0,
    servo_pitch: 90
};

const db = new sqlite3.Database(DB_FILE, (err) => {
    if (err) console.error('[-] Error al abrir base de datos', err.message);
    else console.log('[DB] Base de datos SQLite iniciada correctamente.');
});

db.serialize(() => {
    db.run('PRAGMA journal_mode = WAL;');
    db.run('PRAGMA synchronous = NORMAL;');
    
    db.run(`CREATE TABLE IF NOT EXISTS telemetria (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
        pitch REAL,
        roll REAL,
        altitud REAL,
        velocidad REAL
    )`);
});

let loteGuardado = [];
const TAMANO_LOTE = 10;

function guardarEnBaseDeDatos() {
    if (loteGuardado.length === 0) return;
    
    const datosProcesar = [...loteGuardado];
    loteGuardado = [];

    db.serialize(() => {
        db.run("BEGIN TRANSACTION");
        const stmt = db.prepare("INSERT INTO telemetria (pitch, roll, altitud, velocidad) VALUES (?, ?, ?, ?)");
        for (let dato of datosProcesar) {
            stmt.run(dato.pitch, dato.roll, dato.altitud, dato.velocidad);
        }
        stmt.finalize();
        db.run("COMMIT");
    });
}

const wss = new WebSocket.Server({ port: 8765 }, () => {
    console.log('[WEB] Servidor de telemetría online en ws://localhost:8765');
});

wss.on('connection', (ws) => {
    console.log('[WEB] ¡Nuevo Dashboard conectado!');
});

setInterval(() => {
    const jsonDatos = JSON.stringify(datos_actuales);
    wss.clients.forEach((cliente) => {
        if (cliente.readyState === WebSocket.OPEN) {
            cliente.send(jsonDatos);
        }
    });
}, 50);

console.log(`[USB] Intentando conectar a ${PUERTO_SERIAL} a ${BAUDIOS} baudios...`);

const puerto = new SerialPort({ path: PUERTO_SERIAL, baudRate: BAUDIOS }, (err) => {
    if (err) {
        console.error(`[USB ERROR] No se pudo abrir el puerto: ${err.message}`);
        console.log('Asegúrate de cerrar VS Code / Arduino IDE y de tener permisos dialout.');
    }
});

const parser = puerto.pipe(new ReadlineParser({ delimiter: '\n' }));

puerto.on('open', () => {
    console.log('[USB] ¡Conectado al cohete Atenea! Escuchando datos...');
});

parser.on('data', (linea) => {
    const valores = linea.trim().split(',');
    
    if (valores.length >= 2) {
        const pitch = parseFloat(valores[0]);
        const roll = parseFloat(valores[1]);
        const altitud = valores.length > 2 ? parseFloat(valores[2]) : 0.0;
        const velocidad = valores.length > 3 ? parseFloat(valores[3]) : 0.0;
        const servo_pitch = valores.length > 4 ? parseInt(valores[4], 10) : 90;

        if (!isNaN(pitch) && !isNaN(roll)) {
            datos_actuales = { pitch, roll, altitud, velocidad, servo_pitch };

            loteGuardado.push(datos_actuales);
            if (loteGuardado.length >= TAMANO_LOTE) {
                guardarEnBaseDeDatos();
            }
        }
    }
});