#include "../include/exit_process.h"


void planificar_proceso_exit_en_hilo(pcb* un_pcb){
	ejecutar_en_hilo_detach((void *)planificar_proceso_exit, un_pcb);
}

//QUIZÁS SERÍA MEJOR QUE YO LE PASE EL ESTADO Y NO SE LO PREGUNTE AL PCB
void planificar_proceso_exit(pcb *un_pcb) 
{
	switch (un_pcb->estado)
	{

	case NEW:

		if (_eliminar_pcb_de_lista_sync(un_pcb, new, &mutex_lista_new))
		{	
			agregar_int_a_lista(lista_exit,un_pcb->pid);
			destruir_pcb(un_pcb);
		}

		break;

	case READY:

		switch (ALGORITMO_PCP_SELECCIONADO)
		{
		case VRR:

			if (_eliminar_pcb_de_lista_sync(un_pcb, ready, &mutex_lista_ready)){
			
				_gestionar_salida(un_pcb);

			}else if (_eliminar_pcb_de_lista_sync(un_pcb, ready_plus, &mutex_lista_ready_plus)){

				_gestionar_salida(un_pcb);
				
			}
			break;

		default:

			if (_eliminar_pcb_de_lista_sync(un_pcb, ready, &mutex_lista_ready)){
				_gestionar_salida(un_pcb);
			}
			break;
		}

		break;

	case BLOCKED:

		if (_eliminar_pcb_de_lista_sync(un_pcb, blocked, &mutex_lista_blocked)){
			_gestionar_salida(un_pcb);
		}

		break;

	case EXEC: 

		_gestionar_interrupcion(un_pcb, EXIT_PROCESS);

		break;

	case EXIT: 

		if (_eliminar_pcb_de_lista_sync(un_pcb, execute, &mutex_lista_exec)){
			_gestionar_salida(un_pcb);
		}

		break;

	default:
		log_error(kernel_logger_extra,"ERROR: Se ha intentado borrar un proceso con estado indefinido");
		exit(EXIT_FAILURE);
		break;
	}
}

void _gestionar_salida(pcb* un_pcb){

	agregar_int_a_lista(lista_exit,un_pcb->pid);
	liberar_recursos_pcb(un_pcb);
	destruir_pcb(un_pcb);
	sem_post(&sem_multiprogramacion);
	decrementar_procesos_en_core();

}

//////////// FUNCIONES MANEJO DE RECURSOS EN EXIT

void liberar_recursos_pcb (pcb* un_pcb){
	liberar_memoria(un_pcb);
	// Requisitos:
	// - Cuando un pcb ya se le asignó recurso debe quedar con *pedido_recurso = NULL (Sino sigue en lista de espera)
	liberar_recursos(un_pcb);
	// Requisitos:
	// - Cuando se completí isntrucción pcb debe quedar con *pedido_a_interfaz->nombre_interfaz = NULL (Sino sigue en lista de espera)
	liberar_interfaces(un_pcb);
}

/////// RECURSOS

void liberar_recursos(pcb* un_pcb){

	if(un_pcb->estado == BLOCKED && un_pcb->pedido_recurso != NULL){
		eliminar_de_lista_recurso(un_pcb);
	}
	while(list_size(un_pcb->recursos_en_uso)>0){
			
		instancia_recurso_pcb* un_recurso = list_remove(un_pcb->recursos_en_uso,0);
		_signal_recurso_exit(un_recurso->nombre_recurso,un_recurso->instancias_recurso);
		
	}	
}

void eliminar_de_lista_recurso(pcb* un_pcb){
	
	bool _buscar_recurso(instancia_recurso* recurso_encontrado)
	{
		return strcmp(recurso_encontrado->nombre_recurso,un_pcb->pedido_recurso)== 0;
	}

	bool _buscar_pcb(pcb* otro_pcb)
	{
		return otro_pcb->pid == un_pcb->pid;
	}

	pthread_mutex_lock(&mutex_lista_recursos);

	if(list_any_satisfy(lista_recursos,(void *)_buscar_recurso)){
		instancia_recurso* un_recurso = list_find(lista_recursos,(void *)_buscar_recurso);

		pthread_mutex_lock(&un_recurso->mutex_lista_procesos_en_cola);
		if(list_any_satisfy(un_recurso->lista_procesos_en_cola,(void *)_buscar_pcb)){
			list_remove_by_condition(un_recurso->lista_procesos_en_cola,(void *)_buscar_pcb);
		}
		pthread_mutex_unlock(&un_recurso->mutex_lista_procesos_en_cola);
	}
		
	pthread_mutex_unlock(&mutex_lista_recursos);
}

void _signal_recurso_exit(char* nombre_recurso, int cantidad_instanciada){
	
	bool _buscar_recurso(instancia_recurso* recurso_encontrado)
	{
		return strcmp(recurso_encontrado->nombre_recurso,nombre_recurso)== 0;
	}

	pthread_mutex_lock(&mutex_lista_recursos);
	
	if(list_any_satisfy(lista_recursos,(void *)_buscar_recurso)){
		instancia_recurso* un_recurso = list_find(lista_recursos,(void *)_buscar_recurso);
		while(cantidad_instanciada>0){
		sem_post(&un_recurso->semaforo_recurso);
		cantidad_instanciada--;
		}
	}

	pthread_mutex_unlock(&mutex_lista_recursos);
}

/////// INTERFACES

void liberar_interfaces(pcb* un_pcb){
	if(un_pcb->estado == BLOCKED && un_pcb->pedido_a_interfaz->nombre_interfaz != NULL){
		eliminar_de_lista_interfaz(un_pcb);
	}
}

void eliminar_de_lista_interfaz(pcb* un_pcb){
	
	bool _buscar_interfaz(interfaz* interfaz_encontrada)
	{
		return strcmp(interfaz_encontrada->nombre_interfaz,un_pcb->pedido_a_interfaz->nombre_interfaz)== 0;
	}

	bool _buscar_pcb(pcb* otro_pcb)
	{
		return otro_pcb->pid == un_pcb->pid;
	}

	pthread_mutex_lock(&mutex_lista_interfaces);

	if(list_any_satisfy(interfaces_conectadas,(void *)_buscar_interfaz)){
		interfaz* una_interfaz = list_find(interfaces_conectadas,(void *)_buscar_interfaz);

		pthread_mutex_lock(&una_interfaz->mutex_interfaz);
		if(list_any_satisfy(una_interfaz->lista_procesos_en_cola,(void *)_buscar_pcb)){
			list_remove_by_condition(una_interfaz->lista_procesos_en_cola,(void *)_buscar_pcb);
		}
		pthread_mutex_unlock(&una_interfaz->mutex_interfaz);
	}	
	pthread_mutex_unlock(&mutex_lista_interfaces);
}