#include <stdlib.h>
#include <stdio.h>
#include "receptionist.h"

/* Função principal de um Rececionista. Deve executar um ciclo infinito onde em
* cada iteração lê uma admissão dos pacientes e se a mesma tiver id
* diferente de -1 e se data->terminate ainda for igual a 0, processa-a e
* a encaminha para os médicos. Admissões com id igual a -1 são
* ignoradas (ad inválida) e se data->terminate for igual a 1 é porque foi
* dada ordem de terminação do programa, portanto deve-se fazer return do
* número de admissões realizadas. Para efetuar estes passos, pode usar os
* outros métodos auxiliares definidos em receptionist.h.
*/
int execute_receptionist(int receptionist_id, struct data_container* data, struct communication* comm){
    while(1){
        struct admission* ad=(struct admission*)malloc(sizeof(struct admission));
 
        if(ad->id!=-1 && (*data->terminate)==0){
            receptionist_receive_admission(ad, data,comm);
            receptionist_process_admission(ad, receptionist_id,data);
            receptionist_send_admission(ad,data,comm);
        }
        if((*data->terminate)==1){
            return data->receptionist_stats;
        }
        

    }
}

/* Função que lê uma admissão do buffer de memória partilhada entre os pacientes e os rececionistas.
* Antes de tentar ler a admissão, deve verificar se data->terminate tem valor 1.
* Em caso afirmativo, retorna imediatamente da função.
*/
void receptionist_receive_admission(struct admission* ad, struct data_container* data, struct communication* comm){
    if((*data->terminate)==1){
        return;
    }

    read_patient_receptionist_buffer(comm->patient_receptionist,  data->buffers_size, ad);
}
/* Função que realiza uma admissão, alterando o seu campo receiving_receptionist para o id
* passado como argumento, alterando o estado da mesma para 'R' (receptionist), e
* incrementando o contador de admissões realizadas por este rececionista no data_container.
* Atualiza também a admissão na estrutura data.
*/
void receptionist_process_admission(struct admission* ad, int receptionist_id, struct data_container* data){
    ad->receiving_receptionist=receptionist_id;
    ad->status='R';
    (*data->receptionist_stats++);
   /*int length= sizeof(data->results)/sizeof(struct admission);
    int new_length = length+1;
    struct admission *temp= (struct admission*)realloc(data->results,  new_length * sizeof(struct admission));
    if (temp == NULL) {
        printf("Error al redimensionar el vector.\n");
        free(data->results);
        return 1;
    } else {
        data->results = temp;
    }*/

    data->results=ad;
}

/* Função que escreve uma admissão no buffer de memória partilhada entre
* os rececionistas e os médicos.
*/
void receptionist_send_admission(struct admission* ad, struct data_container* data, struct communication* comm){
    write_receptionist_doctor_buffer(comm->receptionist_doctor,  data->buffers_size, ad);
}
