import os
import re
from io import StringIO

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# ============================================================
# Pega aquí TODA la salida del monitor serial
# ============================================================
datos_txt = """
==== INICIO DE PRUEBA ====
PWM fijo: 120
Posicion inicial: 298
t_ms,pwm,posicion,delta,direccion,sw_abajo,sw_arriba
359,120,293,-5,POT_MENOR,OFF,OFF
409,120,291,-7,POT_MENOR,OFF,OFF
459,120,288,-10,POT_MENOR,OFF,OFF
509,120,286,-12,POT_MENOR,OFF,OFF
559,120,283,-15,POT_MENOR,OFF,OFF
609,120,279,-19,POT_MENOR,OFF,OFF
659,120,276,-22,POT_MENOR,OFF,OFF
709,120,272,-26,POT_MENOR,OFF,OFF
759,120,269,-29,POT_MENOR,OFF,OFF
809,120,267,-31,POT_MENOR,OFF,OFF
859,120,263,-35,POT_MENOR,OFF,OFF
909,120,261,-37,POT_MENOR,OFF,OFF
959,120,256,-42,POT_MENOR,OFF,OFF
1009,120,254,-44,POT_MENOR,OFF,OFF
1059,120,250,-48,POT_MENOR,OFF,OFF
1109,120,245,-53,POT_MENOR,OFF,OFF
1159,120,241,-57,POT_MENOR,OFF,OFF
1209,120,237,-61,POT_MENOR,OFF,OFF
1259,120,231,-67,POT_MENOR,OFF,OFF
1309,120,226,-72,POT_MENOR,OFF,OFF
1359,120,221,-77,POT_MENOR,OFF,OFF
1409,120,217,-81,POT_MENOR,OFF,OFF
1459,120,211,-87,POT_MENOR,OFF,OFF
1509,120,206,-92,POT_MENOR,OFF,OFF
1559,120,202,-96,POT_MENOR,OFF,OFF
1609,120,198,-100,POT_MENOR,OFF,OFF
1659,120,192,-106,POT_MENOR,OFF,OFF
1709,120,188,-110,POT_MENOR,OFF,OFF
1759,120,183,-115,POT_MENOR,OFF,OFF
1809,120,178,-120,POT_MENOR,OFF,OFF
1859,120,174,-124,POT_MENOR,OFF,OFF
1909,120,169,-129,POT_MENOR,OFF,OFF
1959,120,166,-132,POT_MENOR,OFF,OFF
2009,120,160,-138,POT_MENOR,OFF,OFF
2059,120,157,-141,POT_MENOR,OFF,OFF
2109,120,155,-143,POT_MENOR,OFF,OFF
2159,120,151,-147,POT_MENOR,OFF,OFF
2209,120,147,-151,POT_MENOR,OFF,OFF
2259,120,142,-156,POT_MENOR,OFF,OFF
2309,120,137,-161,POT_MENOR,OFF,OFF
2359,120,134,-164,POT_MENOR,OFF,OFF
2409,120,130,-168,POT_MENOR,OFF,OFF
2459,120,126,-172,POT_MENOR,OFF,OFF
2509,120,121,-177,POT_MENOR,OFF,OFF
2560,120,118,-180,POT_MENOR,OFF,OFF
2610,120,113,-185,POT_MENOR,OFF,OFF
2660,120,105,-193,POT_MENOR,OFF,OFF
2710,120,100,-198,POT_MENOR,OFF,OFF
2760,120,96,-202,POT_MENOR,OFF,OFF
2810,120,91,-207,POT_MENOR,OFF,OFF
2860,120,86,-212,POT_MENOR,OFF,OFF
2910,120,81,-217,POT_MENOR,OFF,OFF
2960,120,75,-223,POT_MENOR,OFF,OFF
3010,120,70,-228,POT_MENOR,OFF,OFF
3060,120,67,-231,POT_MENOR,OFF,OFF
3110,120,63,-235,POT_MENOR,OFF,OFF
3160,120,57,-241,POT_MENOR,OFF,OFF
3210,120,53,-245,POT_MENOR,OFF,OFF
3260,120,49,-249,POT_MENOR,OFF,OFF
3310,120,42,-256,POT_MENOR,OFF,OFF
3360,120,38,-260,POT_MENOR,OFF,OFF
3410,120,31,-267,POT_MENOR,OFF,OFF
3460,120,27,-271,POT_MENOR,OFF,OFF
3510,120,22,-276,POT_MENOR,OFF,OFF
3560,120,26,-272,POT_MENOR,OFF,OFF
3610,120,20,-278,POT_MENOR,OFF,OFF
3660,120,26,-272,POT_MENOR,OFF,OFF
3710,120,20,-278,POT_MENOR,OFF,OFF
3760,120,21,-277,POT_MENOR,OFF,OFF
3810,120,20,-278,POT_MENOR,OFF,OFF
3860,120,20,-278,POT_MENOR,OFF,OFF
3910,120,20,-278,POT_MENOR,OFF,OFF
3960,120,21,-277,POT_MENOR,OFF,OFF
4011,120,20,-278,POT_MENOR,OFF,OFF
4061,120,20,-278,POT_MENOR,OFF,OFF
3660,120,26,-272,POT_MENOR,OFF,OFF
3710,120,20,-278,POT_MENOR,OFF,OFF
3760,120,21,-277,POT_MENOR,OFF,OFF
3810,120,20,-278,POT_MENOR,OFF,OFF
3860,120,20,-278,POT_MENOR,OFF,OFF
3910,120,21,-277,POT_MENOR,OFF,OFF
3960,120,21,-277,POT_MENOR,OFF,OFF
4011,120,20,-278,POT_MENOR,OFF,OFF
4061,120,20,-278,POT_MENOR,OFF,OFF
4111,120,21,-277,POT_MENOR,OFF,OFF
4161,120,20,-278,POT_MENOR,OFF,OFF
4211,120,20,-278,POT_MENOR,OFF,OFF
4261,120,21,-277,POT_MENOR,OFF,OFF
4311,120,21,-277,POT_MENOR,OFF,OFF
4361,120,20,-278,POT_MENOR,OFF,OFF
4411,120,21,-277,POT_MENOR,OFF,OFF
4461,120,21,-277,POT_MENOR,OFF,OFF
4511,120,21,-277,POT_MENOR,OFF,OFF
4561,120,21,-277,POT_MENOR,OFF,OFF
4611,120,21,-277,POT_MENOR,OFF,OFF
4661,120,21,-277,POT_MENOR,OFF,OFF
4711,120,21,-277,POT_MENOR,OFF,OFF
4761,120,22,-276,POT_MENOR,OFF,OFF
4811,120,21,-277,POT_MENOR,OFF,OFF
4861,120,22,-276,POT_MENOR,OFF,OFF
4911,120,22,-276,POT_MENOR,OFF,OFF
4961,120,21,-277,POT_MENOR,OFF,OFF
==== FIN DE PRUEBA ====

==== INICIO DE PRUEBA ====
PWM fijo: 120
Posicion inicial: 22
t_ms,pwm,posicion,delta,direccion,sw_abajo,sw_arriba
358,120,15,-7,POT_MAYOR,OFF,OFF
408,120,2,-20,POT_MAYOR,OFF,OFF
458,120,3,-19,POT_MAYOR,OFF,OFF
508,120,8,-14,POT_MAYOR,OFF,OFF
558,120,4,-18,POT_MAYOR,OFF,OFF
608,120,7,-15,POT_MAYOR,OFF,OFF
658,120,12,-10,POT_MAYOR,OFF,OFF
708,120,17,-5,POT_MAYOR,OFF,OFF
758,120,23,1,POT_MAYOR,OFF,OFF
808,120,27,5,POT_MAYOR,OFF,OFF
858,120,32,10,POT_MAYOR,OFF,OFF
909,120,34,12,POT_MAYOR,OFF,OFF
959,120,37,15,POT_MAYOR,OFF,OFF
1009,120,45,23,POT_MAYOR,OFF,OFF
1059,120,49,27,POT_MAYOR,OFF,OFF
1109,120,52,30,POT_MAYOR,OFF,OFF
1159,120,78,56,POT_MAYOR,OFF,OFF
1209,120,67,45,POT_MAYOR,OFF,OFF
1259,120,98,76,POT_MAYOR,OFF,OFF
1309,120,77,55,POT_MAYOR,OFF,OFF
1359,120,79,57,POT_MAYOR,OFF,OFF
1409,120,85,63,POT_MAYOR,OFF,OFF
1459,120,88,66,POT_MAYOR,OFF,OFF
1509,120,97,75,POT_MAYOR,OFF,OFF
1559,120,107,85,POT_MAYOR,OFF,OFF
1609,120,110,88,POT_MAYOR,OFF,OFF
1659,120,114,92,POT_MAYOR,OFF,OFF
1709,120,119,97,POT_MAYOR,OFF,OFF
1759,120,124,102,POT_MAYOR,OFF,OFF
1809,120,129,107,POT_MAYOR,OFF,OFF
1859,120,133,111,POT_MAYOR,OFF,OFF
1909,120,137,115,POT_MAYOR,OFF,OFF
1959,120,141,119,POT_MAYOR,OFF,OFF
2009,120,148,126,POT_MAYOR,OFF,OFF
2059,120,153,131,POT_MAYOR,OFF,OFF
2109,120,157,135,POT_MAYOR,OFF,OFF
2159,120,162,140,POT_MAYOR,OFF,OFF
2209,120,172,150,POT_MAYOR,OFF,OFF
2259,120,176,154,POT_MAYOR,OFF,OFF
2309,120,185,163,POT_MAYOR,OFF,OFF
2360,120,192,170,POT_MAYOR,OFF,OFF
2410,120,199,177,POT_MAYOR,OFF,OFF
2460,120,206,184,POT_MAYOR,OFF,OFF
2510,120,213,191,POT_MAYOR,OFF,OFF
2560,120,219,197,POT_MAYOR,OFF,OFF
2610,120,226,204,POT_MAYOR,OFF,OFF
2660,120,233,211,POT_MAYOR,OFF,OFF
2710,120,238,216,POT_MAYOR,OFF,OFF
2760,120,244,222,POT_MAYOR,OFF,OFF
2810,120,249,227,POT_MAYOR,OFF,OFF
2860,120,255,233,POT_MAYOR,OFF,OFF
2910,120,260,238,POT_MAYOR,OFF,OFF
2960,120,264,242,POT_MAYOR,OFF,OFF
3010,120,269,247,POT_MAYOR,OFF,OFF
3060,120,273,251,POT_MAYOR,OFF,OFF
3110,120,278,256,POT_MAYOR,OFF,OFF
3160,120,281,259,POT_MAYOR,OFF,OFF
3210,120,283,261,POT_MAYOR,OFF,OFF
3260,120,288,266,POT_MAYOR,OFF,OFF
3310,120,292,270,POT_MAYOR,OFF,OFF
3360,120,297,275,POT_MAYOR,OFF,OFF
3410,120,300,278,POT_MAYOR,OFF,OFF
3460,120,304,282,POT_MAYOR,OFF,OFF
3510,120,309,287,POT_MAYOR,OFF,OFF
3560,120,314,292,POT_MAYOR,OFF,OFF
3610,120,319,297,POT_MAYOR,OFF,OFF
3660,120,324,302,POT_MAYOR,OFF,OFF
3710,120,329,307,POT_MAYOR,OFF,OFF
3760,120,334,312,POT_MAYOR,OFF,OFF
3810,120,339,317,POT_MAYOR,OFF,OFF
3860,120,344,322,POT_MAYOR,OFF,OFF
3910,120,348,326,POT_MAYOR,OFF,OFF
3960,120,353,331,POT_MAYOR,OFF,OFF
4010,120,358,336,POT_MAYOR,OFF,OFF
4060,120,359,337,POT_MAYOR,OFF,OFF
4110,120,369,347,POT_MAYOR,OFF,OFF
4160,120,374,352,POT_MAYOR,OFF,OFF
4210,120,381,359,POT_MAYOR,OFF,OFF
4260,120,385,363,POT_MAYOR,OFF,OFF
4310,120,389,367,POT_MAYOR,OFF,OFF
4360,120,394,372,POT_MAYOR,OFF,OFF
4410,120,400,378,POT_MAYOR,OFF,OFF
4460,120,403,381,POT_MAYOR,OFF,OFF
4510,120,410,388,POT_MAYOR,OFF,OFF
4560,120,417,395,POT_MAYOR,OFF,OFF
4610,120,421,399,POT_MAYOR,OFF,OFF
4660,120,427,405,POT_MAYOR,OFF,OFF
4710,120,432,410,POT_MAYOR,OFF,OFF
4760,120,431,409,POT_MAYOR,OFF,OFF
4810,120,445,423,POT_MAYOR,OFF,OFF
4860,120,447,425,POT_MAYOR,OFF,OFF
4910,120,453,431,POT_MAYOR,OFF,OFF
4960,120,458,436,POT_MAYOR,OFF,OFF
==== FIN DE PRUEBA ====
"""

