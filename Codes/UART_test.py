from machine import UART, Pin
import time

# Inicializar UART1
uart1 = UART(1, baudrate=9600, tx=Pin(4), rx=Pin(5))

# Inicializar contador de tiempo
ultimo_envio = time.ticks_ms()

while True:
    # Enviar "HOLA" cada 5 segundos
    if time.ticks_diff(time.ticks_ms(), ultimo_envio) >= 5000:
        uart1.write('HOLA ARDUINO\n')
        print("Enviado: HOLA ARDUINO")
        ultimo_envio = time.ticks_ms()

    # Leer si hay datos entrantes
    if uart1.any():
        recibido = uart1.read().decode('utf-8').strip()
        print("Recibido por UART1:", recibido)