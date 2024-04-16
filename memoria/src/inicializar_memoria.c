#include "/home/utnso/Documents/tp-2024-1c-ubuntunel/memoria/include/memoria.h"
#include "/home/utnso/Documents/tp-2024-1c-ubuntunel/memoria/include/inicializar_memoria.h"

char* path_config_Memoria = "/home/utnso/Documents/tp-2024-1c-ubuntunel/memoria/Memoria.config";

void inicializar_memoria(){
    inicializar_logs();
    iniciar_config();
    imprimir_config();
}

void inicializar_logs(){

    // INCIA MEMORIA
    memoria_logger = log_create("Memoria.log", "Memoria_log", true, LOG_LEVEL_INFO);
    if (memoria_logger == NULL){
        perror ("Error: No se pudo crear log para la memoria");
        exit(EXIT_FAILURE);
    }

    memoria_logger_extra = log_create("Memoria_extra_log.log", "Memoria_extra_log", true, LOG_LEVEL_TRACE);
    if (memoria_logger == NULL){
        perror ("No se pudo crear log extra para la memoria");
        exit(EXIT_FAILURE);
    }
}


void iniciar_config(){

    memoria_config = config_create(path_config_Memoria);
    if (memoria_config == NULL) {
        perror ("Error: No se pudo crear el config para la memoria");
		exit(EXIT_FAILURE);
	}

    PUERTO_ESCUCHA = config_get_string_value(memoria_config,"PUERTO_ESCUCHA");
    TAM_MEMORIA = config_get_int_value(memoria_config,"TAM_MEMORIA");
    TAM_PAGINA = config_get_int_value(memoria_config,"TAM_PAGINA");
    PATH_INSTRUCCIONES= config_get_string_value(memoria_config,"PATH_INSTRUCCIONES");
    RETARDO_RESPUESTA = config_get_int_value(memoria_config,"RETARDO_RESPUESTA");

}

void imprimir_config(){

    log_info(memoria_logger,"PATH INSTRUCCIONES: %s",PATH_INSTRUCCIONES);
}