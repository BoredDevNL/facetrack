# facetrack
*Made possible by [PCBWay.com](https://pcbway.com)*

A Simple Arduino project that uses the camera (python script) to turn coordinates into degrees for servos attached to an arduino and turn on a water pump to spray water. (Turret)

## Hardware Requirements

- Arduino board
- 4 servo motors (horizontal, vertical and 2 servo's to turn the pump on/off)
- LED
- Webcam
- USB cable for Arduino connection
- Water pump
- Breadboard if you don't have an Arduino sensor board

## Pin Configuration

- horservo: Pin 8
- verservo: Pin 7
- LED: Pin 6
- servo_on: Pin 5
- servo_off: Pin 4

## Software Requirements

- Python 3.x
- OpenCV (cv2)
- pyserial
- Arduino IDE

## Installation

1. Install the required Python packages:
```bash
pip install opencv-python pyserial
```

2. Upload the Arduino sketch (`facetrack.ino`) to your Arduino board using the Arduino IDE.

3. Connect the hardware according to the pin configuration.

4. Update the `ARDUINO_PORT` in `facerec.py` to match your Arduino's port.


## Usage

1. Connect the Arduino to your computer.

2. Run the Python script:
```bash
python facerec.py
```

3. The system will:
   - Initialize the camera and Arduino connection
   - Display the camera feed
   - Control the servos to track detected faces
   - Turn on the water pump to spray water when a face is detected.

4. ctrl + c to quit

## Camera Settings

- Resolution: 640x480

