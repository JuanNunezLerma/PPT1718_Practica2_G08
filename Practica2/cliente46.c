/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 2.0
Fecha: 09/2017
Descripción:
Cliente sencillo TCP.

Autor: Juan Núñez Lerma / Pedro Javier Sáez Mira

*******************************************************/
#include <stdio.h>
#include <ws2tcpip.h>//Necesaria para las funciones IPv6
#include <conio.h>
#include "protocol.h"

#pragma comment(lib, "Ws2_32.lib")

int main(int *argc, char *argv[])
{
	SOCKET sockfd;
	struct sockaddr *server_in;
	struct sockaddr_in server_in4;
	struct sockaddr_in6 server_in6;
	struct in_addr address;
	int address_size = sizeof(server_in4);
	char buffer_in[1024], buffer_out[1024], input[1024];
	int recibidos = 0, enviados = 0;
	int estado = S_WELC;
	char option;
	int ipversion = AF_INET;//IPv4 por defecto
	char ipdest[256];
	char ipdestl;
	char default_ip4[16] = "127.0.0.1";
	char default_ip6[64] = "::1";
	boolean flag = 0, flagDest = 0; //Variable bandera para controlar la parte de los dominios

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	//Inicialización Windows sockets - SOLO WINDOWS
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
		return(0);

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	printf("**************\r\nCLIENTE TCP SENCILLO SOBRE IPv4 o IPv6\r\n*************\r\n");


	do {

		printf("CLIENTE> ¿Qué versión de IP desea usar? 6 para IPv6, 4 para IPv4 [por defecto] ");
		gets_s(ipdest, sizeof(ipdest));

		if (strcmp(ipdest, "6") == 0) {
			ipversion = AF_INET6;

		}
		else { //Distinto de 6 se elige la versión 4
			ipversion = AF_INET;
		}

		sockfd = socket(ipversion, SOCK_STREAM, 0);
		if (sockfd == INVALID_SOCKET) {
			printf("CLIENTE> ERROR\r\n");
			exit(-1);
		}
		else {
			do { //while para mantenernos en el bucle en caso de que el dominio no exista o no se haya introducido.
				printf("Introduzca la direccion IP o el dominio destino: ");
				gets(ipdest);

				ipdestl = inet_addr(ipdest);
				if (ipdestl == INADDR_NONE) {
					//La dirección introducida por teclado no es correcta o
					//corresponde con un dominio.
					struct hostent *host;
					host = gethostbyname(ipdest);
					if (host != NULL) {
						flag = 0; //Si el dominio resuelve debe salirse, ya que existe.
						memcpy(&address, host->h_addr_list[0], 4);
						printf("\nDireccion %s\n", inet_ntoa(address));
					}
					else {
						flag = 1; //Si no resuelve vuelve a pedirnos la introduccion de la dirección o dominio.
					}
				}
			} while (flag == 1);

			//Dirección por defecto según la familia
			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET)
				strcpy_s(ipdest, sizeof(ipdest), default_ip4);
			else
				strcpy_s(ipdest, sizeof(ipdest), inet_ntoa(address));//Si no es la de por defecto, copiamos la que ha resuelto en ipdest.

			if (strcmp(ipdest, "") == 0 && ipversion == AF_INET6)
				strcpy_s(ipdest, sizeof(ipdest), default_ip6);
			else
				strcpy_s(ipdest, sizeof(ipdest), inet_ntoa(address));//Si no es la de por defecto, copiamos la que ha resuelto en ipdest.

			if (ipversion == AF_INET) {
				server_in4.sin_family = AF_INET;
				server_in4.sin_port = htons(SMTP_SERVICE_PORT);
				//server_in4.sin_addr.s_addr=inet_addr(ipdest);
				inet_pton(ipversion, ipdest, &server_in4.sin_addr.s_addr);
				server_in = (struct sockaddr*)&server_in4;
				address_size = sizeof(server_in4);
			}

			if (ipversion == AF_INET6) {
				memset(&server_in6, 0, sizeof(server_in6));
				server_in6.sin6_family = AF_INET6;
				server_in6.sin6_port = htons(SMTP_SERVICE_PORT);
				inet_pton(ipversion, ipdest, &server_in6.sin6_addr);
				server_in = (struct sockaddr*)&server_in6;
				address_size = sizeof(server_in6);
			}

			estado = S_WELC; 

			if (connect(sockfd, server_in, address_size) == 0) {
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n", ipdest, SMTP_SERVICE_PORT);

				//Inicio de la máquina de estados
				do {
					switch (estado) {
					case S_WELC:
						// Se recibe el mensaje de bienvenida
						break;
					case S_HELO:
						// establece la conexion de aplicacion 
						printf("Bienvenido al servicio de correo electronico");
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", HELO, ipdest, CRLF); //Mandamos el comando HELO al servidor de correo
						break;
					case S_MF:
						printf("Introduzca el REMITENTE (enter para salir): ");
						gets_s(input, sizeof(input)); //Guardamos el remitente.
						//strcpy(rmt,input);
						if (strlen(input) == 0) { //Si pulsa enter, sale.
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
							estado = S_QUIT;
						}
						else
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", MF, input, CRLF); //Enviamos el remitente formateado.
						break;
					case S_RCPT:
						do {
							printf("Introduzca el DESTINATARIO (enter para salir): ");
							gets_s(input, sizeof(input)); //Guardamos el destinatario.
							if (strlen(input) == 0) { //Si pulsa enter, sale.
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", SD, CRLF);
								estado = S_QUIT;
							}
							else {
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s%s", RCPT, input, CRLF); //Enviamos el destinatario formateado.
								enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
							}
							printf("¿Desea enviar el email a otro destinatario? SI (S/s) / NO (Pulsa cualquier tecla): ");
							gets_s(input, sizeof(input));
							if ((strcmp(input, "s") == 0) || (strcmp(input, "S") == 0)) { //Si introduce opción de enviar el email a otro destinatario volvemos al bucle.
								flagDest = 1;
							}
							else {
								flagDest = 0;
							}
						}while (flagDest == 1);
						break;
					case S_DATA: //caso para entrar en la escritura del mensaje.
						sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", DATA, CRLF); //Mandamos el comando DATA.
						estado++; //Cambio de estado.
						break;
					case S_ENVIA: //Empezamos a formatear cabeceras y cuerpo del mensaje
						printf("Introduzca el ASUNTO del mensaje: ");					
						gets_s(input, sizeof(input)); //Guardamos asunto.
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", SUBJECT, input, CRLF); //Asunto formateado.
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0); //Envio Asunto.

						printf("Introduzca el NOMBRE del REMITENTE del correo: ");
						gets_s(input, sizeof(input)); //Guardamos remitente
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s", RMT, input, CRLF); //Remitente formateado.
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0); //Envio remitente

						printf("Introduzca el NOMBRE del DESTINATARIO del correo: ");
						gets_s(input, sizeof(input)); //Nombre del destinatario.
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %s%s%s", TO, input, CRLF, CRLF); //Nombre formateado.
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0); //Envio nombre.

						printf("Introduzca el CUERPO del mensaje (. y enter para acabar la redaccion): ");
						gets_s(input, sizeof(input)); //Empieza el cuerpo del mensaje.
						sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF); //Cuerpo formateado.
						enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0); //Envio cuerpo.

						while (strcmp(input, ".") != 0) { //El cuerpo se va mandando hasta el momento que escriba un punto. Al escribir un punto solo, sale y termina.
							gets_s(input, sizeof(input));
							sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);	
							enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						}

						printf("¿Deseas enviar otro correo? SI (S/s) / NO (Pulsa cualquier tecla): ");
						gets_s(input, sizeof(input));
						if ((strcmp(input,"s")==0) || (strcmp(input, "S") == 0)){ //Si introduce opción de enviar otro correo vuelve al estado inicial sin salir.
							estado = S_WELC;
						}	
						break;
					}

					if (estado != S_WELC && estado != S_RCPT) { //Se ha añadido la parte de que sea distinto del estado S_RCPT, para conseguir enviar varios destinatarios, concatenados. El envío se realiza
					//en el estado S_RCPT	//Soporte para el comando RESET
						if ((strcmp(buffer_out, "MAIL FROM:RSET\r\n") == 0) || (strcmp(buffer_out, "RCPT TO:RSET\r\n") == 0)) { //Comprobamos que lo que ha introducido no sea RSET. Al venir formateado
							//le añadimos el formato. En caso de que sea, vuelve al estado inicial.
							estado = S_WELC;
						}
						else {
							enviados = send(sockfd, buffer_out, (int)strlen(buffer_out), 0);
						}

						if (enviados == SOCKET_ERROR) {//Aqui tambien controla el error
							estado = S_QUIT;
							continue;
						}
					}

					if ((strcmp(buffer_out, "MAIL FROM:RSET\r\n") != 0) && (strcmp(buffer_out, "RCPT TO:RSET\r\n") != 0)) { //Se añade comprobación para el comando RSET, ya que en ese caso, no recibe nada.
						recibidos = recv(sockfd, buffer_in, 512, 0);
					}
					if (recibidos <= 0) {//Aqui sirve para controlar los errores de transporte
						DWORD error = GetLastError();
						if (recibidos<0) {
							printf("CLIENTE> Error %d en la recepción de datos\r\n", error);
							estado = S_QUIT;
						}
						else {
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado = S_QUIT;
						}
					}
					else {
						buffer_in[recibidos] = 0x00;//Esto se añade para que el mensaje no falle ya que en C los datos tienen que terminar en cero.
						printf(buffer_in);
						if (estado != S_DATA && strncmp(buffer_in, "2", 1) == 0)
							estado++;
					}

				} while (estado != S_QUIT);
			}
			else {
				int error_code = GetLastError();
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n", ipdest, SMTP_SERVICE_PORT);
			}
			// fin de la conexion de transporte
			closesocket(sockfd);

		}
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option = _getche();

	} while (option != 'n' && option != 'N');

	WSACleanup();
	return(0);
}
