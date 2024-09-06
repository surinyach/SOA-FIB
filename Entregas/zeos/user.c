#include <libc.h>
#include <io.h>

char buff[24];
char *text = "\nEn un tranquilo trigal, tres traviesos tigres trotaban, tejian trampas, tomaban trigo. Tan tentador, todo temblaba. Torcieron, torbellino trajo treinta tucanes, todos tratando de tocar tejados. Tras tanto trajin, tres tigres, totalmente tornados, treparon torres, triunfaron. Tramaron travesuras, transformaron el tranquilo trigal en tumultuoso tsunami.\n";

int pid;

void spacer(int spaces) {
  for (int i = 0; i < spaces; ++i) write(1, "\n", 1); 
}

/*
void test_write() {
  int status = 0; 
  status = write(100, "\nBon dia!", 9); // fd error
  if (status < 0) perror(); 
  write(1, NULL, 0); // NULL buffer error
  if (status < 0) perror(); 
  char *memory;
  memory = (char *)0x00110000; 
  write(1, memory, 15) ; // Invalid memory access error
  if (status < 0) perror();  
  write(1, "\nBon dia!", -9); // Negative size error
  if (status < 0) perror(); 
  spacer(9); 
  write(1, "\nBon dia!", 9); // Correct write
  write(1, text, strlen(text)); // Correct write, size > buffer (512) 
}*/


/*
void test_get_time() {
  while(gettime() < 5000){}
  write(1,"\n5000",5);
}
*/

void test_entrega2() {
  char buffer[256];
  pid = fork();
  int a;

  if (pid == 0) {
    a = getpid();
    itoa(a, buffer);
    write(1, " \nSOY EL HIJO, TENGO PID:", strlen(" \nSOY EL HIJO, TENGO PID:")); 
    write (1, buffer, 256);
    write (1," \nSOY EL HIJO, ME BLOQUEO", strlen(" \nSOY EL HIJO, ME BLOQUEO"));
    block();
    write(1, " \nSOY EL HIJO, ME HE DESBLOQUEADO", strlen(" \nSOY EL HIJO, ME HE DESBLOQUEADO")); 
    write(1, " \nSOY EL HIJO, ME MUERO :(", strlen(" \nSOY EL HIJO, ME MUERO :(")); 
    exit(); 
  }

  else {
    a = getpid();
    itoa(a, buffer);
    write (1," \nSOY EL PADRE, TENGO PID:", strlen(" \nSOY EL PADRE, TENGO PID:"));
    write (1, buffer, 256);
    while(gettime() < 2000); 
    unblock(96);
    while(gettime() < 8000); 
    write(1, " \nSE HA MUERTO MI HIJO :(", strlen(" \nSE HA MUERTO MI HIJO :("));
    write(1, " \nSOY EL PADRE, ME MUERO YO", strlen(" \nSOY EL PADRE, ME MUERO YO")); 
    exit(); 
    // Si no se escribe, se pasa correctamente a idle. 
    write(1, " CHECK IDLE", strlen(" CHECK IDLE")); 
  }
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{

  //test_write(); 
  //test_get_time(); 
  test_entrega2(); 

  while(1) { }
}


 