# ============================================================
# Configuración
# ============================================================
CARPETA_SALIDA = "graficas_modelo"
TS_PID = 0.05  # 50 ms

os.makedirs(CARPETA_SALIDA, exist_ok=True)

COLUMNAS = [
    "t_ms",
    "pwm",
    "posicion",
    "delta",
    "direccion",
    "sw_abajo",
    "sw_arriba",
]


# ============================================================
# Parseo de datos
# ============================================================
def extraer_datos(texto):
    lineas = []

    for linea in texto.splitlines():
        linea = linea.strip()

        if re.match(r"^\d+,\d+,-?\d+,-?\d+,[A-Z_]+,[A-Z]+,[A-Z]+$", linea):
            lineas.append(linea)

    csv_limpio = "\n".join(lineas)
    df = pd.read_csv(StringIO(csv_limpio), names=COLUMNAS)

    df = df.drop_duplicates(subset=["t_ms", "direccion"], keep="first")
    df["t_s"] = df["t_ms"] / 1000.0

    return df


# ============================================================
# Filtrado de zona útil para ajuste
# Evita zonas saturadas cerca de 0 y datos raros iniciales
# ============================================================
def filtrar_zona_util(df_dir):
    direccion = df_dir["direccion"].iloc[0]

    if direccion == "POT_MENOR":
        # Quita saturación inferior cerca de 20 ADC
        df_fit = df_dir[(df_dir["posicion"] >= 30) & (df_dir["posicion"] <= 295)].copy()
    else:
        # Quita arranque raro cerca de 0 ADC
        df_fit = df_dir[(df_dir["posicion"] >= 80) & (df_dir["posicion"] <= 460)].copy()

    df_fit["t_rel_s"] = df_fit["t_s"] - df_fit["t_s"].iloc[0]

    return df_fit


