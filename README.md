# Die-SoundBox

Kompakte, benutzerfreundliche Soundbox auf Basis des ESP32 mit Audio-Wiedergabe von MicroSD und Steuerung über verschiedene Eingabekomponenten wie Tasten und Dreh-Encoder.

## Projektbeschreibung

Das Ziel dieses Projekts ist es, eine kompakte und benutzerfreundliche Soundbox zu entwickeln, die es ermöglicht, Musik von einer MicroSD-Karte zu lesen und mit verschiedenen Steuerelementen wie einem Dreh-Encoder und Tasten zu interagieren. Der ESP32-Mikrocontroller übernimmt die zentrale Steuerung der Audio-Wiedergabe sowie die Verwaltung des Displays und der Eingabegeräte.

Das OLED-Display ermöglicht eine visuelle Rückmeldung, zum Beispiel die Anzeige des aktuellen Titels oder der Lautstärke. Mit dem Dreh-Encoder und den Tasten kann der Benutzer die Lautstärke anpassen und zwischen den Tracks navigieren. Der MicroSD-Kartenleser ermöglicht das Laden der Musikdateien der SD-Karte.

Der Youmile PCM5102 DAC Modul wandelt die digitalen Audiodaten in analoge Audiosignale um, während der Adafruit Mono Class D Audio-Verstärker – PAM8302A das Signal verstärkt und an den Lautsprecher weitergibt. Der 3W Lautsprecher sorgt für eine klare Wiedergabe des Audiosignals.

Dieses Projekt kombiniert einfache und effektive Technologien, um eine benutzerfreundliche Soundbox zu schaffen, die Musik von einer SD-Karte abspielt und gleichzeitig Interaktivität und visuelle Rückmeldungen bietet. Es ist eine ideale Lösung für ein selbstgebautes Audio-Gerät.

## Eingesetzte Komponenten

- ESP32 NodeMCU Mikrocontroller  
- Youmile PCM5102 DAC Modul (GY-PCM5102)  
- Adafruit Mono Class D Audio-Verstärker – PAM8302A  
- Lautsprecher 3W, 4 Ohm  
- OLED-Display (Adafruit SSD1306, 128x64, I2C)  
- Dreh-Encoder (Iduino SE055)  
- 3 Tasten (Next, Previous, Power)  
- MicroSD-Kartenleser (HW-125)
  
## Komponentenplan
![Komponentenplan](https://github.com/user-attachments/assets/110586fd-fadd-4957-9b1d-cfbc01645494)


## Visuelle Darstellung
![Visuelle Darstellung](https://github.com/user-attachments/assets/2fcae056-3265-4c45-86ab-eb48bde39d25)

## Fertiger Aufbau



![Fertiger Aufbau](https://github.com/user-attachments/assets/67a8bc0e-26a7-4d6b-90e8-9d7780161cc2)

