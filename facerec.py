import cv2
import serial
import time
import sys

ARDUINO_PORT = '/dev/cu.usbmodem11301'
BAUD_RATE = 9600

CAMERA_WIDTH = 640
CAMERA_HEIGHT = 480
FACE_SCALE_FACTOR = 1.1
FACE_MIN_NEIGHBORS = 5
# eerst indice 1 gebruiken omdat iphone indice 0 soms overneemt
CAMERA_INDICES = [0, 1, 2, 3]

FACE_CASCADE_PATH = cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
face_cascade = cv2.CascadeClassifier(FACE_CASCADE_PATH)

try:
    arduino = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=0.1) # connecten aan arduino
    time.sleep(3)
    print(f"Connected to Arduino on port {ARDUINO_PORT}")
except serial.SerialException as e:
    print(f"Error connecting to Arduino on {ARDUINO_PORT}.")
    print(e)
    sys.exit()

cap = None
chosen_index = -1
for i in CAMERA_INDICES:
    temp_cap = cv2.VideoCapture(i, cv2.CAP_AVFOUNDATION)
    if temp_cap.isOpened():
        temp_cap.set(cv2.CAP_PROP_FRAME_WIDTH, CAMERA_WIDTH)
        temp_cap.set(cv2.CAP_PROP_FRAME_HEIGHT, CAMERA_HEIGHT)
        cap = temp_cap
        chosen_index = i
        break

if cap is None:
    print("Failed to open webcam.")
    arduino.close()
    sys.exit()

print(f"Webcam successfully opened using index {chosen_index}. Resolution: {CAMERA_WIDTH}x{CAMERA_HEIGHT}")
print("Starting face tracking loop. Press 'q' to exit.")

def send_serial_coordinates(x, y):
    command = f"{x},{y}\n"
    try:
        arduino.write(command.encode('utf-8'))
    except serial.SerialException:
        print("Serial connection lost. (skill issue haha)")
        return False
    return True

smoothed_x = CAMERA_WIDTH // 2
smoothed_y = CAMERA_HEIGHT // 2
SMOOTHING_FACTOR = 0.2

try:
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame.")
            break

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        faces = face_cascade.detectMultiScale(
            gray,
            scaleFactor=FACE_SCALE_FACTOR,
            minNeighbors=FACE_MIN_NEIGHBORS,
            minSize=(30, 30)
        )

        if len(faces) > 0:
            # === TRACKING MODE: ALLEEN HIER COÖRDINATEN VERSTUREN ===
            (x, y, w, h) = max(faces, key=lambda f: f[2] * f[3])
            target_x = x + w // 2
            target_y = y + h // 2

            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            cv2.circle(frame, (target_x, target_y), 5, (0, 0, 255), -1)
            cv2.putText(frame, 'TRACKING', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

            # Bereken de gesmoothe coördinaten
            smoothed_x = int(smoothed_x + (target_x - smoothed_x) * SMOOTHING_FACTOR)
            smoothed_y = int(smoothed_y + (target_y - smoothed_y) * SMOOTHING_FACTOR)

            # Verstuur de coördinaten naar de Arduino
            send_serial_coordinates(smoothed_x, smoothed_y)

        else:
            # === SEARCHING MODE: NIETS VERSTUREN ===
            # De Turret moet stoppen met bewegen en uitschakelen (via Arduino timeout)
            target_x = CAMERA_WIDTH // 2
            target_y = CAMERA_HEIGHT // 2
            cv2.putText(frame, 'SEARCHING', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)
            
            # De smoothing kan hier nog steeds plaatsvinden, maar de output wordt niet verzonden.
            # Dit helpt om de turret zachtjes naar het midden te brengen als er opnieuw een gezicht wordt gedetecteerd.
            smoothed_x = int(smoothed_x + (target_x - smoothed_x) * SMOOTHING_FACTOR)
            smoothed_y = int(smoothed_y + (target_y - smoothed_y) * SMOOTHING_FACTOR)

        cv2.line(frame, (CAMERA_WIDTH // 2, 0), (CAMERA_WIDTH // 2, CAMERA_HEIGHT), (255, 255, 255), 1)

        cv2.imshow('Turret view', frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

        time.sleep(0.01)

except KeyboardInterrupt:
    print("Exiting..")

finally:
    print("Closing connection and exiting.")
    # Stuur de midden-positie als een laatste commando om de Arduino te resetten.
    send_serial_coordinates(CAMERA_WIDTH // 2, CAMERA_HEIGHT // 2)
    if cap is not None:
        cap.release()
    cv2.destroyAllWindows()
    arduino.close()
