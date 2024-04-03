#include "memory.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
/* Função que reserva uma zona de memória partilhada com tamanho indicado
* por size e nome name, preenche essa zona de memória com o valor 0, e 
* retorna um apontador para a mesma. Pode concatenar o resultado da função
* getuid() a name, para tornar o nome único para o processo.
*/
void* create_shared_memory(char* name, int size){
     uid_t uid = getuid();
    char name_uid[strlen(name)+10];
    sprintf(name_uid,"%s_%d", name, uid);
 
    int fd = shm_open(name, O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR); 
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    int *shmem_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
     if (shmem_ptr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return shmem_ptr;
}

/* Função que reserva uma zona de memória dinâmica com tamanho indicado
* por size, preenche essa zona de memória com o valor 0, e retorna um 
* apontador para a mesma.
*/
void* allocate_dynamic_memory(int size){
    void* ptr = malloc(size);
    if (ptr == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    memset(ptr, 0, size);
    return ptr;
}

/* Função que liberta uma zona de memória partilhada previamente reservada.
*/
void destroy_shared_memory(char* name, void* ptr, int size){
     int uid = getuid();
    char name_uid[strlen(name)+10];
    sprintf(name_uid,"%s_%d", name, uid);

   if (munmap(ptr, size) == -1) {
        perror("munmap");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(name_uid) == -1) {
        perror("shm_unlink");
        exit(EXIT_FAILURE);
    }

}

/* Função que liberta uma zona de memória dinâmica previamente reservada.
*/
void deallocate_dynamic_memory(void* ptr){
    free(ptr);
}
void write_operation_to_rnd_access_buffer(struct circular_buffer* buffer, int buffer_size, struct admission* ad) {
    for (int i = 0; i < buffer_size; i++) {
        // check if the ith position in the buffer is free
        if (buffer->ptrs[i] == FREE_MEM) {
            // write the operation to the buffer and update pointer to 1 (used)
            buffer->buffer[i] = *ad;
            buffer->ptrs[i] = USED_MEM;
            return; // exit the function
        }
    }   
}


/* Função que escreve uma admissão no buffer de memória partilhada entre a Main
* e os pacientes. A admissão deve ser escrita numa posição livre do buffer, 
* tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
* Se não houver nenhuma posição livre, não escreve nada.
*/
void write_main_patient_buffer(struct circular_buffer* buffer, int buffer_size, struct admission* ad){
 int next_index = (buffer->ptrs->in + 1) % buffer_size;

    // return if buffer is full
    if (next_index == buffer->ptrs->out)
        return;

    // write the operation to the buffer at the next available index
    buffer->buffer[buffer->ptrs->in] = *ad;

    // update the write pointer to the next available index
    buffer->ptrs->in = next_index;
}

/* Função que escreve uma admissão no buffer de memória partilhada entre os pacientes
* e os rececionistas. A admissão deve ser escrita numa posição livre do buffer, 
* tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
* Se não houver nenhuma posição livre, não escreve nada.
*/
void write_patient_receptionist_buffer(struct rnd_access_buffer* buffer, int buffer_size, struct admission* ad){
write_operation_to_rnd_access_buffer(buffer, buffer_size, ad);
}

/* Função que escreve uma admissão no buffer de memória partilhada entre os rececionistas
* e os médicos. A admissão deve ser escrita numa posição livre do buffer, 
* tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
* Se não houver nenhuma posição livre, não escreve nada.
*/
void write_receptionist_doctor_buffer(struct circular_buffer* buffer, int buffer_size, struct admission* ad){
 int next_index = (buffer->ptrs->in + 1) % buffer_size;

    // return if buffer is full
    if (next_index == buffer->ptrs->out)
        return;

    // write the operation to the buffer at the next available index
    buffer->buffer[buffer->ptrs->in] = *ad;

    // update the write pointer to the next available index
    buffer->ptrs->in = next_index;
}

/* Função que lê uma admissão do buffer de memória partilhada entre a Main
* e os pacientes, se houver alguma disponível para ler que seja direcionada ao paciente especificado.
* A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
* Se não houver nenhuma admissão disponível, afeta ad->id com o valor -1.
*/
void read_main_patient_buffer(struct circular_buffer* buffer, int patient_id, int buffer_size, struct admission* ad){
if (buffer->ptrs->in == buffer->ptrs->out) {
        op->id = -1;
        return;
    }
    // read the operation from the buffer
    *ad = buffer->buffer[buffer->ptrs->out];
    // update the out pointer if reading was successful
    buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
}

/* Função que lê uma admissão do buffer de memória partilhada entre os pacientes e rececionistas,
* se houver alguma disponível para ler (qualquer rececionista pode ler qualquer admissão).
* A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo.
* Se não houver nenhuma admissão disponível, afeta ad->id com o valor -1.
*/
void read_patient_receptionist_buffer(struct rnd_access_buffer* buffer, int buffer_size, struct admission* ad){
    for (int i = 0; i < buffer_size; i++) {
        // check if the ith position in the buffer is in used
        if (buffer->ptrs[i] == USED_MEM) {
            if (buffer->buffer[i].requested_enterp == enterp_id) {
                // point op to buffered op and update ith position to free
                *ad = buffer->buffer[i];
                buffer->ptrs[i] = FREE_MEM;
                return; // exit the function
            }
        }
    }
    // no op available
    ad->id = -1;
}

/* Função que lê uma admissão do buffer de memória partilhada entre os rececionistas e os médicos,
* se houver alguma disponível para ler dirigida ao médico especificado. A leitura deve
* ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo. Se não houver
* nenhuma admissão disponível, afeta ad->id com o valor -1.
*/
void read_receptionist_doctor_buffer(struct circular_buffer* buffer, int doctor_id, int buffer_size, struct admission* ad){
    if (buffer->ptrs->in == buffer->ptrs->out) {
        ad->id = -1;
        return;
    }
    // read the operation from the buffer
    *ad = buffer->buffer[buffer->ptrs->out];
    // update the out pointer if reading was successful
    buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
}