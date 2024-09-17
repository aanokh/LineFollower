# __Line Follower Robot__

<img src="./Media/robot.jpg" alt="An image of the robot" width="250" />

This autonomous line follower robot was created for the RoboGames Olympics 2024 by me and my younger brother. It has been designed, modeled, constructed and programmed from scratch. The robot can autonomously follow a black or white line on a contrasting background at high speeds by implementing a PID controller.

### 3D Design

The bracket and other components of the robot body were designed in Autodesk Fusion 360. They were then sliced in Ultimaker Cuda and 3d printed, first out of plastic and finally from carbon fiber. The main frame was designed to be easily replaceable to test varying robot lengths and multiple prototypes were tried before arriving at the final design.

### Electronics Design

<img src="./Media/pcb.jpg" alt="An image of the PCB" width="250" />

The robot uses an Arduino Nano ESP32 Microcontroller, [motor driver, motors, line sensor] These components are mounted on a custom 2-sided PCB Board that was designed and ordered with EasyEDA.

A stand-alone stopwatch device was created for measuring the lap time of the robot. It used [controller, sensor]

### Software Design

Since the robot needs to keep the line sensor centered on the line while experiencing external changes, the PID controller algorithm was chosen for this task. In this scenario the target setpoint is the line position detected by the sensor, set to be held in the middle. The process affected by the outputs is the turning of the motors.

<img src="./Media/PID-controller.jpg" alt="A diagram of the PID Controller" width="600" />

Each of the PID modules requires a coefficient, which is fine tuned by hand to optimally fit a given track. To expedite this process, a simple mobile application was created with MIT App Inventor which uses the Bluetooth Low Energy (BLE) technology to communicate with the Arduino microcontroller. This allows the coefficients to be quickly tweaked while observing the results in real time. When the desired coefficients are reached, they are serialized in the Arduino [permanent memory]

### Result Videos

[video.mp4](https://github.com/aanokh/LineFollower/blob/b71695a8783dbfd515533419e18241214081eb8c/Media/video.mp4)

### Future Improvements

A major improvement that could be made to the robot design is adding a turbine to improve the robot's friction.

The stopwatch device could be improved by letting it also connect to a mobile app and calculate the average lap time.

Multiple coefficient profiles could be added to support easy switching between tracks and frame lengths.
