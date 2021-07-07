//323CB Cazan Bogdan-Marian

#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.hpp"
#include "requests.hpp"
#include "buffer.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <cstring>

// for convenience
using json = nlohmann::json;

using namespace std;

#define INPUT_SIZE 4096 // lungimea maxima a unui input
#define URL_SIZE 256 // lungimea maxima a unui url
#define APP_SIZE 128 // lungimea maxima a unei aplicatii (content type)
#define LOGIN_TOKEN_SIZE 512 // lungimea maxima a unui token/cookie de logare
#define JWT_SIZE 1024 // lungimea maxima a unui token JWT

// realizam o structura pentru client, in care sa retinem toate campurile
// necesare pentru realizarea temei
struct Client {
    int portno = 8080;
    char ip_addr[20] = "34.118.48.238";
    char host[50] = "34.118.48.238:8080";
    char input[INPUT_SIZE];
    char url[URL_SIZE];
    char app_type[128];
    char* message;
    char* response;
    int sockfd;
    json j;
    char* json_str;
    char login_token[LOGIN_TOKEN_SIZE];
    char jwt_token[JWT_SIZE];   
}TClient;

// functie care extrage din raspunsul server-ului tokenul de logarea
char* get_login_token(struct Client* client) {
    char* aux = strstr(client->response, "Date: ");
    aux[-2] = '\0';
    return strstr(client->response, "Set-Cookie: ") + 12;
}

// functie care se apeleaza in momentul in care de la tastatura se introduce
// comanda "register" -- efectueaza inregistrare
void register_command(struct Client* client) {
    client->j = nullptr;
    // introducerea username-ului
    cout << "username = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    client->j["username"] = client->input;
    // introducerea parolei
    cout << "password = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    client->j["password"] = client->input;
    // ruta de acces
    memset(client->url, 0, URL_SIZE);
    strcpy(client->url, "/api/v1/tema/auth/register");
    // content type
    memset(client->app_type, 0, APP_SIZE);
    strcpy(client->app_type, "json");
    // crearea mesajului final ce trebuie trimis serverului + trimitere request
    client->message = compute_post_request(client->host, client->url, 
        client->app_type, client->j.dump().c_str(), NULL, 0, NULL);
    send_to_server(client->sockfd, client->message);
    // primirea unui raspuns din partea serverului
    client->response = receive_from_server(client->sockfd);
    client->json_str = basic_extract_json_response(client->response);
    // interpretarea raspunsului primit
    if(client->json_str == NULL) {
        cout << "Client was registered succesfully!" << endl;
    }
    // mesaj de eroare cel mai probabil
    else {
        client->j = json::parse(client->json_str);
        cout << client->j["error"] << endl;
    }
    cout << endl;
}

// functie care se apeleaza in momentul in care de la tastatura se introduce
// comanda "login" -- efectueaza autentificarea
void login_command(struct Client* client) {
    client->j = nullptr;
    // introducerea username-ului
    cout << "username = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    client->j["username"] = client->input;
     // introducerea parolei
    cout << "password = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    client->j["password"] = client->input;
    // ruta de acces
    memset(client->url, 0, URL_SIZE);
    strcpy(client->url, "/api/v1/tema/auth/login");
    // content type
    memset(client->app_type, 0, APP_SIZE);
    strcpy(client->app_type, "json");
    // crearea mesajului final ce trebuie trimis serverului + trimitere request
    client->message = compute_post_request(client->host, client->url, 
        client->app_type, client->j.dump().c_str(), NULL, 0, NULL);
    send_to_server(client->sockfd, client->message);
    // primirea unui raspuns din partea serverului
    client->response = receive_from_server(client->sockfd);
    client->json_str = basic_extract_json_response(client->response);
    // interpretarea raspunsului primit
    if(client->json_str == NULL) {
        // primirea unui cookie de login, pe care trebuie sa il retinem cat
        // timp utilizatorul este conectat
        memset(client->login_token, 0, LOGIN_TOKEN_SIZE);
        strcpy(client->login_token, get_login_token(client));
        cout << "Client has entered the party!" << endl;
    }
    // mesaj de eroare cel mai probabil
    else {
        client->j = json::parse(client->json_str);
        cout << client->j["error"] << endl;
    }
    cout << endl;
}

