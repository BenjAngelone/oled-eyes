#!/usr/bin/env python3
# ============================================================================
# bridge.py - Puente entre Claude Code (ccusage) y el ESP32 "ojos"
#
# Flujo:
#   Claude Code -> ~/.claude/projects/*.jsonl
#               -> npx ccusage@latest blocks --active --json
#               -> este script parsea y calcula porcentaje
#               -> POST http://esp32-eyes.local/update
#               -> ESP32 muestra los ojos
#
# Requiere: pip install requests
# ============================================================================

import json
import subprocess
import time
import sys
from datetime import datetime, timezone

import requests

# ============================================================================
# CONFIGURACION (ajustar aca)
# ============================================================================
ESP32_URL = "http://esp32-eyes.local/update"

INTERVAL_SECONDS = 30       # cada cuanto se repite el ciclo
HTTP_TIMEOUT = 5            # segundos de timeout para el POST
HTTP_RETRIES = 2            # reintentos por ciclo si falla el POST

# Limite de tokens del bloque de 5 horas. NO se puede leer programaticamente:
# depende del plan del usuario (Pro, Max 5x, Max 20x, etc). Calibrar
# comparando este valor contra lo que muestra "/usage" dentro de Claude Code
# hasta que el porcentaje coincida.
TOKEN_LIMIT = 31000000

CCUSAGE_CMD = ["npx", "ccusage@latest", "blocks", "--active", "--json"]


# ============================================================================
# OBTENER BLOQUE ACTIVO DE CCUSAGE
# ============================================================================
def obtener_bloque_activo():
    """
    Ejecuta ccusage y devuelve el bloque activo (dict) o None si no hay
    ninguno en curso.
    """
    try:
        resultado = subprocess.run(
            CCUSAGE_CMD,
            capture_output=True,
            text=True,
            timeout=30,
            shell=(sys.platform == "win32"),
        )
    except subprocess.TimeoutExpired:
        print("[bridge] Timeout ejecutando ccusage")
        return None
    except FileNotFoundError:
        print("[bridge] No se encontro 'npx'. Verifica que Node.js este instalado.")
        return None

    if resultado.returncode != 0:
        print(f"[bridge] ccusage devolvio codigo {resultado.returncode}: {resultado.stderr.strip()}")
        return None

    try:
        data = json.loads(resultado.stdout)
    except json.JSONDecodeError:
        print("[bridge] Salida de ccusage no es JSON valido")
        return None

    bloques = data.get("blocks", [])
    if not bloques:
        return None

    # Buscamos el bloque marcado como activo; si no viene el flag, tomamos el ultimo
    for b in bloques:
        if b.get("isActive"):
            return b
    return bloques[-1]


# ============================================================================
# CALCULAR PAYLOAD A PARTIR DEL BLOQUE
# ============================================================================
def calcular_payload(bloque):
    """
    A partir del bloque activo de ccusage, arma el JSON que espera el ESP32.
    Devuelve None si el bloque no tiene los datos minimos necesarios.
    """
    token_counts = bloque.get("tokenCounts", {})
    input_tokens = token_counts.get("inputTokens", 0)
    output_tokens = token_counts.get("outputTokens", 0)
    cache_creation = token_counts.get("cacheCreationInputTokens", 0)
    cache_read = token_counts.get("cacheReadInputTokens", 0)

    total_tokens = input_tokens + output_tokens + cache_creation + cache_read

    end_time_str = bloque.get("endTime")
    if not end_time_str:
        print("[bridge] Bloque sin endTime, se descarta")
        return None

    # endTime viene en formato ISO 8601 (ej: "2025-08-22T21:00:00.000Z")
    end_dt = datetime.fromisoformat(end_time_str.replace("Z", "+00:00"))
    reset_epoch = int(end_dt.timestamp())

    percent = round(min(100.0, (total_tokens / TOKEN_LIMIT) * 100.0), 1)

    payload = {
        "percent": percent,
        "reset_epoch": reset_epoch,
        "tokens_used": total_tokens,
        "tokens_limit": TOKEN_LIMIT,
    }
    return payload


# ============================================================================
# ENVIAR AL ESP32 (con reintentos, sin cortar el script si falla)
# ============================================================================
def enviar_al_esp32(payload):
    for intento in range(1, HTTP_RETRIES + 2):
        try:
            resp = requests.post(ESP32_URL, json=payload, timeout=HTTP_TIMEOUT)
            if resp.status_code == 200:
                print(f"[bridge] OK -> {payload['percent']}% (intento {intento})")
                return True
            else:
                print(f"[bridge] ESP32 respondio {resp.status_code}: {resp.text}")
        except requests.exceptions.RequestException as e:
            print(f"[bridge] Error de conexion (intento {intento}/{HTTP_RETRIES + 1}): {e}")
        if intento <= HTTP_RETRIES:
            time.sleep(1)
    print("[bridge] No se pudo contactar al ESP32 en este ciclo, se sigue igual.")
    return False


# ============================================================================
# LOOP PRINCIPAL
# ============================================================================
def main():
    print(f"[bridge] Iniciando. Enviando a {ESP32_URL} cada {INTERVAL_SECONDS}s")
    print(f"[bridge] TOKEN_LIMIT configurado en {TOKEN_LIMIT} (calibrar contra /usage)")

    while True:
        inicio = time.time()

        bloque = obtener_bloque_activo()
        if bloque is None:
            # Sin bloque activo: no mandamos nada y dejamos que el ESP32
            # entre solo en estado "Sin datos" tras 10 minutos sin POST.
            print("[bridge] Sin bloque activo, no se envia nada este ciclo.")
        else:
            payload = calcular_payload(bloque)
            if payload is not None:
                enviar_al_esp32(payload)

        elapsed = time.time() - inicio
        time.sleep(max(0, INTERVAL_SECONDS - elapsed))


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n[bridge] Detenido por el usuario.")