# ============================================================
# Ajuste lineal y cálculo de modelo
# ============================================================
def analizar_direccion(df, direccion):
    df_dir = df[df["direccion"] == direccion].copy()
    df_dir["t_rel_s"] = df_dir["t_s"] - df_dir["t_s"].iloc[0]

    df_fit = filtrar_zona_util(df_dir)

    x = df_fit["t_rel_s"].values
    y = df_fit["posicion"].values

    pendiente, intercepto = np.polyfit(x, y, 1)

    pwm = df_fit["pwm"].iloc[0]
    velocidad = pendiente
    k_signed = velocidad / pwm
    k_abs = abs(k_signed)
    k_discreto_abs = k_abs * TS_PID

    print("\n========================================")
    print(f"Dirección: {direccion}")
    print("========================================")
    print(f"PWM usado: {pwm}")
    print(f"Puntos totales: {len(df_dir)}")
    print(f"Puntos usados para ajuste: {len(df_fit)}")
    print(f"Velocidad aproximada: {velocidad:.4f} ADC/s")
    print(f"Ganancia firmada K: {k_signed:.6f} ADC/(s*PWM)")
    print(f"Ganancia absoluta |K|: {k_abs:.6f} ADC/(s*PWM)")
    print(f"Modelo continuo firmado: G(s) = {k_signed:.6f} / s")
    print(f"Modelo continuo por magnitud: G(s) = {k_abs:.6f} / s")
    print(f"Modelo discreto por magnitud: G(z) = {k_discreto_abs:.6f} / (z - 1)")

    # Gráfica completa
    plt.figure()
    plt.plot(df_dir["t_rel_s"], df_dir["posicion"], marker="o", label="Datos completos")
    plt.xlabel("Tiempo (s)")
    plt.ylabel("Posición del potenciómetro (ADC)")
    plt.title(f"Respuesta completa en lazo abierto - {direccion}")
    plt.grid(True)
    plt.legend()
    plt.savefig(f"{CARPETA_SALIDA}/respuesta_completa_{direccion}.png", dpi=300, bbox_inches="tight")
    plt.close()

    # Gráfica ajuste
    y_ajuste = pendiente * x + intercepto

    plt.figure()
    plt.plot(df_fit["t_rel_s"], df_fit["posicion"], marker="o", label="Datos usados")
    plt.plot(df_fit["t_rel_s"], y_ajuste, label="Ajuste lineal")
    plt.xlabel("Tiempo (s)")
    plt.ylabel("Posición del potenciómetro (ADC)")
    plt.title(f"Ajuste lineal para modelo - {direccion}")
    plt.grid(True)
    plt.legend()
    plt.savefig(f"{CARPETA_SALIDA}/ajuste_lineal_{direccion}.png", dpi=300, bbox_inches="tight")
    plt.close()

    return {
        "direccion": direccion,
        "pwm": pwm,
        "velocidad": velocidad,
        "k_signed": k_signed,
        "k_abs": k_abs,
        "k_discreto_abs": k_discreto_abs,
        "puntos_fit": len(df_fit),
    }


