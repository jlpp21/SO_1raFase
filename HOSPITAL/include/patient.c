#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "memory.h"


/* Função principal de um Paciente. Deve executar um ciclo infinito onde em 
* cada iteração lê uma admissão da main e se data->terminate ainda 
* for igual a 0, processa-a. Admissões com id igual a -1 são ignoradas
* (ad inválida) e se data->terminate for igual a 1 é porque foi dada ordem
* de terminação do programa, portanto deve-se fazer return do número de 
* admissões pedidas. Para efetuar estes passos, pode usar os outros
* métodos auxiliares definidos em patient.h.
*/
int execute_patient(int patient_id, struct data_container* data, struct communication* comm){
    struct admission ad = {0, 0, 0, 0, 0, 0};
    int op_counter = 0;
    while ((*data->terminate) == 0) {
        patient_receive_admission(&ad, patient_id, data, comm);
        if (ad.id == -1){
            continue;
        }
        printf(INFO_RECEIVED_OP, "Client", client_id, ad.id);
        patient_process_admission(&ad, patient_id, data);
        patient_send_admission(&ad, data, comm);
    }
    return 0;
}
/* Função que lê uma admissão (do buffer de memória partilhada entre a main
* e os pacientes) que seja direcionada a patient_id. Antes de tentar ler a admissão, deve
* verificar se data->terminate tem valor 1. Em caso afirmativo, retorna imediatamente da função.
*/
void patient_receive_admission(struct admission* ad, int patient_id, struct data_container* data, struct communication* comm){
    if (*(data->terminate) == 1) return;
    read_main_patient_buffer(comm->main_patient, patient_id, data->buffers_size, ad);
}
/* Função que valida uma admissão, alterando o seu campo receiving_patient para o patient_id
* passado como argumento, alterando o estado da mesma para 'P' (patient), e 
* incrementando o contador de admissões solicitadas por este paciente no data_container. 
* Atualiza também a admissão na estrutura data.
*/
void patient_process_admission(struct admission* ad, int patient_id, struct data_container* data){
    ad->receiving_patient = patient_id; // update receiving patient field
    ad->status = 'P'; // change status to 'P' (processed by patient)
    data->op_counter++; // increment counter of this patient's processed operations
    memcpy(&data->results[ad->id], ad, sizeof(struct admission));
}
/* Função que escreve uma admissão no buffer de memória partilhada entre os
* pacientes e os rececionistas.
*/
void patient_send_admission(struct admission* ad, struct data_container* data, struct communication* comm){
     write_patient_receptionist_buffer(comm->client_interm, data->buffers_size, ad);
}