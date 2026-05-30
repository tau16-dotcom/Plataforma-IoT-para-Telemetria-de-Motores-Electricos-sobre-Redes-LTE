// ============================================================
//  CÓDIGO ORIGINAL — SIN MODIFICACIONES
// ============================================================
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <HardwareSerial.h>

const char* serverHost = "3.150.75.105";
const int   serverPort = 3000;
const char* serverPath = "/api/data";
const char* APN        = "internet.movistar.com.co";

const int INTERVALO_MS = 5000;

#define PIN_PWR    4
#define SIM_RX     16
#define SIM_TX     17
#define PIN_SCT    36
#define MUESTRAS   1000
#define FACTOR_CAL 30.0
#define UMBRAL_A   0.3

HardwareSerial simSerial(2);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// ============================================================
//  MÓDULO DE DIAGNÓSTICO LTE — struct y helpers al inicio
//  para que el compilador de Arduino los conozca antes de usarlos
// ============================================================

struct DiagLTE {
  int    rssi_raw;    // valor crudo 0-31 de AT+CSQ
  int    ber_raw;     // BER 0-7 de AT+CSQ
  int    rssi_dbm;    // RSSI convertido a dBm
  String calidad;     // Excelente / Buena / Regular / Mala / Sin señal
  String operador;    // Nombre del operador (Movistar, Claro, etc.)
  String tecnologia;  // LTE / WCDMA / GSM / Sin red
  String ip_asignada; // IP del PDP context
  long   latencia_ms; // RTT del ping HTTP al servidor
  bool   ping_ok;     // true si el servidor respondió
  String iccid;       // ICCID de la SIM
  String imei;        // IMEI del módulo
  String protocolo;   // HTTP/1.1 (según AT+HTTPPARA soportado)
};

// Extrae un valor entre dos delimitadores de una cadena
String extraerEntre(String src, String desde, String hasta) {
  int iA = src.indexOf(desde);
  if (iA == -1) return "";
  iA += desde.length();
  int iB = src.indexOf(hasta, iA);
  if (iB == -1) return src.substring(iA);
  return src.substring(iA, iB);
}

// Convierte el valor crudo de CSQ a dBm
int csqAdbm(int csq) {
  if (csq == 99) return 0;
  return -113 + (csq * 2);
}

// Clasifica la calidad según dBm
String clasificarSenal(int dbm) {
  if (dbm == 0)    return "Sin señal";
  if (dbm >= -70)  return "Excelente";
  if (dbm >= -85)  return "Buena";
  if (dbm >= -100) return "Regular";
  return "Mala";
}

// ============================================================

bool sendAT(const char* cmd, const char* expected, unsigned long timeout = 3000) {
  simSerial.println(cmd);
  unsigned long t = millis();
  String resp = "";
  while (millis() - t < timeout) {
    while (simSerial.available()) resp += (char)simSerial.read();
    if (resp.indexOf(expected) != -1) return true;
  }
  return false;
}

String sendATResponse(const char* cmd, unsigned long timeout = 5000) {
  simSerial.println(cmd);
  unsigned long t = millis();
  String resp = "";
  while (millis() - t < timeout) {
    while (simSerial.available()) resp += (char)simSerial.read();
  }
  return resp;
}

bool iniciarLTE() {
  for (int i = 0; i < 10; i++) {
    if (sendAT("AT", "OK", 2000)) break;
    delay(1000);
    if (i == 9) { Serial.println("A7608 no responde"); return false; }
  }
  Serial.println("A7608 OK");
  sendAT("ATE0", "OK");
  sendAT("AT+CMEE=2", "OK");

  if (!sendAT("AT+CPIN?", "READY", 5000)) { Serial.println("SIM no lista"); return false; }
  Serial.println("SIM lista");

  Serial.print("Esperando red");
  for (int i = 0; i < 30; i++) {
    String r = sendATResponse("AT+CREG?", 2000);
    if (r.indexOf(",1") != -1 || r.indexOf(",5") != -1) { Serial.println(" OK"); break; }
    Serial.print(".");
    delay(2000);
    if (i == 29) { Serial.println("\nSin red"); return false; }
  }

  String apnCmd = "AT+CGDCONT=1,\"IP\",\"" + String(APN) + "\"";
  sendAT(apnCmd.c_str(), "OK", 3000);
  sendAT("AT+CGACT=1,1", "OK", 10000);
  sendAT("AT+NETOPEN", "+NETOPEN: 0", 10000);
  delay(2000);

  sendAT("AT+HTTPINIT", "OK", 5000);
  Serial.println("LTE listo");
  return true;
}