# ============================================================
# Main
# ============================================================
df = extraer_datos(datos_txt)

resultados = []

for direccion in ["POT_MENOR", "POT_MAYOR"]:
    resultados.append(analizar_direccion(df, direccion))

# Modelo promedio por magnitud
k_promedio = np.mean([r["k_abs"] for r in resultados])
k_discreto_promedio = k_promedio * TS_PID

print("\n========================================")
print("MODELO PROMEDIO PARA EL INFORME")
print("========================================")
print(f"K promedio: {k_promedio:.6f} ADC/(s*PWM)")
print(f"Modelo continuo aproximado: G(s) = {k_promedio:.6f} / s")
print(f"Con Ts = {TS_PID:.3f} s")
print(f"Modelo discreto aproximado: G(z) = {k_discreto_promedio:.6f} / (z - 1)")
print("Polo continuo: s = 0")
print("Ceros continuos: ninguno")
print("Polo discreto: z = 1")
print("Ceros discretos: ninguno")

# Guardar resumen en txt
with open(f"{CARPETA_SALIDA}/resumen_modelo.txt", "w", encoding="utf-8") as f:
    f.write("RESUMEN DEL MODELO MATEMÁTICO\n")
    f.write("=============================\n\n")

    for r in resultados:
        f.write(f"Dirección: {r['direccion']}\n")
        f.write(f"PWM usado: {r['pwm']}\n")
        f.write(f"Velocidad aproximada: {r['velocidad']:.4f} ADC/s\n")
        f.write(f"Ganancia firmada K: {r['k_signed']:.6f} ADC/(s*PWM)\n")
        f.write(f"Ganancia absoluta |K|: {r['k_abs']:.6f} ADC/(s*PWM)\n")
        f.write(f"Modelo continuo: G(s) = {r['k_signed']:.6f} / s\n\n")

    f.write("MODELO PROMEDIO\n")
    f.write(f"K promedio: {k_promedio:.6f} ADC/(s*PWM)\n")
    f.write(f"G(s) = {k_promedio:.6f} / s\n")
    f.write(f"Ts = {TS_PID:.3f} s\n")
    f.write(f"G(z) = {k_discreto_promedio:.6f} / (z - 1)\n")
    f.write("Polo continuo: s = 0\n")
    f.write("Ceros continuos: ninguno\n")
    f.write("Polo discreto: z = 1\n")
    f.write("Ceros discretos: ninguno\n")

print(f"\nGráficas y resumen guardados en: {CARPETA_SALIDA}")