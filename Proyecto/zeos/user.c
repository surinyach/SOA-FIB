#include <libc.h>
#include <stddef.h>
#include <list.h>

char buff[16];

int pid;

int fps = 0;
int points = 0;
int lifes = 3;

struct position {
  int col; 
  int row;
  char previous;  
}; 

struct position pacman_pos; 
struct position blue;
struct position pink;
struct position red;  

void test_read() {
  gotoxy(30,19); 
  int a_escribir = 16; 
  int cont = 0;
  while (cont < a_escribir) {
    set_color(cont, 0);
    int leidos = read(buff, a_escribir); 
    write(1, buff, leidos); 
    cont += leidos; 
  }
}

void test_shared_mem() {
  gotoxy(30,21);
  void* shared = (void*)0x1F4000; 
  void* shared_mem = shmat(0, shared);
  pid = fork(); 

  /*
  Codigo del hijo
  if (pid == 0) {
    void* shared = (void*)0x1F4000; 
    void* shared_mem = shmat(0, shared);
    char* c = (char*)shared_mem; 
    //void* shared = (void*)0x1F4000; 
    char* c = (char*)shared; 
    write(1, "ESCRIBE EL HIJO\n", strlen("ESCRIBE EL HIJO\n"));
    write(1, c, 1); 
  }
  */
  
  /* Codigo del padre */ 
  /* Prueba con exit */ 
  char* c = (char*)shared_mem; 
  *c = 'C';
  write(1, "ESCRIBE EL PADRE\n", strlen("ESCRIBE EL PADRE\n")); 
  write(1, c, 1);
  exit(); 
}

void test_cow() {

  unsigned int data_address = 0x10E000; 
  char* c = (char*)data_address; 
  *(c) = '4';
  pid = fork();  

  /* Codigo del padre */ 
  if (pid != 0) {
    write(1, "ESCRIBE EL PADRE EL CONTENIDO DE LA VARIABLE:\n", 
      sizeof("ESCRIBE EL PADRE EL CONTENIDO DE LA VARIABLE:\n"));
    *(c) = '7'; 
    write(1, c, 1); 
  }
  
  if (pid == 0) {
    write(1, "ESCRIBE EL HIJO EL CONTENIDO DE LA VARIABLE:\n", 
      sizeof("ESCRIBE EL HIJO EL CONTENIDO DE LA VARIABLE:\n"));
    write(1, c, 1); 
  }
}

struct position posibles [4]; 

char voidline[80] = {"                                                                                "};

char screen[24][80] = {
"################################################################################",
"#******************************************************************************#",
"#******************************************************************************#",
"#***######***############***####***############***#######***####***#########***#",
"#***######***############***####***############***#######***####***#########***#",
"#******************************************************************************#",
"#***######***####***############***#######***#######***############***#######**#",
"#***######***####***############***#######***#######***############***#######**#",
"#******************************************************************************#",
"#***######***################***########***#######***########***###########****#",
"#***######***################***########***#######***########***###########****#",
"#******************************************************************************#",
"#***####******###############****########******#################*******####****#",
"#***####******###############****########******#################*******####****#",
"#******************************************************************************#",
"#***######***##########***#######*******###############***#######***########***#",
"#***######***##########***#######*******###############***#######***########***#",
"#******************************************************************************#",
"#***######***################***#######***#######***################***#####***#",
"#***######***################***#######***#######***################***#####***#",
"#***######***################***#######***#######***################***#####***#",
"#******************************************************************************#",
"#******************************************************************************#",
"################################################################################",
};

/* PACMAN FUNCTIONS */

