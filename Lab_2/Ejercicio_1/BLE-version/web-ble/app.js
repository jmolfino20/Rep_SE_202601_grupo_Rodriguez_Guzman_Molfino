const SERVICE_UUID = 0x00ff;
const CHARACTERISTIC_UUID = 0xff01;

let characteristic = null;
let keys = {};
let currentCmd = 0;
let lastCmd = -1;

const statusEl = document.getElementById("status");
const connectBtn = document.getElementById("connectBtn");
const joystick = document.getElementById("joystick");

if (connectBtn) connectBtn.onclick = connectBLE;

// BLE
async function connectBLE() {
  try {
    const device = await navigator.bluetooth.requestDevice({
      acceptAllDevices: true,
      optionalServices: [SERVICE_UUID],
    });

    const server = await device.gatt.connect();
    const service = await server.getPrimaryService(SERVICE_UUID);
    characteristic = await service.getCharacteristic(CHARACTERISTIC_UUID);

    statusEl.innerText = "Conectado a " + (device.name || "ESP32");
  } catch (err) {
    console.error(err);
  }
}

function send(cmd) {
  if (!characteristic) return;

  const data = new Uint8Array([cmd]);
  characteristic.writeValueWithoutResponse(data);
}

function updateAndSend() {
  const keyboardCmd = getKeyboardDirection();
  const cmd = keyboardCmd !== 0 ? keyboardCmd : currentCmd;

  if (cmd !== lastCmd) {
    send(cmd);
    lastCmd = cmd;
    console.log("Enviado:", cmd);
  }
}

// Teclado
window.addEventListener("keydown", (e) => {
  keys[e.key] = true;
  updateAndSend();
});

window.addEventListener("keyup", (e) => {
  keys[e.key] = false;
  updateAndSend(); // ← importante para STOP
});

// Determinar dirección desde teclado
function getKeyboardDirection() {
  const up = keys["w"] || keys["ArrowUp"];
  const down = keys["s"] || keys["ArrowDown"];
  const left = keys["a"] || keys["ArrowLeft"];
  const right = keys["d"] || keys["ArrowRight"];

  if (up && left) return 2;
  if (up && right) return 4;
  if (down && left) return 8;
  if (down && right) return 6;

  if (up) return 3;
  if (down) return 7;
  if (left) return 1;
  if (right) return 5;

  return 0;
}

// Joystick (mouse/touch)
joystick.addEventListener("pointerdown", (e) => {
  joystick.setPointerCapture(e.pointerId);
});

joystick.addEventListener("pointermove", (e) => {
  if (e.buttons === 0) return;

  const rect = joystick.getBoundingClientRect();

  const x = e.clientX - rect.left - rect.width / 2;
  const y = e.clientY - rect.top - rect.height / 2;

  const angle = Math.atan2(y, x);

  currentCmd = angleToCommand(angle);

  updateAndSend();
});

joystick.addEventListener("pointerup", () => {
  currentCmd = 0;
  updateAndSend(); // ← STOP
});

// Convertir ángulo → comando
function angleToCommand(angle) {
  const deg = angle * (180 / Math.PI);

  if (deg >= -22.5 && deg < 22.5) return 5;       // Der
  if (deg >= 22.5 && deg < 67.5) return 6;        // Der-Dwn
  if (deg >= 67.5 && deg < 112.5) return 7;       // Dwn
  if (deg >= 112.5 && deg < 157.5) return 8;      // Dwn-Izq
  if (deg >= 157.5 || deg < -157.5) return 1;     // Izq
  if (deg >= -157.5 && deg < -112.5) return 2;    // Izq-Up
  if (deg >= -112.5 && deg < -67.5) return 3;     // Up
  if (deg >= -67.5 && deg < -22.5) return 4;      // Up-Der

  return 0;
}