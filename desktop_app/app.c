#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h> // per configurare la seriale
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define SERIAL "/dev/ttyACM0"
#define BAUD B19200
#define MAX_ATTEMPTS 3
#define MAX_LENGTH_MSG 254 // non 255, perché serve +1 byte per '\0' 

uint8_t check_auth(int serial) {
  uint8_t cnt = 0;
  char password[MAX_LENGTH_MSG];
  char risposta[MAX_LENGTH_MSG];

  while(cnt < MAX_ATTEMPTS){
    printf("Inserisci password:\t");
    scanf("%254s", password); // 254s per evitare buffer overflow
    password[strlen(password)] = 0;
    password[MAX_LENGTH_MSG] = 0;
    ssize_t sent = write(serial, password, strlen(password)+1);
    if(sent < 0) {
      close(serial);
      assert("Errore scrittura seriale");
    }
    usleep(500000);
    ssize_t n = read(serial, risposta, MAX_LENGTH_MSG);
    if(n<0) {
      close(serial);
      assert("Errore nella lettura");
    }
    printf("Risposta: [%s]\n", risposta);

    if(risposta[0] == 'O' && risposta[1] == 'K') return 0;
    cnt++;
  }
  return 1;
}

int main() {
  int serial = open(SERIAL, O_RDWR);
  if(serial == -1) {
    assert("Errore nell'apertura della porta seriale");
  }
  sleep(2); // per dare tempo ad arduino di caricare il suo programma
  
  // configuro baud rate
  struct termios options;
  if(tcgetattr(serial, &options) == -1){ // legge impostazioni del terminale seriale
    close(serial);
    assert("Errore durante acquisizione impostazioni terminale");
  }

  if(cfsetispeed(&options, BAUD) == -1) {
    close(serial);
    assert("Errore durante setting input baud rate");
  }
  if(cfsetospeed(&options, BAUD) == -1) {
    close(serial);
    assert("Errore durante setting output baud rate");
  }
  options.c_cflag &= ~PARENB;  // Nessuna parità
  options.c_cflag &= ~CSTOPB;  // 1 bit di stop
  options.c_cflag &= ~CSIZE;   // NO configurazione della dimensione dei dati
  options.c_cflag |= CS8;      // 8 bit di dati
  options.c_lflag &= ~(ICANON | ECHO);  // raw mode - legge subito i dati senza aspettare '\n' e disabilita l'eco: quindi non ricevo dati "sporchi" che mando prima

  if(tcsetattr(serial, TCSANOW, &options) == -1) {
    close(serial);
    assert("Errore nell'applicazione degli attributi per seriale"); // 0 per opzioni opzionali
  }

  if(check_auth(serial)){
    printf("Non sei stato correttamente autenticato\nFINE\n");
    close(serial);
    return 1;
  }
  printf("Sei stato correttamente autenticato!\n");

  char msg[MAX_LENGTH_MSG];
  printf("\t\t==========  OPZIONI  ==========");
  printf("\n<C> -- encrypt");
  printf("\n<D> -- decrypt");
  printf("\nSTOP -- terminate\n");

  while(1){

    printf("\n\nFile <F> or message <M> or <STOP>:\t");
    scanf("%4s", msg);

    if(strcmp(msg, "STOP") == 0){
      msg[strlen(msg)] = 0;
      ssize_t n = write(serial, msg, strlen(msg)+1);
      if(n<0){
        close(serial);
        assert("Error on write on serial");
      }
      break;
    }

    if(*msg == 'M'){
      printf("\nInserisci l'opzione:\t");
      scanf("%254s", msg);
      msg[strlen(msg)] = 0;
      ssize_t n = write(serial, msg, strlen(msg)+1);
      if(n<0){
        close(serial);
        assert("Error on write on serial");
      }

      char *msg_to_crypt = malloc(4096);
      if (!msg_to_crypt) {
        close(serial);
        assert("malloc failed");
      }

      if (msg[0] == 'C') {
        printf("\nInserisci il messaggio:\t");
	read(0, msg_to_crypt, 4096); // 0 = stdin
        size_t len = strlen(msg_to_crypt);
        if(len > 0 && msg_to_crypt[len - 1] == '\n') {
          msg_to_crypt[len - 1] = '\0';
          len--;
        }

        int i = 0;
        while(len > 0){
          int chunk = len > 254 ? 254: len;
          *msg = chunk;
	  ssize_t n = write(serial, msg, 1);
          if(n < 0){
            close(serial);
            assert("Error on write on serial - probably you have entered too many bytes on option select");
          }
	  usleep(50000);
          //strncpy(msg, msg_to_crypt + i, chunk);
          n = write(serial, msg_to_crypt+i, chunk);
          if(n < 0){
            close(serial);
            assert("Error on write on serial - probably you have entered too many bytes on option select");
          }
          usleep(500000);
	  
          n = read(serial, msg, chunk);
          for (int j = 0; j < chunk; j++) {
            printf("%02x", (unsigned char)msg[j]);
          }
	  usleep(50000);
          len -= chunk;
          i += chunk;
        }

      } else if (msg[0] == 'D') {
        // ......
      }

      free(msg_to_crypt);
    }

    if(*msg == 'F'){
      // ......
    }
    //else printf("\nNOT A VALID OPTION\n");
  }

  close(serial);
  return 0;
}