void initialize_positions() {
  pacman_pos.row = 12; 
  pacman_pos.col = 44; 
  posibles[0].row = 0; 
  posibles[0].col = 1; 
  posibles[1].row = 0; 
  posibles[1].col = -1; 
  posibles[2].row = 1; 
  posibles[2].col = 0; 
  posibles[3].row = -1; 
  posibles[3].col = 0; 
  blue.row = 1; 
  blue.col = 78; 
  pink.row = 22;
  pink.col = 1; 
  red.row = 22; 
  red.col = 78; 
  red.previous = '*';
  blue.previous = '*'; 
  pink.previous = '*';  
}

void moricion() {
  screen[pacman_pos.row][pacman_pos.col] = ' '; 
  --lifes; 
  screen[red.row][red.col] = ' '; 
  screen[blue.row][blue.col] = ' '; 
  screen[pink.row][pink.col] = ' ';
  initialize_positions(); 
}

void screen_paint() {
  gotoxy(0,1);
  for (int i = 0; i < 24; ++i) {
    for (int j = 0; j < 80; ++j) {
      if (screen[i][j] == '#') set_color(9,0); 
      else if (screen[i][j] == '*') set_color(5,0); 
      else if (screen[i][j] == 'C') set_color(14,0);
      else if (screen[i][j] == 'F') set_color(11,0); 
      buff[0] = screen[i][j];
      write(1, buff, 1); 
    }
  }
}

void lifes_paint() {
  gotoxy(23,0);
  set_color(15,0); 
  itoa(lifes, buff); 
  write(1, buff, strlen(buff));
}

int dist_pacman (struct position pi) {
  // posibles = derecha, izquierda, abajo, arriba.
  int visitados[24][80] = {0}; 
  struct position nivel_actual[1920]; 
  struct position siguiente_nivel[1920]; 
  int size_actual = 0; 
  int size_siguiente = 0; 
  int min_dist = 0;

  nivel_actual[size_actual++] = pi; 
  visitados[pi.row][pi.col] = 1; 

  while (size_actual > 0) {

    for (int i = 0; i < size_actual; ++i) {
      struct position nodo = nivel_actual[i]; 

      //Exploramos vecinos del nodo actual
      for (int j = 0; j < 4; ++j) {
        struct position new_position;  
        new_position.row = nodo.row + posibles[j].row; 
        new_position.col = nodo.col + posibles[j].col;
        if ((screen[new_position.row][new_position.col] != '#')
          && (visitados[new_position.row][new_position.col] == 0)) {
          if (screen[new_position.row][new_position.col] == 'C') return min_dist + 1; 
          siguiente_nivel[size_siguiente++] = new_position; 
          visitados[new_position.row][new_position.col] = 1; 
        }
      }
    }

    //Movemos los nodos del siguiente nivel al actual
    size_actual = size_siguiente; 
    size_siguiente = 0;
    ++min_dist;  
    for (int i = 0; i < size_actual; ++i) {
      nivel_actual[i] = siguiente_nivel[i]; 
    }
  }

  return -1; 
}

void user_info_paint () {
  gotoxy(0,0);
  set_color(15,0); 
  write(1, "POINTS: ", 8); 
  gotoxy(15,0);
  write(1, "LIFES: ", 7);  
  gotoxy(50,0);
  write(1, "FPS: ", 5);
}

void points_paint () {
  gotoxy(8,0);
  set_color(15,0); 
  itoa(points, buff); 
  write(1, buff, strlen(buff));
}

void pacman_moves(char move) {
  screen[pacman_pos.row][pacman_pos.col] = ' '; 
  if (move == 'w') {
    if (screen[pacman_pos.row - 1][pacman_pos.col] != '#') {
      --pacman_pos.row;
    }
  }
  else if (move == 'a') {
    if (screen[pacman_pos.row][pacman_pos.col - 1] != '#') {
      --pacman_pos.col;
    }
  }
  else if (move == 's') {
    if (screen[pacman_pos.row + 1][pacman_pos.col] != '#') {
      ++pacman_pos.row;
    }
  }
  else if (move == 'd') {
    if (screen[pacman_pos.row][pacman_pos.col + 1] != '#') {
      ++pacman_pos.col;
    }
  }
  if(screen[pacman_pos.row][pacman_pos.col] == '*') points += 50;
  if (screen[pacman_pos.row][pacman_pos.col] == 'F') moricion(); 
  screen[pacman_pos.row][pacman_pos.col] = 'C'; 
}