// functie care se apeleaza in momentul in care de la tastatura se introduce
// comanda "enter_library" -- cere acces in biblioteca
void enter_library(struct Client* client) {
    client->j = nullptr;
    // ruta de acces
    memset(client->url, 0, URL_SIZE);
    strcpy(client->url, "/api/v1/tema/library/access");
    // introducem pentru verificare cookie-ul primit la logare
    char** cookies = (char **)malloc(sizeof(char *));
    cookies[0] = (char *)malloc(LOGIN_TOKEN_SIZE * sizeof(char));
    strcpy(cookies[0], client->login_token);
    // crearea mesajului final ce trebuie trimis serverului + trimitere request
    client->message = compute_get_request(client->host, 
        client->url, NULL, cookies, 1, NULL);
    send_to_server(client->sockfd, client->message);
    // primirea unui raspuns din partea serverului
    client->response = receive_from_server(client->sockfd);
    client->json_str = basic_extract_json_response(client->response);
    // interpretarea raspunsului primit
    if(client->json_str == NULL) {
        cout << "Something strange happened :(" << endl;
        cout << client->response << endl;
    }
    // mesaj de eroare cel mai probabil
    else {
        client->j = json::parse(client->json_str);
        if(strncmp(client->json_str, "{\"error:", 7) == 0) {
            cout << client->j["error"] << endl;
        }
        // se primeste un token JWT care ne ofera accesul la diferite componente
        // ale bibliotecii
        else {
            memset(client->jwt_token, 0, JWT_SIZE);
            sprintf(client->jwt_token, "%s", 
                client->j["token"].dump().c_str() + 1);
            client->jwt_token[strlen(client->jwt_token) - 1] = '\0';
            cout << "You have entered the library! Stay tuned" << endl;
        }
    }
    cout << endl;
}

// functie care afiseaza lista de carti din cadrul bibliotecii
void print_books_in_library(struct Client* client) {
    for (auto elem : client->j) {
        cout << "ID: " << elem["id"] << endl;
        cout << "Title: " << elem["title"] << endl;
        cout << endl;
    }
}

// functie care se apeleaza in momentul in care de la tastatura se introduce
// comanda "get_books" -- cere toate cartile de pe server
void get_books(struct Client* client) {
    client->j = nullptr;
    // ruta de acces
    memset(client->url, 0, URL_SIZE);
    strcpy(client->url, "/api/v1/tema/library/books");
    // crearea mesajului final ce trebuie trimis serverului + trimitere request
    // se observa trimiterea cu token-ul de acces JWT
    client->message = compute_get_request(client->host, client->url, 
        NULL, NULL, 0, client->jwt_token);
    send_to_server(client->sockfd, client->message);
    // primirea unui raspuns din partea serverului
    client->response = receive_from_server(client->sockfd);
    client->json_str = advanced_extract_json_response(client->response);
    // interpretarea raspunsului primit
    if (client->json_str == NULL) {
        client->json_str = basic_extract_json_response(client->response);
        if (client->json_str == NULL) {
            // nu exista carti in biblioteca
            if (strstr(client->response, "[]") != NULL) {
                cout << "There are no books in this library :(" << endl;
            }
            // alt mesaj necunoscut
            else {
                cout << "Something strange happened :(" << endl;
                cout << client->response << endl;
            }
        }
        // eroare primita de la server
        else {
            client->j = json::parse(client->json_str);
            
            cout << client->j["error"] << endl;
        }
    }
    // afisare lista carti
    else {
        cout << "You have received this books:" << endl;
        client->j = json::parse(client->json_str);
        print_books_in_library(client);
    }
    cout << endl;
}

