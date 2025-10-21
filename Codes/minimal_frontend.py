import serial, sys, time
from threading import Thread

PORT = "COM5"          # <-- cambia según tu PC (Linux: '/dev/ttyACM0' o '/dev/ttyUSB0')
BAUD = 115200

def reader(ser):
    while True:
        try:
            line = ser.readline().decode(errors="ignore").strip()
            if line:
                print("[MASTER]", line)
        except Exception as e:
            print("Reader error:", e)
            break

def send_line(ser, s):
    ser.write((s + "\n").encode())
    ser.flush()

def main():
    with serial.Serial(PORT, BAUD, timeout=0.1) as ser:
        print("Connected to", PORT)
        t = Thread(target=reader, args=(ser,), daemon=True)
        t.start()

        print("Commands:")
        print("  word HOLA         -> SET WORD HOLA")
        print("  index 2301        -> SET INDEX 2301")
        print("  start             -> SEND START")
        print("  reset             -> SEND RESET")
        print("  status            -> STATUS")
        print("  quit              -> exit")

        while True:
            try:
                s = input("> ").strip()
                if not s: continue
                if s == "quit": break

                if s.startswith("word "):
                    w = s.split()[1].upper()
                    send_line(ser, f"SET WORD {w}")
                elif s.startswith("index "):
                    idx = s.split()[1]
                    send_line(ser, f"SET INDEX {idx}")
                elif s == "start":
                    send_line(ser, "SEND START")
                elif s == "reset":
                    send_line(ser, "SEND RESET")
                elif s == "status":
                    send_line(ser, "STATUS")
                else:
                    print("unknown command")
            except KeyboardInterrupt:
                break

if __name__ == "__main__":
    main()
