Para evitar as situações de competição  (race conditions) no acesso a elementos
partilhados usamos mutexes e também tiramos proveito do uso das variáveis de condição.

Apenas existem race conditions no programa sauna:
A variável partilhada para a qual existem RC é um struct criado por nós
cujo código está no ficheiro shared.h

typedef struct {
  unsigned long total;
  unsigned long free;
  char gender;
} sauna_t;

Sempre que queremos aceder a este struct o thread em questão adquire o mutex e, normalmente
executa a secção crítica, à exceção da main thread quando a sauna está cheia, e o pedido tem
o mesmo género que a sauna, nesse caso usamos a váriavel de condição:

while (!sauna.free){
  pthread_cond_wait(&cvar, &mut);
}

e executamos a secção crítica.

Esta variável de condição é controlada pelos spawned threads que depois de cumprirem o pedido
chamam a função freeSlot () que liberta o seu lugar na sauna e desbloqueiam a main thread:

void freeSlot (){
  pthread_mutex_lock (&mut);
  sauna.free ++;
  pthread_cond_signal(&cvar);
  pthread_mutex_unlock (&mut);
}

T1G06
João Pedro Teixeira Pereira de Sá up201506252
Bernardo Manuel Costa Barbosa up201503477
Duarte Nuno Esteves André Lima de Carvalho up201503661
