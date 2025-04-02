#include"utils.h"

#define SERV_MAX_CONNECTIONS 10

t_log* logger;

int iniciar_servidor(void){
	int fd_escucha;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int errno = getaddrinfo(NULL, PUERTO, &hints, &servinfo);

	if(errno != 0){
		log_error(logger, "Se ha producido un error al ejecutar getaddrinfo. Errno: %s", errno);
		abort();
	}

	// Creamos el socket de escucha del servidor
	fd_escucha = socket(
		servinfo->ai_family,
		servinfo->ai_socktype,
		servinfo->ai_protocol
	);

	errno = setsockopt(fd_escucha, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	if(errno != 0){
		log_error(logger, "Se ha producido un error al ejecutar setsockopt. Errno: %s", errno);
		abort();
	}

	// Asociamos el socket a un puerto
	errno = bind(fd_escucha, servinfo->ai_addr, servinfo->ai_addrlen);
	if(errno != 0){
		log_error(logger, "Se ha producido un error al bindear el puerto al socket. Errno: %s", errno);
		abort();
	}
		
	// Escuchamos las conexiones entrantes
	errno = listen(fd_escucha, SERV_MAX_CONNECTIONS);

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return fd_escucha;
}

int esperar_cliente(int socket_servidor){
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente){
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
