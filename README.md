# ELEN30013 - Electronic System Implementation (ESI)

Introducing... S.N.I.F.E.R - Sensory Network for Intuitive Feedback on Environmental Risks.

This project is inspired by Mark Watney's bio-monitor displayed The Martian movie.

This project uses the tft_espi ui library for embedded displays. The initial setup to use this library was elusive so I have included the
setup that I used for my specific drvier chip (ILI9488). In the setup files you'll see how to select your supported driver chip and the
pin definitions you need to make to ensure the library operates correctly.

## The Details
The purpose of this project is to build an assistive technology that  uses an array of sensors. The sensor suite for our product is 
tailored to give the user feedback on their surrounding environment. At the heart of this project is the MQ135 air quality 
sensor that returns the air quality in parts-per-million (PPM). Based on the thresholds we have set, the user is notified of
increasingly worsening environmental conditions via visuals on a liquid crystal display (LCD) and other electronic components with a 
sensory output. This device is portable and can be strapped to a bag or on the users arm. It is powered by two lithium ion batteries.

## Implementation
The main sensor suite is connected to an arduino uno which is used to process ingress sensor data and output visual indication of 
air quality using an RGB LED module (green, yellow, red) as well as an audio output using a piezo buzzer that increases its tone
frequency as conditions get increasingly worse. 

The data from the sensors is packaged into a packet, serialised  and sent to an ESP8266 via UART software serial communication. Since
arduino uses 5V logic and ESPs use 3.3V logic, a level shifter is employed to safely send data between these two microcontrollers. 
Once data arrives at the ESP8266, it is deserialised and used to set the internal state of the ESP while also displaying the data
to a thin-film-transistor (TFT) LCD using the ILI9488 driver chip.

## System Diagram
![Image of Product System Diagram](https://github.com/yash-chaudhary/ELEN30013_ESI/blob/main/assets/system_diagram.png)

## LCD Interface
![Image of LCD Interface](https://github.com/yash-chaudhary/ELEN30013_ESI/blob/main/assets/lcd_ui.png)

## Finished build
![Image of Mechanical Design](https://github.com/yash-chaudhary/ELEN30013_ESI/blob/main/assets/mechanical_assembly.png)


