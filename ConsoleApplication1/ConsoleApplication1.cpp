#include <string>       //Für std:: string
#include <iostream>     //Für Ein- und Ausgabe
#include <fstream>      //Für Dateioperationen
#include <windows.h>        //Für Windows-spezifische serielle Kommunikation

int main() 
{
    HANDLE hSerial;
    DCB dcbSerialParams = { 0 };
    COMMTIMEOUTS timeouts = { 0 };

    std::ofstream logFile("kommunikation_log.txt");     //Logdatei öffnen/schreiben

    hSerial = CreateFile(L"\\\\.\\COM6", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);      //Serielle Schnittstelle
    if (hSerial == INVALID_HANDLE_VALUE)        // Fehlercode beim fehlerhaftigen Öffnen des Ports
    {
        std::cerr << "Fehler beim Öffnen des Ports.\n";
        return 1;
    }

    if (!GetCommState(hSerial, &dcbSerialParams))       //Aktuelle Einstellungen lesen
    {
        std::cerr << "❌ Fehler beim Lesen der seriellen Einstellungen.\n"; // Fehlerbehandlungsmodul
        CloseHandle(hSerial);
        return 1;
    }
    
    //Parameter setzen z.b.: Baudrate, Datenbits, Stopbits, Parität
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);
    dcbSerialParams.BaudRate = CBR_9600;        
    dcbSerialParams.ByteSize = 8;      
    dcbSerialParams.StopBits = ONESTOPBIT; 
    dcbSerialParams.Parity = NOPARITY;
    SetCommState(hSerial, &dcbSerialParams);

    //Timeout-Einstellungen
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);

    std::string input;      //Eingabe (Eingabe-Modul)
    while (true) {
        std::cout << "Gib eine Rechung ein oder 'exit' um die Seite zu verlassen: ";
        std::getline(std::cin, input); // lesen der Eingabe
        if (input == "exit") break;

        input += "\n"; // wichtig für den Arduino, damit er weiß, wann die Eingabe fertig ist

        // an den Mikrocontroller senden (Kommunikationsmodul)
        DWORD bytesWritten;
        WriteFile(hSerial, input.c_str(), input.size(), &bytesWritten, nullptr);

        //Anwort von Mikrocontroller lesen
        char buffer[128];
        DWORD bytesRead;
        BOOL success = ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, nullptr);
        if (!success) {
            std::cerr << "❌ Fehler beim Lesen vom Port! Fehlercode: " << GetLastError() << "\n"; //Fehlerbehandlungsmodul
            continue;
        }
        buffer[bytesRead] = '\0';

        std::cout << "" << buffer; //ANzeige-Modul
        //In Logdatei schreiben
        logFile << "Eingabe: " << input;
        logFile << "Antwort: " << buffer << "\n";
    }
    //Ressourcen freigeben
    CloseHandle(hSerial);
    logFile.close();
    return 0;
}