// functie care afiseaza toate detaliile unei carti; cartea a fost gasita dupa
// id
void print_book(struct Client* client) {
    for( auto elem : client->j) {
        cout << endl;
        cout << "Id: " << client->input << endl;
        cout << "title: " << elem["title"] << endl;
        cout << "author: " << elem["author"] << endl;
        cout << "genre: " << elem["genre"] << endl;
        cout << "page_count: " << elem["page_count"] << endl;
        cout << "publisher: " << elem["publisher"] << endl;
    }   
}

// functie care se apeleaza in momentul in care de la tastatura se introduce
// comanda "get_book" -- cere informatie despre o carte.
void get_book(struct Client* client) {
    client->j = nullptr;
    // introducerea id-ului cartii dorite
    cout << "id = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    // ruta de acces
    memset(client->url, 0, URL_SIZE);
    strcpy(client->url, "/api/v1/tema/library/books/");
    strcat(client->url, client->input);
    // crearea mesajului final ce trebuie trimis serverului + trimitere request
    // se observa trimiterea cu token-ul de acces JWT
    client->message = compute_get_request(client->host, client->url, 
        NULL, NULL, 0, client->jwt_token);
    send_to_server(client->sockfd, client->message);
    // primirea unui raspuns din partea serverului
    client->response = receive_from_server(client->sockfd);
    client->json_str = advanced_extract_json_response(client->response);
    // interpretarea raspunsului primit
    if (client->json_str == NULL) {
        client->json_str = basic_extract_json_response(client->response);
        // mesaj de eroare
        if (strncmp(client->json_str, "{\"error:", 7) == 0) {
            client->j = json::parse(client->json_str);
            cout << client->j["error"] << endl;
        }
        // alt mesaj necunoscut
        else {
            cout << "Something strange happened :(" << endl;
            cout << client->response << endl;
        }    
    }
    // afisare carte dorita
    else {
        client->j = json::parse(client->json_str);
        cout << endl << "Your book is:";
        print_book(client);
    }
    cout << endl;
}

// functie care se apeleaza in momentul in care de la tastatura se introduce
// comanda "add_book" -- adauga o carte.
void add_book(struct Client* client) {
    client->j = nullptr;
    // introducerea titlului
    cout << "title = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    client->j["title"] = client->input;
    // introducerea autorului
    cout << "author = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    client->j["author"] = client->input;
    // introducerea genului
    cout << "genre = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    client->j["genre"] = client->input;
    // introducerea editorului
    cout << "publisher = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    client->j["publisher"] = client->input;
    // introducerea numarului de pagini
    cout << "page_count = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    client->j["page_count"] = client->input;
    // ruta de acces
    memset(client->url, 0, URL_SIZE);
    strcpy(client->url, "/api/v1/tema/library/books");
    // content type
    memset(client->app_type, 0, APP_SIZE);
    strcpy(client->app_type, "json");
    // crearea mesajului final ce trebuie trimis serverului + trimitere request
    // se observa trimiterea cu token-ul de acces JWT
    client->message = compute_post_request(client->host, client->url, 
        client->app_type, client->j.dump().c_str(), NULL, 0, client->jwt_token);
    send_to_server(client->sockfd, client->message);
    // primirea unui raspuns din partea serverului
    client->response = receive_from_server(client->sockfd);
    client->json_str = basic_extract_json_response(client->response);
    // interpretarea raspunsului primit
    if(client->json_str == NULL) {
        cout << "The book was added succesfully" << endl;
    }
    // mesaj de eroare cel mai probabil
    else if(strncmp(client->json_str, "{\"error:", 7) == 0) {
        client->j = json::parse(client->json_str);    
        cout << client->j["error"] << endl;
    }
    cout << endl;
}

