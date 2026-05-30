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
//  MÓDULO DE DIAGNÓSTICO LTE
// ============================================================

struct DiagLTE {
  int    rssi_raw;     // valor crudo 0-31 de AT+CSQ
  int    rssi_dbm;     // RSSI convertido a dBm
  String calidad;      // Excelente / Buena / Regular / Mala / Sin señal
  String operador;     // Nombre del operador (Movistar, Claro, etc.)
  String tecnologia;   // LTE / WCDMA / GSM / Sin red  (AT+COPS?)
  String ip_asignada;  // IP del PDP context
  long   latencia_ms;  // RTT del ping HTTP al servidor
  bool   ping_ok;      // true si el servidor respondió
  String iccid;        // ICCID de la SIM
  String imei;         // IMEI del módulo
  String protocolo;    // HTTP/1.1 (según AT+HTTPPARA soportado)
  String estado_modulo;// Estado del módulo vía AT+CPAS
  // --- campos extendidos (AT+CPSI) ---
  String cpsi_raw;     // Respuesta completa de AT+CPSI
  String banda;        // Banda de frecuencia (AT+CBAND)
  String modulacion;   // QPSK / 16-QAM / 64-QAM (AT+CAATT)
  String rsrp;         // RSRP en dBm (de AT+CPSI)
  String rsrq;         // RSRQ en dB  (de AT+CPSI)
  String sinr;         // SINR en dB  (de AT+CPSI)
};

String extraerEntre(String src, String desde, String hasta) {
  int iA = src.indexOf(desde);
  if (iA == -1) return "";
  iA += desde.length();
  int iB = src.indexOf(hasta, iA);
  if (iB == -1) return src.substring(iA);
  return src.substring(iA, iB);
}

