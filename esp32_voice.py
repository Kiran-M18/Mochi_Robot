import speech_recognition as sr
import serial
import pyttsx3
import time

# ---------- CONNECT ESP32 ----------

print("Connecting to ESP32...")

ESP32_PORT = "COM9"
BAUD = 115200

try:
    ser = serial.Serial(ESP32_PORT, BAUD, timeout=1)
    print("ESP32 Connected")
except Exception as e:
    print("ERROR:", e)
    exit()

time.sleep(3)
ser.reset_input_buffer()
print("Serial ready\n")

# ---------- VOICE ----------

engine = pyttsx3.init()
rec = sr.Recognizer()

try:
    mic = sr.Microphone()
    print("Microphone ready")
except Exception as e:
    print("Mic error:", e)
    exit()

print("\nREADY — Speak\n")

# ---------- MAIN LOOP ----------

while True:
    try:
        # LISTEN
        with mic as source:
            print("Listening...")
            audio = rec.listen(source)

        # SPEECH → TEXT
        text = rec.recognize_google(audio)
        print("YOU:", text)

        # SEND TO ESP32
        ser.write((text + "\n").encode())

        # WAIT FOR RESPONSE
        reply = ""
        start = time.time()

        while True:
            if ser.in_waiting:
                line = ser.readline().decode(errors="ignore").strip()
                if line:
                    reply = line
                    break

            if time.time() - start > 30:
                reply = "ESP32 timeout"
                break

            time.sleep(0.05)

        print("AI:", reply)

        # SPEAK
        engine.say(reply)
        engine.runAndWait()

    except sr.UnknownValueError:
        print("Didn't catch that")

    except sr.RequestError:
        print("Speech API error")

    except Exception as e:
        print("Error:", e)