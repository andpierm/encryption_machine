#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h> // per configurare la seriale
#include <assert.h>
#include <string.h>

#define SERIAL "/dev/ttyACM0"
#define BAUD B19200
#define MAX_ATTEMPTS 3
#define MAX_LENGTH_MSG 255

uint8_t check_auth(int serial){
  uint8_t cnt = 0;
  char password[MAX_LENGTH_MSG];
  char risposta[MAX_LENGTH_MSG];
  while(cnt < MAX_ATTEMPTS){
    printf("Inserisci password:\t");
    scanf("%s", password);
    password[strlen(password)] = 0;
    ssize_t sent = write(serial, password, strlen(password)+1);
    if(sent < 0) {
      close(serial);
      assert("Errore scrittura seriale");
    }
    
    ssize_t n = read(serial, risposta, MAX_LENGTH_MSG); // non c'è bisogno della sleep in quanto read bloccante
    if(n<0) {
      close(serial);
      assert("Errore nella lettura");
    }
    printf("Risposta: [%s]\n", risposta);
    // tcflush(serial, TCIFLUSH); NON ERA QUI L'ERRORE!!!

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
  
  close(serial);
  return 0;
}