int csqAdbm(int csq) {
  if (csq == 99) return 0;
  return -113 + (csq * 2);
}

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
  d.rssi_dbm    = 0;
  d.latencia_ms = -1;
  d.ping_ok     = false;
  d.protocolo   = "HTTP/1.1 (AT+HTTP)";

  // --- 1. Estado del módulo (AT+CPAS) ---
  // Respuesta: +CPAS: <estado>
  // 0=Listo, 2=Desconocido, 3=Llamada entrante, 4=En llamada
  String cpas = sendATResponse("AT+CPAS", 3000);
  String cpasVal = extraerEntre(cpas, "+CPAS: ", "\r");
  cpasVal.trim();
  int cpasInt = cpasVal.toInt();
  switch (cpasInt) {
    case 0: d.estado_modulo = "Listo (0)";            break;
    case 2: d.estado_modulo = "Desconocido (2)";      break;
    case 3: d.estado_modulo = "Llamada entrante (3)"; break;
    case 4: d.estado_modulo = "En llamada (4)";       break;
    default:
      d.estado_modulo = cpasVal.length() > 0
        ? "Código " + cpasVal
        : "No disponible";
  }

  // --- 2. RSSI (AT+CSQ) ---
  // Respuesta: +CSQ: <rssi>,<ber>
  // rssi: 0-31 (99 = sin dato). Fórmula: dBm = -113 + (rssi * 2)
  String csq = sendATResponse("AT+CSQ", 3000);
  String csqVal = extraerEntre(csq, "+CSQ: ", "\r");
  if (csqVal.length() > 0) {
    int coma = csqVal.indexOf(',');
    if (coma > 0) d.rssi_raw = csqVal.substring(0, coma).toInt();
  }
  d.rssi_dbm = csqAdbm(d.rssi_raw);
  d.calidad  = clasificarSenal(d.rssi_dbm);

  // --- 3. Operador y tecnología (AT+COPS?) ---
  // Respuesta: +COPS: <modo>,<formato>,"<operador>",<Act>
  // Act: 0=GSM, 2=UTRAN(3G), 7=LTE, 13=NR(5G)
  String cops = sendATResponse("AT+COPS?", 5000);
  d.operador = extraerEntre(cops, "\"", "\"");
  if (d.operador.length() == 0) d.operador = "Desconocido";

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
  d.ip_asignada = extraerEntre(ip, "+CGPADDR: 1,", "\r");
  if (d.ip_asignada.length() == 0) d.ip_asignada = "No asignada";

  // --- 5. ICCID de la SIM (AT+ICCID) ---
  String iccid = sendATResponse("AT+ICCID", 3000);
  d.iccid = extraerEntre(iccid, "+ICCID: ", "\r");
  if (d.iccid.length() == 0) {
    iccid = sendATResponse("AT+CCID", 3000);
    d.iccid = extraerEntre(iccid, "+CCID: ", "\r");
  }
  if (d.iccid.length() == 0) d.iccid = "No disponible";

  // --- 6. IMEI del módulo (AT+CGSN) ---
  String imei = sendATResponse("AT+CGSN", 3000);
  for (int i = 0; i <= (int)imei.length() - 15; i++) {
    bool ok = true;
    for (int j = i; j < i + 15; j++) {
      if (!isDigit(imei[j])) { ok = false; break; }
    }
    if (ok) { d.imei = imei.substring(i, i + 15); break; }
  }
  if (d.imei.length() == 0) d.imei = "No disponible";

  // --- 7. Info extendida LTE (AT+CPSI) ---
  // Respuesta: +CPSI: LTE,Online,730-02,0x...,<RxLev>,<TxPwr>,EUTRAN-BAND28,<EARFCN>,<RSRP>,<RSRQ>,<SINR>,<CQI>
  // RSRP: potencia señal referencia (dBm), mejor cuanto más cerca de 0
  // RSRQ: calidad señal referencia (dB),  mejor cuanto más cerca de 0
  // SINR: relación señal/ruido+interferencia (dB), mayor = mejor
  String cpsi = sendATResponse("AT+CPSI", 5000);
  if (cpsi.indexOf("+CPSI:") != -1) {
    d.cpsi_raw = extraerEntre(cpsi, "+CPSI: ", "\r");
    if (d.cpsi_raw.length() == 0)
      d.cpsi_raw = extraerEntre(cpsi, "+CPSI:", "\r");

    // Separar campos por coma
    // Campo índice: 0=tech, 1=estado, 2=MCC-MNC, 3=LAC/CellID,
    //               4=RxLev, 5=TxPwr, 6=Banda, 7=EARFCN,
    //               8=?, 9=?, 10=RSRP, 11=RSRQ, 12=SINR, 13=CQI
    String tmp = d.cpsi_raw;
    String campos[14];
    int idx = 0;
    while (tmp.length() > 0 && idx < 14) {
      int c = tmp.indexOf(',');
      if (c == -1) { campos[idx++] = tmp; break; }
      campos[idx++] = tmp.substring(0, c);
      tmp = tmp.substring(c + 1);
    }
    if (idx >= 7)  { campos[6].trim(); if (campos[6].length() > 0) d.banda = campos[6]; }
    if (idx >= 11) { campos[10].trim(); if (campos[10].length() > 0) d.rsrp = campos[10] + " dBm"; }
    if (idx >= 12) { campos[11].trim(); if (campos[11].length() > 0) d.rsrq = campos[11] + " dB";  }
    if (idx >= 13) { campos[12].trim(); if (campos[12].length() > 0) d.sinr = campos[12] + " dB";  }
  }
  if (d.cpsi_raw.length() == 0) d.cpsi_raw  = "No disponible";
  if (d.banda.length()    == 0) d.banda      = "No disponible";
  if (d.rsrp.length()     == 0) d.rsrp       = "No disponible";
  if (d.rsrq.length()     == 0) d.rsrq       = "No disponible";
  if (d.sinr.length()     == 0) d.sinr       = "No disponible";

  // --- 8. Banda de frecuencia (AT+CBAND) ---
  // Respuesta: +CBAND: B28  (o similar según firmware)
  // Confirma la banda LTE activa (complementa AT+CPSI)
  String cband = sendATResponse("AT+CBAND", 3000);
  String cbandVal = extraerEntre(cband, "+CBAND: ", "\r");
  if (cbandVal.length() > 0) {
    cbandVal.trim();
    // Si CPSI ya entregó banda, mostrar ambas para comparar
    if (d.banda != "No disponible" && d.banda != cbandVal)
      d.banda = d.banda + " / CBAND: " + cbandVal;
    else if (d.banda == "No disponible")
      d.banda = cbandVal;
  }

  // --- 9. Modulación activa (AT+CAATT) ---
  // Respuesta: +CAATT: <DL_mod>,<UL_mod>
  // Valores posibles: 0=QPSK, 2=16-QAM, 4=64-QAM, 6=256-QAM
  // No todos los firmwares del A7608E lo soportan
  String caatt = sendATResponse("AT+CAATT", 3000);
  if (caatt.indexOf("+CAATT:") != -1) {
    String caattVal = extraerEntre(caatt, "+CAATT: ", "\r");
    if (caattVal.length() == 0)
      caattVal = extraerEntre(caatt, "+CAATT:", "\r");
    caattVal.trim();
    int coma2 = caattVal.indexOf(',');
    String dlStr = coma2 > 0 ? caattVal.substring(0, coma2) : caattVal;
    String ulStr = coma2 > 0 ? caattVal.substring(coma2 + 1) : "";
    dlStr.trim(); ulStr.trim();

    auto modStr = [](String s) -> String {
      int v = s.toInt();
      switch (v) {
        case 0: return "QPSK";
        case 2: return "16-QAM";
        case 4: return "64-QAM";
        case 6: return "256-QAM";
        default: return "(" + s + ")";
      }
    };

    d.modulacion = "DL: " + modStr(dlStr);
    if (ulStr.length() > 0) d.modulacion += "  UL: " + modStr(ulStr);
  } else {
    d.modulacion = "No soportado por firmware";
  }

  // --- 10. Latencia HTTP (ping al servidor) ---
  String pingUrl = "http://" + String(serverHost) + ":" + String(serverPort) + "/";
  String pingCmd = "AT+HTTPPARA=\"URL\",\"" + pingUrl + "\"";
  sendAT(pingCmd.c_str(), "OK", 3000);

  unsigned long t0 = millis();
  String pingResp = sendATResponse("AT+HTTPACTION=0", 12000);
  unsigned long t1 = millis();

  if (pingResp.indexOf("+HTTPACTION: 0,") != -1) {
    d.latencia_ms = (long)(t1 - t0);
    d.ping_ok     = true;
  } else {
    d.latencia_ms = -1;
    d.ping_ok     = false;
  }

  // Restaura la URL original
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
  Serial.println("  IMEI        : " + d.imei);
  Serial.println("  ICCID       : " + d.iccid);
  Serial.println("  Estado (CPAS): " + d.estado_modulo);
  Serial.println("-----------------------------------------");
  Serial.println("  RED");
  Serial.println("  Operador    : " + d.operador);
  Serial.println("  Tecnología  : " + d.tecnologia);
  Serial.println("  Protocolo   : " + d.protocolo);
  Serial.println("  IP local    : " + d.ip_asignada);
  Serial.println("-----------------------------------------");
  Serial.println("  SEÑAL");
  Serial.print  ("  RSSI raw    : ");
  Serial.print(d.rssi_raw);
  Serial.println(" (0-31, 99=sin dato)  [AT+CSQ]");
  Serial.print  ("  RSSI dBm    : ");
  if (d.rssi_dbm == 0) Serial.println("N/A");
  else { Serial.print(d.rssi_dbm); Serial.println(" dBm"); }
  Serial.println("  Calidad     : " + d.calidad);
  Serial.println("-----------------------------------------");
  Serial.println("  SEÑAL LTE EXTENDIDA  [AT+CPSI]");
  Serial.println("  Banda       : " + d.banda);
  Serial.println("  RSRP        : " + d.rsrp + "  (ref: > -80 excelente)");
  Serial.println("  RSRQ        : " + d.rsrq + "  (ref: > -10 buena)");
  Serial.println("  SINR        : " + d.sinr + "  (ref: > 20 excelente)");
  Serial.println("  Raw CPSI    : " + d.cpsi_raw);
  Serial.println("-----------------------------------------");
  Serial.println("  MODULACIÓN  [AT+CAATT]");
  Serial.println("  Esquema     : " + d.modulacion);
  Serial.println("-----------------------------------------");
  Serial.println("  CONECTIVIDAD");
  Serial.print  ("  Servidor    : ");
  Serial.print(serverHost); Serial.print(":"); Serial.println(serverPort);
  if (d.ping_ok) {
    Serial.print("  Latencia    : "); Serial.print(d.latencia_ms); Serial.println(" ms");
  } else {
    Serial.println("  Latencia    : Servidor no alcanzable");
  }
  Serial.println("=========================================");
  Serial.println();
}

// ============================================================
//  SETUP Y LOOP
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