bool enviarHTTP(float tAmb, float tObj, float corriente) {
  String body = "{";
  body += "\"temp_ambiente\":" + String(tAmb, 2) + ",";
  body += "\"temp_objeto\":"   + String(tObj, 2) + ",";
  body += "\"corriente\":"     + String(corriente, 3);
  body += "}";

  String url = "http://" + String(serverHost) + ":" + String(serverPort) + String(serverPath);
  String urlCmd = "AT+HTTPPARA=\"URL\",\"" + url + "\"";
  if (!sendAT(urlCmd.c_str(), "OK", 3000)) return false;

  sendAT("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK", 3000);

  String dataCmd = "AT+HTTPDATA=" + String(body.length()) + ",5000";
  simSerial.println(dataCmd);
  unsigned long t = millis();
  String prompt = "";
  while (millis() - t < 5000) {
    while (simSerial.available()) prompt += (char)simSerial.read();
    if (prompt.indexOf("DOWNLOAD") != -1) break;
  }
  simSerial.print(body);
  delay(1000);

  String postResp = sendATResponse("AT+HTTPACTION=1", 10000);

  if (postResp.indexOf("+HTTPACTION: 1,200") != -1) {
    Serial.println("OK T.amb=" + String(tAmb,1) + "C  T.obj=" + String(tObj,1) + "C  I=" + String(corriente,2) + "A");
    return true;
  }

  Serial.println("Fallo: " + postResp.substring(0, 80));
  return false;
}

float leerCorrienteRMS() {
  long offsetAcum = 0;
  for (int i = 0; i < MUESTRAS; i++) { offsetAcum += analogRead(PIN_SCT); delayMicroseconds(200); }
  float offset = offsetAcum / (float)MUESTRAS;
  long suma2 = 0;
  for (int i = 0; i < MUESTRAS; i++) {
    float m = analogRead(PIN_SCT) - offset;
    suma2 += (long)(m * m);
    delayMicroseconds(200);
  }
  float c = sqrt((float)suma2 / MUESTRAS) * (3.3f / 4095.0f) * FACTOR_CAL;
  return c < UMBRAL_A ? 0.0f : c;
}

// ============================================================
//  FUNCIONES DEL MÓDULO DE DIAGNÓSTICO LTE
// ============================================================

DiagLTE diagnosticarLTE() {
  DiagLTE d;
  d.rssi_raw    = 99;
  d.ber_raw     = 99;
  d.rssi_dbm    = 0;
  d.latencia_ms = -1;
  d.ping_ok     = false;
  d.protocolo   = "HTTP/1.1 (AT+HTTP)";

  // --- 1. RSSI y BER (AT+CSQ) ---
  String csq = sendATResponse("AT+CSQ", 3000);
  // Respuesta: +CSQ: 18,0
  String csqVal = extraerEntre(csq, "+CSQ: ", "\r");
  if (csqVal.length() > 0) {
    int coma = csqVal.indexOf(',');
    if (coma > 0) {
      d.rssi_raw = csqVal.substring(0, coma).toInt();
      d.ber_raw  = csqVal.substring(coma + 1).toInt();
    }
  }
  d.rssi_dbm = csqAdbm(d.rssi_raw);
  d.calidad  = clasificarSenal(d.rssi_dbm);

  // --- 2. Operador (AT+COPS?) ---
  String cops = sendATResponse("AT+COPS?", 5000);
  // Respuesta: +COPS: 0,0,"Movistar CO",7
  d.operador = extraerEntre(cops, "\"", "\"");
  if (d.operador.length() == 0) d.operador = "Desconocido";

  // --- 3. Tecnología de red (AT+COPS? campo Act) ---
  // El cuarto campo de +COPS: es el Act (Access Technology)
  // 0=GSM, 2=UTRAN(3G), 7=LTE, 13=NR(5G)
  int lastComa = cops.lastIndexOf(',');
  if (lastComa > 0) {
    String actStr = cops.substring(lastComa + 1);
    actStr.trim();
    int act = actStr.toInt();
    switch (act) {
      case 0:  d.tecnologia = "GSM (2G)";    break;
      case 1:  d.tecnologia = "GSM Compact"; break;
      case 2:  d.tecnologia = "UTRAN (3G)";  break;
      case 3:  d.tecnologia = "GSM EGPRS";   break;
      case 4:  d.tecnologia = "HSDPA";       break;
      case 5:  d.tecnologia = "HSUPA";       break;
      case 6:  d.tecnologia = "HSPA+";       break;
      case 7:  d.tecnologia = "LTE (4G)";    break;
      case 13: d.tecnologia = "NR (5G)";     break;
      default: d.tecnologia = "Desconocida (Act=" + actStr + ")";
    }
  } else {
    d.tecnologia = "No disponible";
  }

  // --- 4. IP asignada (AT+CGPADDR) ---
  String ip = sendATResponse("AT+CGPADDR=1", 3000);
  // Respuesta: +CGPADDR: 1,100.xx.xx.xx
  d.ip_asignada = extraerEntre(ip, "+CGPADDR: 1,", "\r");
  if (d.ip_asignada.length() == 0) d.ip_asignada = "No asignada";

  // --- 5. ICCID de la SIM (AT+ICCID) ---
  String iccid = sendATResponse("AT+ICCID", 3000);
  d.iccid = extraerEntre(iccid, "+ICCID: ", "\r");
  if (d.iccid.length() == 0) {
    // Algunos módulos responden sin prefijo
    iccid = sendATResponse("AT+CCID", 3000);
    d.iccid = extraerEntre(iccid, "+CCID: ", "\r");
  }
  if (d.iccid.length() == 0) d.iccid = "No disponible";

  // --- 6. IMEI del módulo (AT+CGSN) ---
  String imei = sendATResponse("AT+CGSN", 3000);
  // Busca 15 dígitos consecutivos
  for (int i = 0; i <= (int)imei.length() - 15; i++) {
    bool ok = true;
    for (int j = i; j < i + 15; j++) {
      if (!isDigit(imei[j])) { ok = false; break; }
    }
    if (ok) { d.imei = imei.substring(i, i + 15); break; }
  }
  if (d.imei.length() == 0) d.imei = "No disponible";

  // --- 7. Latencia HTTP (ping al servidor) ---
  // Reutiliza la sesión HTTP ya inicializada por iniciarLTE()
  // Hace un GET liviano al mismo host para medir RTT
  String pingUrl = "http://" + String(serverHost) + ":" + String(serverPort) + "/";
  String pingCmd = "AT+HTTPPARA=\"URL\",\"" + pingUrl + "\"";
  sendAT(pingCmd.c_str(), "OK", 3000);

  unsigned long t0 = millis();
  String pingResp = sendATResponse("AT+HTTPACTION=0", 12000);  // GET
  unsigned long t1 = millis();

  if (pingResp.indexOf("+HTTPACTION: 0,") != -1) {
    d.latencia_ms = (long)(t1 - t0);
    d.ping_ok     = true;
  } else {
    d.latencia_ms = -1;
    d.ping_ok     = false;
  }

  // Restaura la URL original para no romper enviarHTTP()
  String urlOrig = "http://" + String(serverHost) + ":" + String(serverPort) + String(serverPath);
  String urlCmd  = "AT+HTTPPARA=\"URL\",\"" + urlOrig + "\"";
  sendAT(urlCmd.c_str(), "OK", 3000);

  return d;
}