void ghost_moves(struct position ghost_position, int fantasma) {
  screen[ghost_position.row][ghost_position.col] = ghost_position.previous; 
  int movimiento = -1;
  int min_dist = 2000;  
  for (int i = 0; i < 4; ++i) {
    struct position pos; 
    pos.row = ghost_position.row + posibles[i].row;
    pos.col = ghost_position.col + posibles[i].col; 
    if (screen[pos.row][pos.col] != '#') {
      int dist = dist_pacman(pos);
      if ((dist != -1) && (dist < min_dist)) {
        movimiento = i;  
        min_dist = dist;
      }
      if (screen[pos.row][pos.col] == 'C') {
        min_dist = -1; 
        movimiento = i; 
      }
    }
  } 
  if (movimiento != -1) {
    ghost_position.row += posibles[movimiento].row; 
    ghost_position.col += posibles[movimiento].col; 
  }
  if (screen[ghost_position.row][ghost_position.col] == 'C') {
    moricion(); 
    return; 
  }
  ghost_position.previous = screen[ghost_position.row][ghost_position.col];

  if (ghost_position.previous == 'F') {
    ghost_position.previous = ' ';
    points += 50; 
  }  
  screen[ghost_position.row][ghost_position.col] = 'F'; 

  if (fantasma == 1) red = ghost_position; 
  if (fantasma == 2) blue = ghost_position; 
  if (fantasma == 3) pink = ghost_position; 
}

void fps_control() {
  ++fps;
  if(gettime()%18 == 0) {
    gotoxy(55,0);
    itoa(fps, buff);
    write(1, (char *)buff, strlen(buff));
    fps = 0;
  } 
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* MAX POINTS = 50600 */ 
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
    pid = fork(); 
    if (pid == 0) {
      while(1) {
        void* shared = shmat(0, (void*)0x1F4000); 
        char* move = (char*)shared;
        int a_leer = 1; 
        int leidos = 0; 
        while (leidos < a_leer) {
          leidos = read(move, 1); 
        }
      } 
    }
    else {
      gotoxy(0,0);
      write(1, (char*)voidline, 80);
      void* shared = shmat(0, (void*)0x1F4000); //Shared memory page 500
      char* move = (char*)shared; 
      user_info_paint(); 
      initialize_positions(); 
      while(lifes && (points < 50000)) {
        if (gettime()%4 == 0) pacman_moves(*(move));
        if (gettime()%7 == 0) ghost_moves(pink, 3); 
        if (gettime()%8 == 0) ghost_moves(blue, 2); 
        if (gettime()%9 == 0) ghost_moves(red, 1); 
        screen_paint(); 
        points_paint();
        lifes_paint(); 
        fps_control(); 
      }
      gotoxy(0,0); 
      for (int i = 0; i < 25; ++i) {
        write(1, voidline, strlen(voidline));
      }
      gotoxy(28, 10); 
      if (!lifes) {
        write(1, "TE HAN MATADO!\n", sizeof("THE HAN MATADO!\n"));
        gotoxy(18, 12); 
        write(1, "EL SENYOR BAKA BAKA ESTA DECEPCIONADO :(\n", sizeof("EL SENYOR BAKA BAKA ESTA DECEPCIONADO :(\n")); 
      }
      else {
        write(1, "HAS GANADO!\n", sizeof("HAS GANADO!\n")); 
        gotoxy(18, 11); 
        write(1, "EL SENYOR BAKA BAKA ESTA ORGULLOSO :)\n", sizeof("EL SENYOR BAKA BAKA ESTA ORGULLOSO :)\n"));
      }
    }
    while(1); 
}
