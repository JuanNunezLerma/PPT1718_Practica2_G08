#ifndef protocolostpte_practicas_headerfile
#define protocolostpte_practicas_headerfile
#endif

// COMANDOS DE APLICACION
#define HELO "HELO"  // SOLICITUD DE CONEXION USER usuario 
#define MF "MAIL FROM:"
#define RCPT "RCPT TO:"
#define PW "PASS"  // Password del usuario  PASS password
#define SC "USER"
#define DATA "DATA"
#define SUBJECT "SUBJECT:"
#define RMT "FROM:"
#define TO "TO:"

#define SD  "QUIT"  // Finalizacion de la conexion de aplicacion
//#define SD2 "EXIT"  // Finalizacion de la conexion de aplicacion 
#define ECHO "ECHO" // Definicion del comando "ECHO" para el servicio de eco


// RESPUESTAS A COMANDOS DE APLICACION
#define OK  "OK"
#define OKDATA "3"
#define ER  "5"

//FIN DE RESPUESTA
#define CRLF "\r\n"

//ESTADOS
#define S_WELC 0
#define S_HELO 1
#define S_MF 2
#define S_RCPT 3
#define S_DATA 4
#define S_ENVIA 5
#define S_QUIT 6
#define S_EXIT 7

//PUERTO DEL SERVICIO
#define SMTP_SERVICE_PORT	25

// NOMBRE Y PASSWORD AUTORIZADOS
#define USER		"alumno"
#define PASSWORD	"123456"

#define mod "mod"