void imprimirDiagnostico(const DiagLTE& d) {
  Serial.println();
  Serial.println("=========================================");
  Serial.println("     DIAGNÓSTICO LTE — A7608E");
  Serial.println("=========================================");
  Serial.println("  MÓDULO / SIM");
  Serial.println("  IMEI   : " + d.imei);
  Serial.println("  ICCID  : " + d.iccid);
  Serial.println("-----------------------------------------");
  Serial.println("  RED");
  Serial.println("  Operador  : " + d.operador);
  Serial.println("  Tecnología: " + d.tecnologia);
  Serial.println("  Protocolo : " + d.protocolo);
  Serial.println("  IP local  : " + d.ip_asignada);
  Serial.println("-----------------------------------------");
  Serial.println("  SEÑAL");
  Serial.print  ("  RSSI raw  : ");  Serial.print(d.rssi_raw);  Serial.println(" (0-31, 99=sin dato)");
  Serial.print  ("  RSSI dBm  : ");
  if (d.rssi_dbm == 0) Serial.println("N/A");
  else { Serial.print(d.rssi_dbm); Serial.println(" dBm"); }
  Serial.print  ("  BER       : ");  Serial.print(d.ber_raw);   Serial.println(" (0=<0.2%, 99=sin dato)");
  Serial.println("  Calidad   : " + d.calidad);
  Serial.println("-----------------------------------------");
  Serial.println("  CONECTIVIDAD");
  Serial.print  ("  Servidor  : ");
  Serial.print(serverHost); Serial.print(":"); Serial.println(serverPort);
  if (d.ping_ok) {
    Serial.print("  Latencia  : "); Serial.print(d.latencia_ms); Serial.println(" ms");
  } else {
    Serial.println("  Latencia  : Servidor no alcanzable");
  }
  Serial.println("=========================================");
  Serial.println();
}

// ============================================================
//  SETUP Y LOOP — SETUP LLAMA AL DIAGNÓSTICO ANTES DEL LOOP
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  Wire.begin(21, 22);
  if (!mlx.begin()) { Serial.println("MLX no detectado"); while(1); }
  Serial.println("MLX OK");
  simSerial.begin(115200, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(500);
  if (!iniciarLTE()) { delay(10000); ESP.restart(); }

  // ── DIAGNÓSTICO LTE (se ejecuta una sola vez al arrancar) ──
  DiagLTE diag = diagnosticarLTE();
  imprimirDiagnostico(diag);
  // ─────────────────────────────────────────────────────────

  Serial.println("-----------------------------------------");
}

void loop() {
  float tAmb = mlx.readAmbientTempC();
  float tObj = mlx.readObjectTempC();
  if (isnan(tAmb) || isnan(tObj)) { delay(2000); return; }
  float corriente = leerCorrienteRMS();

  if (!enviarHTTP(tAmb, tObj, corriente)) {
    sendAT("AT+HTTPTERM", "OK", 3000);
    delay(1000);
    sendAT("AT+HTTPINIT", "OK", 5000);
    delay(5000);
  }

  delay(INTERVALO_MS);
}
