#include "doctor.h"

/* Função principal de um Médico. Deve executar um ciclo infinito onde em 
* cada iteração lê uma admissão e se data->terminate ainda for igual a 0, processa-a e
* escreve-a para os resultados. Admissões com id igual a -1 são ignoradas
* (ad inválida) e se data->terminate for igual a 1 é porque foi dada ordem
* de terminação do programa, portanto deve-se fazer return do número de 
* consultas realizadas. Para efetuar estes passos, pode usar os outros
* métodos auxiliares definidos em doctor.h.
*/
int execute_doctor(int doctor_id, struct data_container* data, struct communication* comm){
    while(1){
        struct admission* ad=(struct admission*)malloc(sizeof(struct admission));

        if(data->terminate==0){
           doctor_receive_admission( ad,  doctor_id,  data, comm);
           doctor_process_admission(ad, doctor_id, data);
        }
        if((*data->terminate)==1){
            return data->doctor_stats;
        }
        
    }
}

/* Função que lê uma admissão (do buffer de memória partilhada entre
* os rececionistas e os médicos) que seja direcionada ao médico doctor_id.
* Antes de tentar ler a admissão, o processo deve
* verificar se data->terminate tem valor 1.
* Em caso afirmativo, retorna imediatamente da função.
*/
void doctor_receive_admission(struct admission* ad, int doctor_id, struct data_container* data, struct communication* comm){
    if((*data->terminate)==1)
     return;

    read_receptionist_doctor_buffer(comm->receptionist_doctor,doctor_id, data->buffers_size, ad);
}

/* Função que processa uma admissão, alterando o seu campo receiving_doctor para o id
* passado como argumento, alterando o estado da mesma para 'A' e 
* incrementando o contador de consultas no data_container ou para 'N'. 
* Atualiza também a admissão na estrutura data.
*/
void doctor_process_admission(struct admission* ad, int doctor_id, struct data_container* data){
    ad->receiving_doctor=doctor_id;
    if((*data->doctor_stats) < data->max_ads){
        ad->status='A';
        (*data->doctor_stats)++;
        data->results=ad;
    }

    ad->status='N';
}