// functie care se apeleaza in momentul in care de la tastatura se introduce
// comanda "delete_book" -- sterge o carte.
void delete_book(struct Client* client) {
    // introducerea id-ului cartii dorite
    client->j = nullptr;
    cout << "id = ";
    memset(client->input, 0, INPUT_SIZE);
    getchar();
    scanf("%[^\n]", client->input);
    // ruta de acces
    memset(client->url, 0, URL_SIZE);
    strcpy(client->url, "/api/v1/tema/library/books/");
    // crearea mesajului final ce trebuie trimis serverului + trimitere request
    // se observa trimiterea cu token-ul de acces JWT
    strcat(client->url, client->input);
    client->message = compute_delete_request(client->host, client->url, 
        NULL, NULL, 0, client->jwt_token);
    send_to_server(client->sockfd, client->message);
    // primirea unui raspuns din partea serverului
    client->response = receive_from_server(client->sockfd);
    client->json_str = basic_extract_json_response(client->response);
    // interpretarea raspunsului primit
    if(client->json_str == NULL) {
        cout << "The book was deleted successfully!" << endl;
    }
    // mesaj de eroare cel mai probabil
    else {
        client->j = json::parse(client->json_str);
        cout << client->j["error"] << endl;
    }
    cout << endl;
}

// functie care se apeleaza in momentul in care de la tastatura se introduce
// comanda "logout" -- efectueaza logout.
void logout_command(struct Client* client) {
    client->j = nullptr;
    // ruta de acces
    memset(client->url, 0, URL_SIZE);
    strcpy(client->url, "/api/v1/tema/auth/logout");
    // introducem pentru verificare cookie-ul primit la logare
    char** cookies = (char **)malloc(sizeof(char *));
    cookies[0] = (char *)malloc(LOGIN_TOKEN_SIZE * sizeof(char));
    strcpy(cookies[0], client->login_token);
    // crearea mesajului final ce trebuie trimis serverului + trimitere request
    client->message = compute_get_request(client->host, 
        client->url, NULL, cookies, 1, NULL);
    send_to_server(client->sockfd, client->message);
    // primirea unui raspuns din partea serverului
    client->response = receive_from_server(client->sockfd);
    client->json_str = basic_extract_json_response(client->response);
    // interpretarea raspunsului primit
    if(client->json_str == NULL) {
        cout << "Client is now logged out!" << endl;
    }
    // mesaj de eroare cel mai probabil
    else {
        client->j = json::parse(client->json_str);
        cout << client->j["error"] << endl;
    }
    cout << endl;
}

// functie care ruleaza tot programul; practic, aici se face conexiunea prin
// deschiderea unui socket, sockfd, si se face si parsarea comenzilor pe care
// clientul le adauga de la tastatura; in functie de comenzile primite,
// programul va rula corespunzator
void run_program(struct Client* client) {
    while(1) {
        client->sockfd = open_connection(client->ip_addr,
            client->portno, AF_INET, SOCK_STREAM, 0);
        memset(client->input, 0, INPUT_SIZE);
        cin >> client->input;
        // aici se fac match-ul dintre input de la tastatura si comenzile valide
        if (strcmp(client->input, "register") == 0) {
            register_command(client);
        }
        else if (strcmp(client->input, "login") == 0) {
            login_command(client);
        }
        else if (strcmp(client->input, "enter_library") == 0) {
            enter_library(client);
        }
        else if (strcmp(client->input, "get_books") == 0) {
            get_books(client);
        }
        else if (strcmp(client->input, "get_book") == 0) {
            get_book(client);
        }
        else if (strcmp(client->input, "add_book") == 0) {
            add_book(client);
        }
        else if (strcmp(client->input, "delete_book") == 0) {
            delete_book(client);
        }
        // in cazul delogarii, se vor reseta campurile de jwt_token si 
        // login_token, din moment ce clientul nu mai are acces la ele
        else if (strcmp(client->input, "logout") ==0 ) {
            logout_command(client);
            memset(client->jwt_token, 0, JWT_SIZE);
            memset(client->login_token, 0, LOGIN_TOKEN_SIZE);
        }
        // iesire din program
        else if (strcmp(client->input, "exit") == 0) {
            exit(0);
        }
        // comanda inexistenta
        else {
            cout << "No such command. Please try again" << endl << endl;
        }
    }
    close(client->sockfd);
}

// un main prea incarcat si complicat; prea mult efort pentru a intelege ce se
// intampla in acesta; Only God knows... and I'm not sure about that neither
int main() {
    struct Client client;
    run_program(&client);
    return 0;
}